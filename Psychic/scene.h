#ifndef SCENE_H
#define SCENE_H

#include <imgui/imgui.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "box2d/box2d.h"
#include "draw.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct Settings;
class ContactListener;
class DestructionListener;

constexpr auto RAND_LIMIT = 32767;
constexpr auto PI = 3.14159265358979323846;

/// Random number in range [-1,1]
inline float RandomFloat()
{
	float r = (float)(rand() & (RAND_LIMIT));
	r /= RAND_LIMIT;
	r = 2.0f * r - 1.0f;
	return r;
}

/// Random floating point number in range [lo, hi]
inline float RandomFloat(float lo, float hi)
{
	float r = (float)(rand() & (RAND_LIMIT));
	r /= RAND_LIMIT;
	r = (hi - lo) * r + lo;
	return r;
}

class DestructionListener : public b2DestructionListener
{
public:
	void SayGoodbye(b2Fixture* fixture) override { B2_NOT_USED(fixture); }
	void SayGoodbye(b2Joint* joint) override { B2_NOT_USED(joint); };

	ContactListener* ContactListener;
};

struct ContactPoint
{
	b2Fixture* fixtureA;
	b2Fixture* fixtureB;
	b2Vec2 normal;
	b2Vec2 position;
	b2PointState state;
	float normalImpulse;
	float tangentImpulse;
	float separation;
};

class ContactListener : public b2ContactListener
{
public:
	ContactListener(){};
	virtual ~ContactListener(){};

	// Let derived tests know that a joint was destroyed.
	virtual void JointDestroyed(b2Joint* joint) { B2_NOT_USED(joint); }

	// Callbacks for derived classes.
	virtual void BeginContact(b2Contact* contact)  override { B2_NOT_USED(contact); }
	virtual void EndContact(b2Contact* contact)  override { B2_NOT_USED(contact); }
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override
	{
		B2_NOT_USED(contact);
		B2_NOT_USED(impulse);
	}

protected:
	friend class DestructionListener;
	friend class BoundaryListener;
	friend class ContactListener;
};

struct ObjectData
{
	int id;
	std::string name;
	float x;
	float y;
	float density;
	float mass;
};

class Scene
{
public:
	Scene();
	virtual ~Scene();

	void DrawTitle(const char* string);
	virtual void Step(Settings& settings);
	virtual void UpdateUI();
	virtual void Keyboard(int key) { B2_NOT_USED(key); }
	virtual void KeyboardUp(int key) { B2_NOT_USED(key); }
	void ShiftMouseDown(const b2Vec2& p);
	virtual void MouseDown(const b2Vec2& p);
	virtual void MouseUp(const b2Vec2& p);
	virtual void MouseMove(const b2Vec2& p);
	void ShiftOrigin(const b2Vec2& newOrigin);

	void AddCircle(b2Vec2 pos, float radius, b2BodyType type);
	void AddEdge(b2Vec2 a, b2Vec2 b);

protected:
	ContactListener m_contactListener;
	DestructionListener m_destructionListener;

	std::vector<ObjectData*> m_objectDataList;

	b2Body* m_groundBody;
	b2World* m_world;
	b2MouseJoint* m_mouseJoint;
	b2Vec2 m_mouseWorld;
	int32 m_stepCount;
	int32 m_objectId;
	int32 m_textLine;
	int32 m_textIncrement;
	ObjectData* m_objectSeleted;
};

#endif