#include "scene.h"
#include "settings.h"

Scene::Scene()
{
	b2Vec2 gravity;
	gravity.Set(0.0f, -10.0f);
	m_world = new b2World(gravity);
	m_textLine = 30;
	m_textIncrement = 13;
	m_mouseJoint = NULL;
	m_pointCount = 0;

	m_destructionListener.ContactListener = &this->m_contactListener;
	m_world->SetDestructionListener(&m_destructionListener);
	m_world->SetContactListener(&this->m_contactListener);
	m_world->SetDebugDraw(&g_debugDraw);

	m_stepCount = 0;

	b2BodyDef bodyDef;
	m_groundBody = m_world->CreateBody(&bodyDef);
}

Scene::~Scene()
{
	delete m_world;
	m_world = NULL;
}

void Scene::DrawTitle(const char* string)
{
	g_debugDraw.DrawString(5, 5, string);
	m_textLine = int32(26.0f);
}

void Scene::ShiftOrigin(const b2Vec2& newOrigin)
{
	m_world->ShiftOrigin(newOrigin);
}

class QueryCallback : public b2QueryCallback
{
public:
	QueryCallback(const b2Vec2& point)
	{
		m_point = point;
		m_fixture = NULL;
	}

	bool ReportFixture(b2Fixture* fixture) override
	{
		b2Body* body = fixture->GetBody();
		if (body->GetType() == b2_dynamicBody)
		{
			bool inside = fixture->TestPoint(m_point);
			if (inside)
			{
				m_fixture = fixture;
				return false;
			}
		}

		return true;
	}

	b2Vec2 m_point;
	b2Fixture* m_fixture;
};

void Scene::MouseDown(const b2Vec2& p)
{
	m_mouseWorld = p;

	if (m_mouseJoint != NULL)
	{
		return;
	}

	b2AABB aabb;
	b2Vec2 d;
	d.Set(0.001f, 0.001f);
	aabb.lowerBound = p - d;
	aabb.upperBound = p + d;

	QueryCallback callback(p);
	m_world->QueryAABB(&callback, aabb);

	if (callback.m_fixture)
	{
		float frequencyHz = 5.0f;
		float dampingRatio = 0.7f;

		b2Body* body = callback.m_fixture->GetBody();
		b2MouseJointDef jd;
		jd.bodyA = m_groundBody;
		jd.bodyB = body;
		jd.target = p;
		jd.maxForce = 1000.f * body->GetMass();
		b2LinearStiffness(jd.stiffness, jd.damping, frequencyHz, dampingRatio, jd.bodyA, jd.bodyB);

		m_mouseJoint = (b2MouseJoint*)m_world->CreateJoint(&jd);
		body->SetAwake(true);
	}
}

void Scene::ShiftMouseDown(const b2Vec2& p)
{
	m_mouseWorld = p;

	if (m_mouseJoint != NULL)
	{
		return;
	}

}

void Scene::MouseUp(const b2Vec2& p)
{
	if (m_mouseJoint)
	{
		m_world->DestroyJoint(m_mouseJoint);
		m_mouseJoint = NULL;
	}
}

void Scene::MouseMove(const b2Vec2& p)
{
	m_mouseWorld = p;

	if (m_mouseJoint)
	{
		m_mouseJoint->SetTarget(p);
	}
}

void Scene::Step(Settings& settings)
{
	float timeStep = settings.m_hertz > 0.0f ? 1.0f / settings.m_hertz : float(0.0f);
	if (settings.m_pause)
	{
		timeStep = 0.0f;

		g_debugDraw.DrawString(5, m_textLine, "====PAUSED====");
		m_textLine += m_textIncrement;
	}

	uint32 flags = 0;
	flags += settings.m_drawShapes * b2Draw::e_shapeBit;
	flags += settings.m_drawJoints * b2Draw::e_jointBit;
	flags += settings.m_drawAABBs * b2Draw::e_aabbBit;
	flags += settings.m_drawCOMs * b2Draw::e_centerOfMassBit;
	g_debugDraw.SetFlags(flags);

	m_world->SetAllowSleeping(settings.m_enableSleep);
	m_world->SetWarmStarting(settings.m_enableWarmStarting);
	m_world->SetContinuousPhysics(settings.m_enableContinuous);
	m_world->SetSubStepping(settings.m_enableSubStepping);

	m_pointCount = 0;

	m_world->Step(timeStep, settings.m_velocityIterations, settings.m_positionIterations);

	m_world->DebugDraw();
	g_debugDraw.Flush();

	if (timeStep > 0.0f)
	{
		++m_stepCount;
	}
}