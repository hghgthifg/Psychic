#include "scene.h"
#include "settings.h"

#include <iostream>

Scene::Scene()
{
	b2Vec2 gravity;
	gravity.Set(0.0f, -10.0f);
	m_world = new b2World(gravity);
	m_textLine = 30;
	m_textIncrement = 13;
	m_mouseJoint = NULL;
	m_objectId = 1;
	m_objectSeleted = NULL;

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
	for (auto &i : m_objectDataList)
	{
		delete i;
		i = NULL;
	}
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

		m_objectSeleted = (ObjectData*)(callback.m_fixture->GetBody()->GetUserData().pointer);
		//std::cout << m_objectSeleted << std::endl;

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

	m_world->Step(timeStep, settings.m_velocityIterations, settings.m_positionIterations);

	//std::cout << m_world->GetBodyCount() << std::endl;;
	for (b2Body* i = m_world->GetBodyList(); i != NULL; i = i->GetNext())
	{
		if (i->GetType() == b2_dynamicBody)
		{
			int m = i->GetMass();
			ObjectData* od = (ObjectData*)i->GetUserData().pointer;
			std::cout << m << " " << (ObjectData*)i->GetUserData().pointer << " " << (__int64)(i) << std::endl;
			od->x = i->GetPosition().x;
			od->y = i->GetPosition().y;
		}
	}

	m_world->DebugDraw();
	g_debugDraw.Flush();

	if (timeStep > 0.0f)
	{
		++m_stepCount;
	}
}

void Scene::AddEdge(b2Vec2 a, b2Vec2 b)
{
	b2EdgeShape shape;
	shape.SetTwoSided(a, b);

	ObjectData* od = new ObjectData();
	m_objectDataList.push_back(od);

	b2FixtureDef fd;
	fd.shape = &shape;

	b2BodyDef bd;
	bd.userData.pointer = reinterpret_cast<uintptr_t>(od);

	b2Body* edge = m_world->CreateBody(&bd);
	edge->CreateFixture(&fd);

	od->id = m_objectId;
	od->mass = 0;
	od->density = 0;
	od->name = "Object" + std::to_string(m_objectId);
	od->x = (a.x + b.x) / 2.0f;
	od->y = (a.y + b.y) / 2.0f;
	m_objectId++;
}

void Scene::AddCircle(b2Vec2 pos, float radius, b2BodyType type)
{
	b2CircleShape shape;
	shape.m_radius = radius;

	ObjectData* od = new ObjectData();
	m_objectDataList.push_back(od);

	b2FixtureDef fd;
	fd.shape = &shape;
	fd.density = 1.0f;

	b2BodyDef bd;
	bd.type = type;
	bd.position.Set(pos);
	bd.userData.pointer = reinterpret_cast<uintptr_t>(od);

	b2Body* body = m_world->CreateBody(&bd);
	body->CreateFixture(&fd);

	od->id = m_objectId;
	od->mass = body->GetMass();
	od->density = fd.density;
	od->name = "Object" + std::to_string(m_objectId);
	od->x = pos.x;
	od->y = pos.y;
	m_objectId++;
}

void Scene::UpdateUI()
{
	if (m_objectSeleted != NULL)
	{
		ImGui::SetNextWindowPos(ImVec2(0, 20));
		//ImGui::SetNextWindowSize(ImVec2((float)200, (float)g_camera.m_height - 40));
		ImGui::Begin("Attribute", (bool*)1);
		ImGui::Text("name : %s" , m_objectSeleted->name.c_str());
		ImGui::Text("mass : %f", m_objectSeleted->mass);
		ImGui::Text("density : %f", m_objectSeleted->density);
		ImGui::Text("position : (%f,%f)", m_objectSeleted->x, m_objectSeleted->y);
		ImGui::End();
	}
}