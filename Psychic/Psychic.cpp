#define _CRT_SECURE_NO_WARNINGS
#define DEBUG

#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <imgui/imgui.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "stb_image.h"
#include "draw.h"
#include "settings.h"
#include "scene.h"

GLFWwindow* g_mainWindow = nullptr;

enum class OperateModel
{
	MOVING,
	EDITING
};
static Scene* s_scene = nullptr;
static Settings s_settings;
static bool s_rightMouseDown = false;
static bool s_leftMouseDown = true;
static b2Vec2 s_clickPointWS = b2Vec2_zero;
static OperateModel s_operateModel = OperateModel::MOVING;
static int s_operateDirection = 0;
static b2Body* s_bodySelected = nullptr;
static b2Vec2 s_objectOriginalPosition;

//回调
#pragma region
void glfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "GLFW error occured. Code: %d. Description: %s\n", error, description);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		return;
	}

	if (action == GLFW_PRESS)
	{
		switch (key)
		{
			/*
			case GLFW_KEY_ESCAPE:
				// Quit
				glfwSetWindowShouldClose(g_mainWindow, GL_TRUE);
				break;
			*/

		case GLFW_KEY_R:
			delete s_scene;
			s_scene = new Scene();
			break;

		case GLFW_KEY_P:
			s_settings.m_pause = !s_settings.m_pause;
			break;

		case GLFW_KEY_TAB:
			g_debugDraw.m_showUI = !g_debugDraw.m_showUI;
			break;

		case GLFW_KEY_C:
			s_scene->AddEdge(b2Vec2(40.f, 40.f), b2Vec2(0.f, 0.f));
			s_scene->AddCircle(b2Vec2(20.f, 30.f), 1, b2_dynamicBody);
			break;

		default:
			if (s_scene)
			{
				s_scene->Keyboard(key);
			}
		}
	}
	else if (action == GLFW_RELEASE)
	{
		s_scene->KeyboardUp(key);
	}
}

static void CharCallback(GLFWwindow* window, unsigned int c)
{
	ImGui_ImplGlfw_CharCallback(window, c);
}

static void MouseButtonCallback(GLFWwindow* window, int32 button, int32 action, int32 mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

	double xd, yd;
	glfwGetCursorPos(g_mainWindow, &xd, &yd);
	b2Vec2 ps((float)xd, (float)yd);
	s_clickPointWS = g_camera.ConvertScreenToWorld(ps);
	std::cout << s_clickPointWS.x << " " << s_clickPointWS.y << std::endl;

	// Use the mouse to move things around.
	if (button == GLFW_MOUSE_BUTTON_1)
	{
		if (action == GLFW_PRESS)
		{
			s_leftMouseDown = true;
		}

		if (action == GLFW_RELEASE)
		{
			s_leftMouseDown = false;
		}
		b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
		switch (s_operateModel)
		{
		case OperateModel::MOVING:
		{
			if (action == GLFW_PRESS)
			{
				if (mods == GLFW_MOD_SHIFT)
				{
					s_scene->ShiftMouseDown(pw);
				}
				else
				{
					s_scene->MouseDown(pw);
				}
			}

			if (action == GLFW_RELEASE)
			{
				s_scene->MouseUp(pw);
			}
			break;
		}
		case OperateModel::EDITING:
		{
			if (action == GLFW_RELEASE)
			{
				s_operateDirection = 0;
			}
			else
			{
				double bx = s_bodySelected->GetPosition().x;
				double by = s_bodySelected->GetPosition().y;
				std::cout << "(" << bx << ", " << by << ")" << std::endl;
				std::cout << "[" << s_objectOriginalPosition.x << ", " << s_objectOriginalPosition.y << "]" << std::endl;
				std::cout << "mouse position : (" << pw.x << ", " << pw.y << ")" << std::endl;
				std::cout << "object position : (" << bx << ", " << by << ")" << std::endl;
				if (pw.x > bx + 1.f && pw.x<bx + 5.f && pw.y>by - 1.f && pw.y < by + 1.f)
				{
					std::cout << "direction : x" << std::endl;
					s_operateDirection = 1;
					break;
				}
				if (pw.y > by + 1.f && pw.y<by + 5.f && pw.x>bx - 1.f && pw.x < bx + 1.f)
				{
					std::cout << "direction : y" << std::endl;
					s_operateDirection = 2;
					break;
				}
				break;
			}
		}
		default:
			break;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_2)
	{
		if (action == GLFW_PRESS)
		{
			s_rightMouseDown = true;
		}

		if (action == GLFW_RELEASE)
		{
			s_rightMouseDown = false;
		}
	}
}

static void MouseMotionCallback(GLFWwindow*, double xd, double yd)
{
	b2Vec2 ps((float)xd, (float)yd);

	b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
	s_scene->MouseMove(pw);

	switch (s_operateDirection)
	{
	case 1:
	{
		b2Transform tf = s_bodySelected->GetTransform();
		s_bodySelected->SetTransform(b2Vec2(s_objectOriginalPosition.x + pw.x - s_clickPointWS.x, tf.p.y), s_bodySelected->GetAngle());
		break;
	}
	case 2:
	{
		b2Transform tf = s_bodySelected->GetTransform();
		s_bodySelected->SetTransform(b2Vec2(tf.p.x, s_objectOriginalPosition.y + pw.y - s_clickPointWS.y), s_bodySelected->GetAngle());
		break;
	}
	default:
		break;
	}

	if (!s_leftMouseDown)
	{
		if (s_operateModel == OperateModel::EDITING)
		{
			s_objectOriginalPosition = s_bodySelected->GetPosition();
		}
	}
	if (s_rightMouseDown)
	{
		b2Vec2 diff = pw - s_clickPointWS;
		g_camera.m_center.x -= diff.x;
		g_camera.m_center.y -= diff.y;
		s_clickPointWS = g_camera.ConvertScreenToWorld(ps);
	}
}

static void ScrollCallback(GLFWwindow* window, double dx, double dy)
{
	ImGui_ImplGlfw_ScrollCallback(window, dx, dy);
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	if (dy > 0)
	{
		g_camera.m_zoom /= 1.1f;
	}
	else
	{
		g_camera.m_zoom *= 1.1f;
	}
}

static void ResizeWindowCallback(GLFWwindow*, int width, int height)
{
	g_camera.m_width = width;
	g_camera.m_height = height;
}
#pragma endregion

//ImGui初始化
static void CreateUI(GLFWwindow* window, const char* glslVersion = NULL)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	bool success;
	success = ImGui_ImplGlfw_InitForOpenGL(window, false);
	if (success == false)
	{
		fprintf(stderr, "ImGui_ImplGlfw_InitForOpenGL failed.\n");
		assert(false);
	}
	success = ImGui_ImplOpenGL3_Init(glslVersion);
	if (success == false)
	{
		fprintf(stderr, "ImGui_ImplOpenGL3_Init failed.\n");
		assert(false);
	}

	const char* fontPath1 = "resource/droid_sans.ttf";
	const char* fontPath2 = "../resource/droid_sans.ttf";
	const char* fontPath = nullptr;
	FILE* file1 = fopen(fontPath1, "rb");
	FILE* file2 = fopen(fontPath2, "rb");
	if (file1)
	{
		fontPath = fontPath1;
		fclose(file1);
	}

	if (file2)
	{
		fontPath = fontPath2;
		fclose(file2);
	}

	if (fontPath)
	{
		ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath, 13.0f);
	}
}

static void Restart()
{
	delete s_scene;
	s_scene = new Scene();
	s_operateModel = OperateModel::MOVING;
	s_operateDirection = 0;
	s_bodySelected = nullptr;
}

static void UpdateUI()
{
	const int menuWidth = 180;
	if (g_debugDraw.m_showUI)
	{
		ImGui::SetNextWindowPos(ImVec2((float)g_camera.m_width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)g_camera.m_height - 20));

		ImGui::Begin("Tools", &g_debugDraw.m_showUI, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		if (ImGui::BeginTabBar("ControlTabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Controls"))
			{
				ImVec2 button_sz = ImVec2(-1, 0);
				if (ImGui::Button("Pause (P)", button_sz))
				{
					s_settings.m_pause = !s_settings.m_pause;
				}

				if (ImGui::Button("Restart (R)", button_sz))
				{
					Restart();
				}

				if (ImGui::Button("Quit (Esc)", button_sz))
				{
					glfwSetWindowShouldClose(g_mainWindow, GL_TRUE);
				}

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();

		//Circle Button
#pragma region
		ImGui::SetNextWindowPos(ImVec2((float)g_camera.m_width - menuWidth - 100, 10));
		ImGui::SetNextWindowSize(ImVec2(80, 80));

		ImGui::Begin("Circle", NULL,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoBackground);
		if (ImGui::ImageButton(
			g_debugDraw.CreateTextureForImgui("circle.png"),
			ImVec2(48, 48),
			ImVec2(0, 0),
			ImVec2(1, 1),
			0,
			ImVec4(0.761719, 0.761719, 0.871094, 1),
			ImVec4(0.6, 0.7, 0.9, 1)))
		{
			if (s_operateModel != OperateModel::EDITING)
			{
				s_settings.m_pause = true;
				s_operateModel = OperateModel::EDITING;
				s_operateDirection = 0;

				s_scene->AddCircle(g_camera.ConvertScreenToWorld(b2Vec2(g_camera.m_width / 2.0f, g_camera.m_height / 2.0f)), 1, b2_dynamicBody);
				s_bodySelected = s_scene->GetLastBody();
				s_objectOriginalPosition = s_bodySelected->GetPosition();
				s_scene->SelectBody(s_bodySelected);
			}
		}
		ImGui::End();
#pragma endregion

		//Square Button
		ImGui::SetNextWindowPos(ImVec2((float)g_camera.m_width - menuWidth - 150, 10));
		ImGui::SetNextWindowSize(ImVec2(80, 80));

		ImGui::Begin("Square", NULL,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoBackground);
		if (ImGui::ImageButton(
			g_debugDraw.CreateTextureForImgui("square.png"),
			ImVec2(48, 48),
			ImVec2(0, 0),
			ImVec2(1, 1),
			0,
			ImVec4(0.761719, 0.761719, 0.871094, 1),
			ImVec4(0.6, 0.7, 0.9, 1)))
		{
			if (s_operateModel != OperateModel::EDITING)
			{
				s_settings.m_pause = true;
				s_operateModel = OperateModel::EDITING;
				s_operateDirection = 0;

				s_scene->AddRectangle(g_camera.ConvertScreenToWorld(b2Vec2(g_camera.m_width / 2.0f, g_camera.m_height / 2.0f)), b2Vec2(1, 1), b2_dynamicBody);
				s_bodySelected = s_scene->GetLastBody();
				s_objectOriginalPosition = s_bodySelected->GetPosition();
				s_scene->SelectBody(s_bodySelected);
			}
		}
		ImGui::End();

		//Confrim Button
#pragma region
		ImGui::SetNextWindowPos(ImVec2((float)g_camera.m_width - menuWidth - 200, 10));
		ImGui::SetNextWindowSize(ImVec2(80, 80));

		ImGui::Begin("Confirm", NULL,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoBackground);
		if (ImGui::ImageButton(
			g_debugDraw.CreateTextureForImgui("confirm.png"),
			ImVec2(48, 48),
			ImVec2(0, 0),
			ImVec2(1, 1),
			0,
			ImVec4(0.761719, 0.761719, 0.871094, 1),
			ImVec4(0.6, 0.7, 0.9, 1)))
		{
			if (s_operateModel == OperateModel::EDITING)
			{
				s_settings.m_pause = false;
				s_operateModel = OperateModel::MOVING;
				s_operateDirection = 0;

				std::cout << "Confrim" << std::endl;

				s_bodySelected = nullptr;
				s_objectOriginalPosition = b2Vec2_zero;
			}
		}
		ImGui::End();
#pragma endregion

		if (s_operateModel == OperateModel::EDITING)
		{
			g_debugDraw.DrawArrow(
				s_bodySelected->GetPosition() + b2Vec2(1, 0),
				s_bodySelected->GetPosition() + b2Vec2(5, 0),
				b2Color(1, 0, 0, 1));
			g_debugDraw.DrawArrow(
				s_bodySelected->GetPosition() + b2Vec2(0, 1),
				s_bodySelected->GetPosition() + b2Vec2(0, 5),
				b2Color(0, 1, 0, 1));
			std::cout << s_bodySelected->GetPosition().x <<" " << s_bodySelected->GetPosition().y << std::endl;
			g_debugDraw.Flush();
		}
		s_scene->UpdateUI();
	}
}

int main(int, char**)
{
	s_settings.Load();

	//OpenGl初始化
#pragma region
	glfwSetErrorCallback(glfwErrorCallback);

	g_camera.m_width = 1600;
	g_camera.m_height = 900;

	if (glfwInit() == 0)
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

#if __APPLE__
	const char* glslVersion = "#version 150";
#else
	const char* glslVersion = NULL;
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	char buffer[128];

	sprintf(buffer, "Psychic version 0.1");

	g_mainWindow = glfwCreateWindow(g_camera.m_width, g_camera.m_height, buffer, NULL, NULL);

	if (g_mainWindow == NULL)
	{
		fprintf(stderr, "Failed to create window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(g_mainWindow);

	int version = gladLoadGL(glfwGetProcAddress);
	printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	glfwSetScrollCallback(g_mainWindow, ScrollCallback);
	glfwSetWindowSizeCallback(g_mainWindow, ResizeWindowCallback);
	glfwSetKeyCallback(g_mainWindow, KeyCallback);
	glfwSetCharCallback(g_mainWindow, CharCallback);
	glfwSetMouseButtonCallback(g_mainWindow, MouseButtonCallback);
	glfwSetCursorPosCallback(g_mainWindow, MouseMotionCallback);
	glfwSetScrollCallback(g_mainWindow, ScrollCallback);

	glfwSwapInterval(1);

	g_debugDraw.Create();

	CreateUI(g_mainWindow, glslVersion);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
#pragma endregion

	s_scene = new Scene();

	std::chrono::duration<double> frameTime(0.0);
	std::chrono::duration<double> sleepAdjust(0.0);

	//窗口循环
	while (!glfwWindowShouldClose(g_mainWindow))
	{
		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

		glfwGetWindowSize(g_mainWindow, &g_camera.m_width, &g_camera.m_height);

		int bufferWidth, bufferHeight;
		glfwGetFramebufferSize(g_mainWindow, &bufferWidth, &bufferHeight);
		glViewport(0, 0, bufferWidth, bufferHeight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		if (g_debugDraw.m_showUI)
		{
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowSize(ImVec2(float(g_camera.m_width), float(g_camera.m_height)));
			ImGui::SetNextWindowBgAlpha(0.0f);
			ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
			ImGui::End();

			s_scene->DrawTitle("Psychic");
		}

		s_scene->Step(s_settings);

		UpdateUI();

#ifdef DEBUG
		if (g_debugDraw.m_showUI)
		{
			sprintf(buffer, "%.1f fps", 1.0 / frameTime.count());
			g_debugDraw.DrawString(5, g_camera.m_height - 20, buffer);

			double xx, yy;
			glfwGetCursorPos(g_mainWindow, &xx, &yy);
			b2Vec2 ps((float)xx, (float)yy);
			b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);

			g_debugDraw.DrawString(5, g_camera.m_height - 40, "mouse position : (%.5f,%.5f)", pw.x, pw.y);
		}
#endif // DEBUG

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_mainWindow);

		glfwPollEvents();

		std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		std::chrono::duration<double> target(1.0 / 60.0);
		std::chrono::duration<double> timeUsed = t2 - t1;
		std::chrono::duration<double> sleepTime = target - timeUsed + sleepAdjust;
		while (sleepTime > std::chrono::duration<double>(0))
		{
			t2 = std::chrono::steady_clock::now();
			timeUsed = t2 - t1;
			sleepTime = target - timeUsed + sleepAdjust;
		}

		std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();
		frameTime = t3 - t1;

		sleepAdjust = 0.9 * sleepAdjust + 0.1 * (target - frameTime);
	}

	delete s_scene;
	s_scene = nullptr;

	g_debugDraw.Destroy();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	glfwTerminate();

	return 0;
}