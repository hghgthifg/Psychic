#include "../test.h"

#include <vector>

class Dominos : public Test
{
public:

	struct platform
	{
		b2Body* body;
		float left;
		float right;
	};

	std::vector<platform*> m_platform;

	auto CreateMovingPlatform(float px,float py,float speed,float hx,float hy,float left,float right)
	{
		platform* pf = new platform();

		b2BodyDef bd;
		bd.type = b2_kinematicBody;
		bd.position.Set(px,py);

		pf->body = m_world->CreateBody(&bd);

		b2PolygonShape shape;
		shape.SetAsBox(hx, hy, b2Vec2(0, 0), 0.0f);

		b2FixtureDef fd;
		fd.shape = &shape;
		fd.friction = 0.6f;
		fd.density = 2.0f;
		pf->body->CreateFixture(&fd);
		pf->body->SetLinearVelocity(b2Vec2(-speed, 0.0f));
		pf->body->SetAngularVelocity(0.0f);
		pf->left = left;
		pf->right = right;
		m_platform.push_back(pf);
		return m_platform.back();
	}

	Dominos()
	{
		//地面
		b2Body* b1;
		{
			b2EdgeShape shape;
			shape.SetTwoSided(b2Vec2(-10.0f, 0.0f), b2Vec2(12.0f, 0.0f));

			b2BodyDef bd;
			b1 = m_world->CreateBody(&bd);
			b1->CreateFixture(&shape, 0.0f);
		}

		//平台1
		{
			b2PolygonShape shape;
			shape.SetAsBox(6.0f, 0.25f);

			b2BodyDef bd;
			bd.position.Set(-1.5f, 10.0f);
			b2Body* ground = m_world->CreateBody(&bd);
			ground->CreateFixture(&shape, 0.0f);
		}

		//竖着的多米诺骨牌
		{
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, 1.0f);

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 20.0f;
			fd.friction = 0.1f;

			for (int i = 0; i < 10; ++i)
			{
				b2BodyDef bd;
				bd.type = b2_dynamicBody;
				bd.position.Set(-6.0f + 1.0f * i, 11.25f);
				b2Body* body = m_world->CreateBody(&bd);
				body->CreateFixture(&fd);
			}
		}

		//斜着的平台
		{
			b2PolygonShape shape;
			shape.SetAsBox(7.0f, 0.25f, b2Vec2_zero, 0.3f);

			b2BodyDef bd;
			bd.position.Set(1.0f, 6.0f);
			b2Body* ground = m_world->CreateBody(&bd);
			ground->CreateFixture(&shape, 0.0f);
		}

		//竖着的墙
		b2Body* b2;
		{
			b2PolygonShape shape;
			shape.SetAsBox(0.25f, 1.5f);

			b2BodyDef bd;
			bd.position.Set(-7.0f, 4.0f);
			b2 = m_world->CreateBody(&bd);
			b2->CreateFixture(&shape, 0.0f);
		}

		//斜着的可动平台
		b2Body* b3;
		{
			b2PolygonShape shape;
			shape.SetAsBox(6.0f, 0.125f);

			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(-0.9f, 1.0f);
			bd.angle = -0.15f;

			b3 = m_world->CreateBody(&bd);
			b3->CreateFixture(&shape, 10.0f);
		}

		b2RevoluteJointDef jd;
		b2Vec2 anchor;

		anchor.Set(-2.0f, 1.0f);
		jd.Initialize(b1, b3, anchor);
		jd.collideConnected = true;
		m_world->CreateJoint(&jd);

		//摆锤
		b2Body* b4;
		{
			b2PolygonShape shape;
			shape.SetAsBox(0.25f, 0.25f);

			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(-10.0f, 15.0f);
			b4 = m_world->CreateBody(&bd);
			b4->CreateFixture(&shape, 10.0f);
		}

		//摆锤中心
		anchor.Set(-7.0f, 15.0f);
		jd.Initialize(b2, b4, anchor);
		m_world->CreateJoint(&jd);

		//盒子
		b2Body* b5;
		{
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(6.5f, 3.0f);
			b5 = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			b2FixtureDef fd;

			fd.shape = &shape;
			fd.density = 10.0f;
			fd.friction = 0.1f;

			shape.SetAsBox(1.0f, 0.1f, b2Vec2(0.0f, -0.9f), 0.0f);
			b5->CreateFixture(&fd);

			shape.SetAsBox(0.1f, 1.0f, b2Vec2(-0.9f, 0.0f), 0.0f);
			b5->CreateFixture(&fd);

			shape.SetAsBox(0.1f, 1.0f, b2Vec2(0.9f, 0.0f), 0.0f);
			b5->CreateFixture(&fd);
		}

		//盒子倾倒中心
		anchor.Set(6.0f, 2.0f);
		jd.Initialize(b1, b5, anchor);
		m_world->CreateJoint(&jd);

		//盖子
		b2Body* b6;
		{
			b2PolygonShape shape;
			shape.SetAsBox(1.0f, 0.1f);

			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(6.5f, 4.1f);
			b6 = m_world->CreateBody(&bd);
			b6->CreateFixture(&shape, 30.0f);
		}

		//盖子旋转
		anchor.Set(7.5f, 4.0f);
		jd.Initialize(b5, b6, anchor);
		m_world->CreateJoint(&jd);

		//盒子支架
		b2Body* b7;
		{
			b2PolygonShape shape;
			shape.SetAsBox(0.1f, 1.0f);

			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(7.4f, 1.0f);

			b7 = m_world->CreateBody(&bd);
			b7->CreateFixture(&shape, 10.0f);
		}

		//硬连接
		b2DistanceJointDef djd;
		djd.bodyA = b3;
		djd.bodyB = b7;
		djd.localAnchorA.Set(6.0f, 0.0f);
		djd.localAnchorB.Set(0.0f, -1.0f);
		b2Vec2 d = djd.bodyB->GetWorldPoint(djd.localAnchorB) - djd.bodyA->GetWorldPoint(djd.localAnchorA);
		djd.length = d.Length();

		b2LinearStiffness(djd.stiffness, djd.damping, 1.0f, 1.0f, djd.bodyA, djd.bodyB);
		m_world->CreateJoint(&djd);

		//球
		{
			float radius = 0.2f;

			b2CircleShape shape;
			b2FixtureDef fd;
			shape.m_radius = radius;
			fd.shape = &shape;
			fd.restitution = 0.3f;
			fd.density = 10.f;

			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(5.9f + 2.0f * radius, 2.4f);
			b2Body* body = m_world->CreateBody(&bd);
			body->CreateFixture(&fd);
		}
		
		//平台
		b2Body* b9 = CreateMovingPlatform(150.0f, -5.f, 10.f, 1.f, 0.2f, 5.f, 83.f)->body;
		{
			b2PolygonShape shape;
			b2FixtureDef fd;

			fd.shape = &shape;
			fd.density = 10.0f;
			fd.friction = 0.1f;

			/*
			shape.SetAsBox(0.1f, 1.0f, b2Vec2(-2.1f, 0.5f), 0.0f);
			b9->CreateFixture(&fd);
			*/

			shape.SetAsBox(0.2f, 0.9f, b2Vec2(1.2f, 0.7f), 0.0f);
			b9->CreateFixture(&fd);
		}

		//桥
		int e_count = 10;
		b2Body* m_middle;
		{
			b2PolygonShape shape;
			shape.SetAsBox(0.5f, 0.125f);

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 10.0f;
			fd.friction = 0.2f;

			b2RevoluteJointDef jd;

			b2Body* prevBody = b1;
			for (int32 i = 0; i < e_count; ++i)
			{
				b2BodyDef bd;
				bd.type = b2_dynamicBody;
				bd.position.Set(-14.5f + 1.0f * i, -10.0f);
				b2Body* body = m_world->CreateBody(&bd);
				body->CreateFixture(&fd);

				b2Vec2 anchor(-15.0f + 1.0f * i, -10.0f);
				jd.Initialize(prevBody, body, anchor);
				m_world->CreateJoint(&jd);

				if (i == (e_count >> 1))
				{
					m_middle = body;
				}
				prevBody = body;
			}

			b2Vec2 anchor(-15.0f + 1.0f * e_count, -10.0f);
			jd.Initialize(prevBody, b1, anchor);
			m_world->CreateJoint(&jd);
		}

		//阻挡平台
		b2Body* b10;
		{
			b2EdgeShape shape;
			b2BodyDef bd;
			b2FixtureDef fd;
			b10 = m_world->CreateBody(&bd);
			fd.friction = 10.f;

			shape.SetTwoSided(b2Vec2(-35.f, -17.96f), b2Vec2(-20.f, -20.0f));
			fd.shape = &shape;
			b10->CreateFixture(&fd);
			//shape.SetTwoSided(b2Vec2(-24.5f, -19.0f), b2Vec2(-24.5f, -20.0f));
			//fd.shape = &shape;
			//b10->CreateFixture(&fd);
		}

		//弹簧上的平台
		b2Body* b11;
		{
			b2PolygonShape shape;
			shape.SetAsBox(1.0f, 0.1f);
			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 20.f;

			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(-20.f, -15.f);
			bd.fixedRotation = true;

			b11 = m_world->CreateBody(&bd);
			b11->CreateFixture(&fd);

			b2DistanceJointDef jd;
			jd.Initialize(b1, b11, b2Vec2(0.0, 15.0f), bd.position);
			jd.localAnchorA.Set(-20.f, -5.f);
			jd.collideConnected = true;
			jd.minLength = 0.f;
			jd.maxLength = 10.f;
			jd.length = 8.f;
			b2LinearStiffness(jd.stiffness, jd.damping, 0.5f, 0.05f, jd.bodyA, jd.bodyB);
			m_world->CreateJoint(&jd);
		}

		//平台
		b2Body* b12;
		{
			b2EdgeShape shape;
			b2BodyDef bd;
			b2FixtureDef fd;
			b12 = m_world->CreateBody(&bd);

			shape.SetTwoSided(b2Vec2(-12.5f, -20.0f), b2Vec2(-5.f, -20.0f));
			fd.shape = &shape;
			fd.friction = 10.f;
			b10->CreateFixture(&fd);

			shape.SetTwoSided(b2Vec2(-5.f, -18.f), b2Vec2(-5.f, -20.f));
			fd.shape = &shape;
			fd.friction = 10.f;
			b10->CreateFixture(&fd);

		}

		//活塞
		{
			b2Body* prevBody = b12;
			{
				b2PolygonShape shape;
				shape.SetAsBox(4.f, 0.2f);

				b2BodyDef bd;
				bd.type = b2_dynamicBody;
				bd.position.Set(-32.f, -22.5f);
				b2Body* body = m_world->CreateBody(&bd);
				body->CreateFixture(&shape, 2.0f);

				b2RevoluteJointDef rjd;
				rjd.Initialize(prevBody, body, b2Vec2(-36.0f, -22.5f));
				m_world->CreateJoint(&rjd);

				prevBody = body;
			}
			{
				b2PolygonShape shape;
				shape.SetAsBox(7.0f, 0.2f);

				b2BodyDef bd;
				bd.type = b2_dynamicBody;
				bd.position.Set(-21.0f, -22.5f);
				b2Body* body = m_world->CreateBody(&bd);
				body->CreateFixture(&shape, 2.0f);

				b2RevoluteJointDef rjd;
				rjd.Initialize(prevBody, body, b2Vec2(-28.0f, -22.5f));
				m_world->CreateJoint(&rjd);

				prevBody = body;
			}
			{
				b2PolygonShape shape;
				shape.SetAsBox(2.45f, 2.45f);

				b2BodyDef bd;
				bd.type = b2_dynamicBody;
				bd.fixedRotation = true;
				bd.position.Set(-14.0f, -22.5f);
				b2Body* body = m_world->CreateBody(&bd);
				body->CreateFixture(&shape, 2.0f);

				b2RevoluteJointDef rjd;
				rjd.Initialize(prevBody, body, b2Vec2(-14.0f, -22.5f));
				m_world->CreateJoint(&rjd);

				b2PrismaticJointDef pjd;
				pjd.Initialize(b12, body, b2Vec2(-16.0f, -22.5), b2Vec2(1.0f, 0.0f));
				m_world->CreateJoint(&pjd);
			}
		}
	}

	void Step(Settings& settings) override
	{
		b2Vec2 p;
		b2Vec2 v;
		for (auto it = m_platform.begin(); it != m_platform.end(); it++)
		{
			auto i = *it;
			p = i->body->GetTransform().p;
			v = i->body->GetLinearVelocity();

			if ((p.x < i->left && v.x < 0.0f) ||
				(p.x > i->right && v.x > 0.0f))
			{
				v.x = -v.x;
				i->body->SetLinearVelocity(v);
			}
		}

		Test::Step(settings);
	}

	static Test* Create()
	{
		return new Dominos;
	}
};

static int testIndex = RegisterTest("Domino", "Domino1", Dominos::Create);
