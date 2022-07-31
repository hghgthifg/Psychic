#ifndef DRAW_H
#define DRAW_H

#define GLFW_INCLUDE_NONE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <map>
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <box2d/box2d.h>
#include <imgui/imgui.h>

struct b2AABB;
struct GLRenderPoints;
struct GLRenderLines;
struct GLRenderTriangles;
struct GLFWwindow;

struct Camera
{
	Camera()
	{
		m_center.Set(0.0f, 20.f);
		m_zoom = 1.0f;
		m_width = 1280;
		m_height = 800;
	}

	b2Vec2 ConvertScreenToWorld(const b2Vec2& screenPoint);
	b2Vec2 ConvertWorldToScreen(const b2Vec2& worldPoint);
	void BuildProjectionMatrix(float* m, float zBias);

	b2Vec2 m_center;
	float m_zoom;
	int32 m_width;
	int32 m_height;
};

class DebugDraw : public b2Draw
{
public:
	DebugDraw();
	~DebugDraw();

	void Create();
	void Destroy();

	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;

	void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;

	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

	void DrawTransform(const b2Transform& xf) override;

	void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;

	void DrawString(int x, int y, const char* string, ...);

	void DrawString(const b2Vec2& p, const char* string, ...);

	void DrawAABB(const b2AABB* aabb, const b2Color& color);

	void DrawArrow(const b2Vec2& from, const b2Vec2& to, const b2Color& color);

	ImTextureID CreateTextureForImgui(const char* filename);

	void Flush();

	bool m_showUI;

private:
	bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);

	GLRenderPoints* m_points;
	GLRenderLines* m_lines;
	GLRenderTriangles* m_triangles;
	std::map<const char*, GLuint> m_imageTexture;
};

extern DebugDraw g_debugDraw;
extern Camera g_camera;
extern GLFWwindow* g_mainWindow;

#endif