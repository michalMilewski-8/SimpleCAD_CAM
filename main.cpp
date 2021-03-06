#include <stdio.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <math.h>
#include <vector>

#include "Shader.h"
#include "Camera.h"
#include "Cursor.h"
#include "Torus.h"
#include "Point.h"
#include "BezierC0.h"
#include "BezierC2.h"
#include "BezierInterpol.h"
#include "BezierFlakeC0.h"
#include "BezierFlakeC2.h"
#include "TriangularGregoryPatch.h"
#include "Virtual.h"
#include "Virtual.h"
#include "Intersection.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Dependencies/include/rapidxml-1.13/rapidxml.hpp"
#include "Dependencies/include/rapidxml-1.13/rapidxml_print.hpp"
#include "Dependencies/include/rapidxml-1.13/rapidxml_utils.hpp"
//#include "./Dependencies/include/ImGuiFileDialog-Lib_Only/ImGuiFileDialog.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define EPS 0.1
#define PRECISION 1.0f
using namespace rapidxml;
using namespace std;

glm::vec3 cameraPos, cameraFront, cameraUp, lookAt, moving_up;
unsigned int width_, height_;

int e = 0;
glm::mat4 projection, view, model, mvp;
glm::mat4 projection_i, view_i, model_i, mvp_i;
glm::vec2 mousePosOld, angle;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool stereoscopic = false;
bool plain = true;
float ipd = 0.01f;
float d = 1.0f;
float near = 0.001f;
float far = 200.0f;
bool serch_for_intersections_using_cursor = false;
float distance_d = 0.01f;

int number_of_divisions = 20;
Camera cam;
Shader ourShader;
std::unique_ptr<Cursor> cursor, center;

std::vector<std::shared_ptr<Object>> objects_list = {};

void draw_scene();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void adding_menu(std::vector<std::shared_ptr<Object>>& objects, glm::vec3 starting_pos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void transform_screen_coordinates_to_world(glm::vec3& world_coordinates_end, glm::vec3& world_coordinates_start, float x_pos, float y_pos);
void add_selected_points_to_selected_curve();
void create_gui();
std::pair<glm::vec4, glm::uvec2> look_for_intersection(std::vector<std::shared_ptr<Object>> obj);

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "SimpleCAD", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	const char* glsl_version = "#version 330";
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// build and compile our shader program
	// ------------------------------------
	ourShader = Shader("shader.vs", "shader.fs"); // you can name your shader files however you like
	Shader stereoscopicShader = Shader("stereo.vs", "stereo.fs");
	Shader gridShader = Shader("shader_grid.vs", "shader_grid.fs");

	model = glm::mat4(1.0f);
	view = glm::mat4(1.0f);

	cameraPos = { 0,0,3 };
	cameraFront = { 0,0,-1 };
	lookAt = { 0,0,0 };
	cameraUp = { 0,1,0 };

	angle = { -90.0f, 0.0f };
	width_ = DEFAULT_WIDTH;
	height_ = DEFAULT_HEIGHT;

	cam = Camera(cameraPos, cameraFront, cameraUp);
	cam.SetPerspective(glm::radians(45.0f), DEFAULT_WIDTH / (float)DEFAULT_HEIGHT, near, far);
	//cam->SetOrthographic(-1, 1, 1, -1, -1, 1);

	//objects_list.push_back(std::make_shared<Torus>(0.5, 0.1, 10, 10, glm::vec4(1, 1, 0, 1), ourShader));
	//objects_list.back()->screen_height = &height_;
	//objects_list.back()->screen_width = &width_;

	glEnable(GL_DEPTH_TEST);

	cursor = std::make_unique<Cursor>(ourShader);
	center = std::make_unique<Cursor>(ourShader);

	cursor->SetCursorPosition(lookAt);
	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates. NOTE that this plane is now much smaller and at the top of the screen
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f,  -1.0f,  0.0f, 0.0f,
		 1.0f,  -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f,  -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	unsigned int framebufferLeftEye;
	glGenFramebuffers(1, &framebufferLeftEye);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferLeftEye);
	unsigned int texColorBufferLeftEye;
	glGenTextures(1, &texColorBufferLeftEye);
	glBindTexture(GL_TEXTURE_2D, texColorBufferLeftEye);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBufferLeftEye, 0);
	unsigned int rboL;
	glGenRenderbuffers(1, &rboL);
	glBindRenderbuffer(GL_RENDERBUFFER, rboL);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboL);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int framebufferRightEye;
	glGenFramebuffers(1, &framebufferRightEye);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferRightEye);
	unsigned int texColorBufferRightEye;
	glGenTextures(1, &texColorBufferRightEye);
	glBindTexture(GL_TEXTURE_2D, texColorBufferRightEye);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBufferRightEye, 0);

	auto leftTexLocation = glGetUniformLocation(stereoscopicShader.ID, "screenTextureL");
	auto rightTexLocation = glGetUniformLocation(stereoscopicShader.ID, "screenTextureR");

	stereoscopicShader.use();
	glUniform1i(leftTexLocation, 0);
	glUniform1i(rightTexLocation, 1);

	unsigned int rboR;
	glGenRenderbuffers(1, &rboR);
	glBindRenderbuffer(GL_RENDERBUFFER, rboR);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboR);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	objects_list.push_back(std::make_shared<BezierFlakeC2>(ourShader, 1, glm::uvec2(5, 5), glm::vec2(0.25, 1)));
	objects_list.push_back(std::make_shared<BezierFlakeC0>(ourShader, 0, glm::uvec2(3, 3), glm::vec2(1, 1)));
	objects_list.back()->Select();
	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// cleaning frame
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		projection = cam.GetProjectionMatrix();
		projection_i = glm::inverse(projection);
		view = cam.GetViewMatrix();
		view_i = glm::inverse(view);

		mvp = projection * view;

		processInput(window);
		create_gui();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		GLint data;
		glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &data);

		if (!stereoscopic) {
			draw_scene();
		}
		else {
			glm::vec3 posN = cameraPos;
			glBindFramebuffer(GL_FRAMEBUFFER, framebufferLeftEye);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
			glEnable(GL_DEPTH_TEST);

			posN -= cam.GetRightVector() * (ipd / 2.0f);
			cam.LookAt(posN, cameraFront, cameraUp);
			view = cam.GetViewMatrix();
			projection = cam.ComputeProjectionMatrix(near, far, -near * ((float)width_ / (float)height_ - ipd) / (2.0f * d), near * ((float)width_ / (float)height_ + ipd) / (2.0f * d), near * 1.0f / (2.0f * d), -near * 1.0f / (2.0f * d));
			mvp = projection * view;
			draw_scene();

			glBindFramebuffer(GL_FRAMEBUFFER, framebufferRightEye);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
			glEnable(GL_DEPTH_TEST);

			posN += cam.GetRightVector() * ipd;
			cam.LookAt(posN, cameraFront, cameraUp);
			view = cam.GetViewMatrix();
			projection = cam.ComputeProjectionMatrix(near, far, -near * ((float)width_ / (float)height_ + ipd) / (2.0f * d), near * ((float)width_ / (float)height_ - ipd) / (2.0f * d), near * 1.0f / (2.0f * d), -near * 1.0f / (2.0f * d));
			mvp = projection * view;
			draw_scene();

			// second pass
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			stereoscopicShader.use();
			glBindVertexArray(quadVAO);
			glDisable(GL_DEPTH_TEST);

			glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
			glBindTexture(GL_TEXTURE_2D, texColorBufferLeftEye);
			glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
			glBindTexture(GL_TEXTURE_2D, texColorBufferRightEye);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			cam.LookAt(cameraPos, cameraFront, cameraUp);
		}

		// Render dear imgui into screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	// cleanup stuff
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	objects_list.clear();
	ourShader.deleteShader();
	return 0;
}

void draw_scene() {
	cursor->DrawObject(mvp);

	int number_of_selected = 0;
	glm::vec3 center_point = glm::vec3(0.0f);
	for (auto& ob : objects_list) {
		if (ob->selected) {
			if (std::dynamic_pointer_cast<Bezier>(ob)) continue;
			number_of_selected++;
			center_point += ob->GetPosition();
		}
		for (auto& vir : ob->GetVirtualObjects()) {
			if (vir->selected) {
				number_of_selected++;
				center_point += vir->GetPosition();
			}
		}
	}
	if (number_of_selected > 0) {
		center_point /= number_of_selected;
		center->SetCursorPosition(center_point);
		center->DrawObject(mvp);
	}

	for (auto& ob : objects_list) {
		ob->DrawObject(mvp);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	cam.SetPerspective(glm::radians(45.0f), width / (float)height, near, far);
	width_ = width;
	height_ = height;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		for (auto& obj : objects_list) {
			if (obj->selected)
			{
				obj->UnSelect();
			}
			for (auto& virt : obj->GetVirtualObjects()) {
				if (virt->selected)
				{
					virt->UnSelect();
				}
			}
		}
	}
}

void transform_screen_coordinates_to_world(glm::vec3& world_coordinates_end, glm::vec3& world_coordinates_start, float x_pos, float y_pos) {
	glm::vec4 NDC_ray_start(
		((float)x_pos / (float)width_ - 0.5f) * 2.0f,
		-((float)y_pos / (float)height_ - 0.5f) * 2.0f,
		-1.0f,
		1.0f
	);
	glm::vec4 NDC_ray_end(
		((float)x_pos / (float)width_ - 0.5f) * 2.0f,
		-((float)y_pos / (float)height_ - 0.5f) * 2.0f,
		0.0f,
		1.0f
	);

	glm::vec4 lRayStart_camera = projection_i * NDC_ray_start;
	lRayStart_camera /= -lRayStart_camera.w;
	glm::vec4 lRayStart_world = view_i * lRayStart_camera;
	lRayStart_world /= lRayStart_world.w;
	glm::vec4 lRayEnd_camera = projection_i * NDC_ray_end;
	lRayEnd_camera /= -lRayEnd_camera.w;
	glm::vec4 lRayEnd_world = view_i * lRayEnd_camera;
	lRayEnd_world /= lRayEnd_world.w;


	world_coordinates_end = lRayEnd_world;
	world_coordinates_start = lRayStart_world;

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;
	glm::vec2 mousePos = { xpos,ypos };
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
	{
		glm::vec2 diff = (mousePosOld - mousePos) * PRECISION;
		float cameraSpeed = 5.0f * deltaTime;
		float radius;
		diff *= cameraSpeed;

		glm::vec3 right_movement = cam.GetRightVector() * -diff.x;
		glm::vec3 up_movement = cam.GetUpVector() * diff.y;
		glm::vec3 angle2 = lookAt - (cameraPos + right_movement + up_movement);

		auto rotation = Object::RotationBetweenVectors(lookAt - cameraPos, angle2);
		auto roation = glm::toMat4(rotation);
		glm::vec3 odn = center->GetPosition();
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			switch (e) {
			case 0:
				odn = center->GetPosition();
				break;
			case 1:
				odn = cursor->GetPosition();
				break;
			default:
				return;
				break;
			}
			for (auto& obj : objects_list) {
				if (obj->selected)
				{
					obj->RotateObject(rotation);
					glm::vec4 pos = { obj->GetPosition() - odn, 0.0f };
					obj->MoveObjectTo(odn + static_cast<glm::vec3>(roation * pos));
				}
				for (auto& virt : obj->GetVirtualObjects()) {
					if (virt->selected)
					{
						glm::vec4 pos2 = { virt->GetPosition() - odn, 0.0f };
						virt->MoveObjectTo(odn + static_cast<glm::vec3>(roation * pos2));
					}
				}
			}
		}

		else {
			angle += diff;
			if (angle.y > 90.0f) angle.y = 90.0f - EPS;
			if (angle.y < -90.0f) angle.y = -90.0f + EPS;
			if (angle.x > 180.0f) angle.x = -180.0f + EPS;
			if (angle.x < -180.0f) angle.x = 180.0f - EPS;
			radius = glm::length(cameraPos - lookAt);

			cameraPos.x = lookAt.x + radius * glm::cos(glm::radians(angle.y)) * glm::cos(glm::radians(angle.x));
			cameraPos.z = lookAt.z + radius * -glm::cos(glm::radians(angle.y)) * glm::sin(glm::radians(angle.x));
			cameraPos.y = lookAt.y + radius * glm::sin(glm::radians(-angle.y));

			cameraFront = glm::normalize(lookAt - cameraPos);
			cam.LookAt(cameraPos, cameraFront, cameraUp);
		}
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		glm::vec2 diff = (mousePosOld - mousePos) * PRECISION;
		float cameraSpeed = 0.1f * deltaTime;

		glm::vec2 movement = diff * cameraSpeed;

		glm::vec3 right_movement = cam.GetRightVector() * movement.x;
		glm::vec3 up_movement = cam.GetUpVector() * -movement.y;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			for (auto& obj : objects_list) {
				if (obj->selected)
					obj->MoveObject(-right_movement + -up_movement);
				for (auto& virt : obj->GetVirtualObjects()) {
					if (virt->selected)
						virt->MoveObject(-right_movement + -up_movement);
				}
			}
		}
		else {
			cameraPos += right_movement + up_movement;
			lookAt += right_movement + up_movement;

			cam.LookAt(cameraPos, cameraFront, cameraUp);
		}
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT == GLFW_PRESS))
	{
		glm::vec2 diff = (mousePosOld - mousePos) * PRECISION;
		float cameraSpeed = 0.2f * deltaTime;

		glm::vec2 movement = diff * cameraSpeed;

		glm::vec3 right_movement = cam.GetRightVector() * -movement.x;
		glm::vec3 up_movement = cam.GetUpVector() * movement.y;
		cursor->MoveObject(right_movement + up_movement);
	}

	mousePosOld = mousePos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse)
		return;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		float minLength = std::numeric_limits<float>::max();

		Object* closest = nullptr;

		glm::vec3 lRayEnd_world;
		glm::vec3 lRayStart_world;
		double x_pos, y_pos;
		glfwGetCursorPos(window, &x_pos, &y_pos);

		//char buf[256];
		//sprintf_s(buf, "x: %f y: %f", x_pos, y_pos);
		//glfwSetWindowTitle(window, buf);

		transform_screen_coordinates_to_world(lRayEnd_world, lRayStart_world, x_pos, y_pos);

		glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
		lRayDir_world = glm::normalize(lRayDir_world);

		for (auto& obj : objects_list) {
			if (dynamic_cast<Point*>(obj.get())) {
				glm::vec3 point = obj->GetPosition();
				glm::vec3 toPoint(point - glm::vec3(lRayStart_world));
				glm::vec3 crossp = glm::cross(lRayDir_world, toPoint);
				float length = glm::length(crossp);
				if (length < minLength && length < 0.1f)
				{
					minLength = length;
					closest = obj.get();
				}
			}
			for (auto& virt : obj->GetVirtualObjects()) {
				auto vr_point = std::dynamic_pointer_cast<Point>(virt);
				if (vr_point) {
					glm::vec3 point2 = vr_point->GetPosition();
					glm::vec3 toPoint2(point2 - glm::vec3(lRayStart_world));
					glm::vec3 crossp2 = glm::cross(lRayDir_world, toPoint2);
					float length = glm::length(crossp2);
					if (length < minLength && length < 0.1f)
					{
						minLength = length;
						closest = vr_point.get();
					}
				}
			}
		}
		if (closest) {
			closest->Select();
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		float precision = 0.01f;

		float movement = 1.0f + yoffset * precision;
		if (movement <= 0.0f)
			movement = 0.1f;
		for (auto& obj : objects_list) {
			if (obj->selected)
				obj->ResizeObject({ movement,movement,movement });
		}
	}
	else {
		float precision = 0.01f;

		float movement = 1.0f - yoffset * precision;
		if (movement <= 0.0f)
			movement = 0.1f;
		//TODO: przerobi? to na co? ?adniejszego;
		//cam.ScaleWorld({ movement,movement,movement });
		cameraFront = glm::normalize(lookAt - cameraPos);
		float dist = glm::length(lookAt - cameraPos);
		cameraPos = lookAt - (cameraFront * dist * movement);
		cam.LookAt(cameraPos, cameraFront, cameraUp);
	}

	//cam.Zoom(yoffset * precision);

}

glm::vec4 solveGauss(glm::mat4x4 jac, glm::vec4 free) {

	float mat[4][5];

	for (int z = 0; z < 4; z++) {
		for (int h = 0; h < 4; h++) {
			mat[z][h] = jac[h][z];
		}
		mat[z][4] = free[z];
	}


	glm::vec4 res{0,0,0,0};
	int i, j, k, n;
	n = 4;
	for (i = 0; i < n; i++)
	{
		for (j = i + 1; j < n; j++)
		{
			if (abs(mat[i][i]) < abs(mat[j][i]))
			{
				for (k = 0; k < n + 1; k++)
				{
					/* swapping mat[i][k] and mat[j][k] */
					mat[i][k] = mat[i][k] + mat[j][k];
					mat[j][k] = mat[i][k] - mat[j][k];
					mat[i][k] = mat[i][k] - mat[j][k];
				}
			}
		}
	}

	/* performing Gaussian elimination */
	for (i = 0; i < n - 1; i++)
	{
		for (j = i + 1; j < n; j++)
		{
			float f = mat[j][i] / mat[i][i];
			for (k = 0; k < n + 1; k++)
			{
				mat[j][k] = mat[j][k] - f * mat[i][k];
			}
		}
	}
	/* Backward substitution for discovering values of unknowns */
	for (i = n - 1; i >= 0; i--)
	{
		res[i] = mat[i][n];

		for (j = i + 1; j < n; j++)
		{
			if (i != j)
			{
				res[i] = res[i] - mat[i][j] * res[j];
			}
		}
		res[i] = res[i] / mat[i][i];
	}

	return res;
}

std::pair<glm::vec4, glm::uvec2> look_for_intersection(std::vector<std::shared_ptr<Object>> obj) {
	float eps = 0.001f;
	float eps_cursor = 0.1f;

	if (obj.size() == 2) {
		auto first = obj[0]->GetParametrisations();
		auto second = obj[1]->GetParametrisations();

		for (int i = 0; i < first.size(); i++) {
			for (int j = 0; j < second.size(); j++) {
				int l = 0;

				auto x = glm::vec4(0.5, 0.5, 0.5, 0.5);
				auto xstart = glm::vec4(0, 0, 0, 0);
				auto xend = glm::vec4(1, 1, 1, 1);
				auto diff = first[i](x.x, x.y) - second[j](x.z, x.w);
				std::vector<glm::vec4> divides = {};
				std::vector<int> randoms = {};

				if (serch_for_intersections_using_cursor) {
					xstart = glm::vec4(1, 1, 0, 0);
					xend = glm::vec4(0, 0, 1, 1);
					
					float stride = 1.0f / (number_of_divisions - 1);
					float u = 0, v = 0;
					auto cursor_pos = cursor->GetPosition();
					for (int s = 0; s < number_of_divisions; s++) {
						for (int f = 0; f < number_of_divisions; f++) {
							if (glm::length(cursor_pos - first[i](u, v)) < eps_cursor) {
								xstart.x = min(u, xstart.x);
								xstart.y = min(v, xstart.y);
								xend.x = max(u, xend.x);
								xend.y = max(v, xend.y);
							}
							v += stride;
						}
						u += stride;
						v = 0;
					}
				}

				while ((std::abs(diff.x) > eps || std::abs(diff.y) > eps || std::abs(diff.z) > eps) && l <= 15) {
					l++;
					divides.clear();
					randoms.clear();
					divides.push_back(xstart);

					for (int n = 0; n < 4* number_of_divisions; n++) {
						randoms.push_back(rand() % 1000);
					}

					std::sort(randoms.begin(), randoms.end());
					auto len = xend - xstart;

					for (int n = 0; n < number_of_divisions; n++) {
						divides.push_back({
							(randoms[4 * n] / 1000.f) * len.x + xstart.x,
							(randoms[4 * n + 1] / 1000.f) * len.y + xstart.y,
							(randoms[4 * n + 2] / 1000.f) * len.z + xstart.z,
							(randoms[4 * n + 3] / 1000.f) * len.w + xstart.w });
					}

					divides.push_back(xend);

					float min_len = 100000.0f;
					int min_first_u = 0;
					int min_first_v = 0;
					int min_second_u = 0;
					int min_second_v = 0;

					for (int g = 0; g < divides.size(); g++) {
						for (int z = 0; z < divides.size(); z++) {
							for (int h = 0; h < divides.size(); h++) {
								for (int r = 0; r < divides.size(); r++) {
									diff = first[i](divides[g].x, divides[z].y) - second[j](divides[h].z, divides[r].w);
									if (glm::length(diff) < min_len) {
										min_len = glm::length(diff);
										min_first_u = g;
										min_first_v = z;
										min_second_u = h;
										min_second_v = r;
									}
								}
							}
						}
					}

					x = { divides[min_first_u].x,divides[min_first_v].y,divides[min_second_u].z,divides[min_second_v].w };
					xstart = {
						divides[std::max(min_first_u - 1,0)].x,
						divides[std::max(min_first_v - 1,0)].y,
						divides[std::max(min_second_u - 1,0)].z,
						divides[std::max(min_second_v - 1,0)].w };
					xend = {
						divides[std::min(min_first_u + 1, (int)(divides.size()) - 1)].x,
						divides[std::min(min_first_v + 1, (int)(divides.size()) - 1)].y,
						divides[std::min(min_second_u + 1,(int)(divides.size()) - 1)].z,
						divides[std::min(min_second_v + 1,(int)(divides.size()) - 1)].w };

					diff = first[i](x.x, x.y) - second[j](x.z, x.w);
				}

				diff = first[i](x.x, x.y) - second[j](x.z, x.w);
				if (glm::length(diff) < eps)
				{

					return { x, {i,j} };
				}
			}
		}
		return { glm::vec4(-1, 0, 0, 0),{0,0} };
	}
	else { //size() == 1
		auto first = obj[0]->GetParametrisations();
		for (int i = 0; i < first.size(); i++) {
			for (int j = i; j < first.size(); j++) {
				int l = 0;
				float eps = 0.001f;
				auto x = glm::vec4(rand()%1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f);
				auto xstart = glm::vec4(0, 0, 0, 0);
				auto xend = glm::vec4(1, 1, 1, 1);
				
				std::vector<glm::vec4> divides = {};
				std::vector<int> randoms = {};

				if (serch_for_intersections_using_cursor) {
					xstart.x = 1;
					xstart.y = 1;
					xend.x = 0;
					xend.y = 0;
					float stride = 1.0f / (number_of_divisions - 1);
					float u = 0, v = 0;
					auto cursor_pos = cursor->GetPosition();
					for (int s = 0; s < number_of_divisions; s++) {
						for (int f = 0; f < number_of_divisions; f++) {
							if (glm::length(cursor_pos - first[i](u, v)) < eps_cursor) {
								xstart.x = min(u, xstart.x);
								xstart.y = min(v, xstart.y);
								xend.x = max(u, xend.x);
								xend.y = max(v, xend.y);
							}
							v += stride;
						}
						u += stride;
						v = 0;
					}
				}

				auto diff = first[i](x.x, x.y) - first[j](x.z, x.w);

				do {
					l++;
					divides.clear();
					randoms.clear();
					divides.push_back(xstart);

					for (int n = 0; n < 4* number_of_divisions; n++) {
						randoms.push_back(rand() % 1000);
					}

					std::sort(randoms.begin(), randoms.end());
					auto len = xend - xstart;
					if (len.x < 0 || len.y < 0 || len.z < 0 || len.w < 0) break;
					for (int n = 0; n < number_of_divisions; n++) {
						divides.push_back({
							(randoms[4 * n] / 1000.f) * len.x + xstart.x,
							(randoms[4 * n + 1] / 1000.f) * len.y + xstart.y,
							(randoms[4 * n + 2] / 1000.f) * len.z + xstart.z,
							(randoms[4 * n + 3] / 1000.f) * len.w + xstart.w });
					}

					divides.push_back(xend);

					float min_len = 100000.0f;
					int min_first_u = 0;
					int min_first_v = 0;
					int min_second_u = 0;
					int min_second_v = 0;

					for (int g = 0; g < divides.size(); g++) {
						for (int z = 0; z < divides.size(); z++) {
							for (int h = 0; h < divides.size(); h++) {
								for (int r = 0; r < divides.size(); r++) {
									diff = first[i](divides[g].x, divides[z].y) - first[j](divides[h].z, divides[r].w);
									if (glm::length(diff) < min_len) {
										if (i == j && abs(divides[g].x - divides[h].z) < 0.1f && abs(divides[z].y - divides[r].w) < 0.1f) continue;
										min_len = glm::length(diff);
										min_first_u = g;
										min_first_v = z;
										min_second_u = h;
										min_second_v = r;
									}
								}
							}
						}
					}

					x = { divides[min_first_u].x,divides[min_first_v].y,divides[min_second_u].z,divides[min_second_v].w };
					xstart = {
						divides[std::max(min_first_u - 1,0)].x,
						divides[std::max(min_first_v - 1,0)].y,
						divides[std::max(min_second_u - 1,0)].z,
						divides[std::max(min_second_v - 1,0)].w };
					xend = {
						divides[std::min(min_first_u + 1, (int)(divides.size()) - 1)].x,
						divides[std::min(min_first_v + 1, (int)(divides.size()) - 1)].y,
						divides[std::min(min_second_u + 1,(int)(divides.size()) - 1)].z,
						divides[std::min(min_second_v + 1,(int)(divides.size()) - 1)].w };

					diff = first[i](x.x, x.y) - first[j](x.z, x.w);
				} while (glm::length(diff) > eps && l <= 15);

				diff = first[i](x.x, x.y) - first[j](x.z, x.w);
				if (glm::length(diff) < eps)
				{
					if (i == j && x.x == x.z && x.y == x.w) return { glm::vec4(-1, 0, 0, 0),{-1,-1} };
					return { x, {i,j} };
				}
			} 
		}
		return { glm::vec4(-1, 0, 0, 0),{-1,-1} };
	}
}

glm::mat4x4 createJacobian(glm::vec4 values, glm::vec3 t,
	std::function<glm::vec3(double, double)>& fu, std::function<glm::vec3(double, double)>& fv,
	std::function<glm::vec3(double, double)>& qu, std::function<glm::vec3(double, double)>& qv) {

	auto fu_ = fu(values.x, values.y);
	auto fv_ = fv(values.x, values.y);
	auto qu_ = qu(values.z, values.w);
	auto qv_ = qv(values.z, values.w);

	glm::mat4x4 res = {
		{fu_.x, fu_.y, fu_.z, glm::dot(fu_,t)},
		{fv_.x, fv_.y, fv_.z, glm::dot(fv_,t)},
		{-qu_.x, -qu_.y, -qu_.z, 0},
		{-qv_.x, -qv_.y, -qv_.z, 0}
	};

	return res;
}

glm::vec4 evaluateFfunction(glm::vec4 values, glm::vec3 t, glm::vec3 P0,
	std::function<glm::vec3(double, double)>& f, std::function<glm::vec3(double, double)>& q) {
	return { f(values.x,values.y) - q(values.z,values.w),glm::dot(f(values.x,values.y) - P0,t) - distance_d };
}

void look_for_other_intersection_points(glm::vec4 start_values, glm::uvec2 functions, std::vector<std::shared_ptr<Object>> obj) {

	auto intersection_obj = std::make_shared<Intersection>(ourShader, obj.front(), obj.back());
	intersection_obj->screen_width = &width_;
	intersection_obj->screen_height = &height_;
	auto f = obj.front()->GetParametrisations()[functions.x];
	auto fu = obj.front()->GetUParametrisations()[functions.x];
	auto fv = obj.front()->GetVParametrisations()[functions.x];

	auto q = obj.back()->GetParametrisations()[functions.y];
	auto qu = obj.back()->GetUParametrisations()[functions.y];
	auto qv = obj.back()->GetVParametrisations()[functions.y];
	float eps = 0.001f;

	bool do_other_direction = false;
	float direction = 1.0f;

	std::vector<glm::vec3> points_on_intersection{};

	glm::vec4 x = start_values;
	glm::vec4 x1;
	glm::vec3 P0 = f(x.x, x.y);
	points_on_intersection.push_back(f(x.x, x.y));
	int let_one_more_point = 0;
	bool do_not_add_next_point = false;
	do {
		x = start_values;
		while (points_on_intersection.size() < 2 ||
			glm::length(points_on_intersection[0] - points_on_intersection.back()) > distance_d * 0.75f|| let_one_more_point<1)
		{
			if (!(points_on_intersection.size() < 2 ||
				glm::length(points_on_intersection[0] - points_on_intersection.back()) > distance_d * 0.75f))
				let_one_more_point++;
			if (glm::length(f(x.x, x.y) - q(x.z, x.w)) <= 10*eps) {
				intersection_obj->AddPoints(std::make_shared<Point>(f(x.x, x.y), glm::vec4(0, 1, 1, 1), ourShader), std::make_shared<Point>(q(x.z, x.w), glm::vec4(0, 1, 0, 1), ourShader));
				intersection_obj->AddParameters({ x.x,x.y }, { x.z,x.w });
			}
			else break;
			P0 = f(x.x, x.y);
			glm::vec3 nf = glm::normalize(glm::cross(fu(x.x, x.y), fv(x.x, x.y)));
			glm::vec3 nq = glm::normalize(glm::cross(qu(x.z, x.w), qv(x.z, x.w)));
			glm::vec3 t = direction * glm::normalize(glm::cross(nf, nq));
			int l = 0;
			do {
				l++;
				auto jac = createJacobian(x, t, fu, fv, qu, qv);
				auto jac_i = glm::inverse(jac);
				auto func = evaluateFfunction(x, t, P0, f, q);
				//x1 = solveGauss(jac,x - func ); // gauss is broken :( - will debug in free time :D
				x1 = x - jac_i * func;
				x = x1;
				if (x.x < 0) x.x += 1.0f;
				if (x.y < 0) x.y += 1.0f;
				if (x.z < 0) x.z += 1.0f;
				if (x.w < 0) x.w += 1.0f;

				if (x.x > 1) x.x -= 1.0f;
				if (x.y > 1) x.y -= 1.0f;
				if (x.z > 1) x.z -= 1.0f;
				if (x.w > 1) x.w -= 1.0f;
			} while (glm::length(f(x.x, x.y) - q(x.z, x.w)) > eps && l <= 25);


			if (x.x < 0 || x.x > 1 ||
				x.y < 0 || x.y > 1 ||
				x.z < 0 || x.z > 1 ||
				x.w < 0 || x.w > 1) {
				break;
			}

			if (glm::length(f(x.x, x.y) - q(x.z, x.w)) > 10*eps ||
				glm::length(points_on_intersection.back() - f(x.x, x.y)) > 3.0f * distance_d ||
				glm::length(points_on_intersection.back() - q(x.z, x.w)) > 3.0f * distance_d) break;

			points_on_intersection.push_back(f(x.x, x.y));
			
		}
		if (glm::length(f(x.x, x.y) - q(x.z, x.w)) <= eps) {
			intersection_obj->AddPoints(std::make_shared<Point>(f(x.x, x.y), glm::vec4(0, 1, 1, 1), ourShader), std::make_shared<Point>(q(x.z, x.w), glm::vec4(0, 1, 0, 1), ourShader));
			intersection_obj->AddParameters({ x.x,x.y }, { x.z,x.w });
		}
		intersection_obj->Reverse();
		std::reverse(points_on_intersection.begin(), points_on_intersection.end());
		direction *= -1.0f;
		do_other_direction = !do_other_direction;
	} while (do_other_direction);
	objects_list.push_back(intersection_obj);
	intersection_obj->create_texture();
}

void adding_menu(std::vector<std::shared_ptr<Object>>& objects, glm::vec3 starting_pos) {
	if (ImGui::Button("Torus")) {
		objects.push_back(std::make_shared<Torus>(0.5, 0.1, 10, 10, glm::vec4(1, 1, 0, 1), ourShader));
		objects.back()->screen_height = &height_;
		objects.back()->screen_width = &width_;
		objects.back()->MoveObject(starting_pos);
	}
	ImGui::SameLine();
	if (ImGui::Button("Point")) {
		auto point = std::make_shared<Point>(starting_pos, glm::vec4(1, 1, 0, 1), ourShader);
		point->screen_height = &height_;
		point->screen_width = &width_;
		for (auto& obj : objects)
		{
			if (obj->selected) {
				auto bez = std::dynamic_pointer_cast<Bezier>(obj);
				if (bez) {
					bez->AddPointToCurve(point);
				}
			}
		}
		objects.push_back(point);
	}
	ImGui::SameLine();
	if (ImGui::Button("BezierC0")) {
		auto sh = std::make_shared<BezierC0>(ourShader);
		sh->screen_height = &height_;
		sh->screen_width = &width_;
		for (auto& obj : objects) {
			if (obj->selected) {
				auto pt = std::dynamic_pointer_cast<Point>(obj);
				if (pt) {
					sh->AddPointToCurve(pt);
				}
			}
		}
		objects.push_back(sh);
	}
	ImGui::SameLine();
	if (ImGui::Button("BezierC2")) {
		auto sh = std::make_shared<BezierC2>(ourShader);
		sh->screen_height = &height_;
		sh->screen_width = &width_;
		for (auto& obj : objects) {
			if (obj->selected) {
				auto pt = std::dynamic_pointer_cast<Point>(obj);
				if (pt) {
					sh->AddPointToCurve(pt);
				}
			}
		}
		objects.push_back(sh);
	}
	ImGui::SameLine();
	if (ImGui::Button("BezierInterpol")) {
		auto sh = std::make_shared<BezierInterpol>(ourShader);
		sh->screen_height = &height_;
		sh->screen_width = &width_;
		for (auto& obj : objects) {
			if (obj->selected) {
				auto pt = std::dynamic_pointer_cast<Point>(obj);
				if (pt) {
					sh->AddPointToCurve(pt);
				}
			}
		}
		objects.push_back(sh);
	}
	if (ImGui::CollapsingHeader("BezierFlakeC0")) {
		static int planar = 0;
		ImGui::RadioButton("plain", &planar, 0);
		ImGui::RadioButton("barrel", &planar, 1);
		static float dimensions[2] = { 1,1 };
		if (planar == 0) {
			ImGui::InputFloat2("set width and height of the plane", dimensions);
		}
		else {
			ImGui::InputFloat2("set radius and height of the barell", dimensions);
		}
		static int patches[2] = { 1,1 };
		ImGui::DragInt2("Set number of patches in u and v dimensions", patches, 0.5f, 1, 100);
		if (ImGui::Button("Create flake!!!")) {
			objects.push_back(std::make_shared<BezierFlakeC0>(ourShader, planar, glm::uvec2(patches[0], patches[1]), glm::vec2(dimensions[0], dimensions[1])));

		}
	}
	if (ImGui::CollapsingHeader("BezierFlakeC2")) {
		static int planar = 0;
		ImGui::RadioButton("plain", &planar, 0);
		ImGui::RadioButton("barrel", &planar, 1);
		static float dimensions[2] = { 1,1 };
		if (planar == 0) {
			ImGui::InputFloat2("set width and height of the plane", dimensions);
		}
		else {
			ImGui::InputFloat2("set radius and height of the barell", dimensions);
		}
		static int patches[2] = { 1,1 };
		ImGui::DragInt2("Set number of patches in u and v dimensions", patches, 0.5f, 1, 100);
		if (ImGui::Button("Create flake!!!")) {
			objects.push_back(std::make_shared<BezierFlakeC2>(ourShader, planar, glm::uvec2(patches[0], patches[1]), glm::vec2(dimensions[0], dimensions[1])));

		}
	}
	if (ImGui::CollapsingHeader("Intersection adding")) {
		ImGui::Checkbox("Use cursor to point intersection", &serch_for_intersections_using_cursor);
		ImGui::InputFloat("Set D", &distance_d);
		ImGui::SliderInt("Set number of diuvisions", &number_of_divisions,4,50);
		if (ImGui::Button("Intersection")) {
			auto selected_objects = std::vector<std::shared_ptr<Object>>();
			for (auto& obj : objects_list) {
				if (dynamic_cast<BezierFlakeC0*>(obj.get()) || dynamic_cast<Torus*>(obj.get())) {
					if (obj->selected) {
						selected_objects.push_back(obj);
					}
				}
			}
			if (selected_objects.size() > 2 || selected_objects.size() <= 0) return;

			auto intersection = look_for_intersection(selected_objects);
			if (intersection.first.x < 0) return;

			look_for_other_intersection_points(intersection.first, intersection.second,selected_objects);
		}
	}
}

void add_selected_points_to_selected_curve() {
	std::shared_ptr<Bezier> bez;
	for (auto& obj : objects_list)
	{
		if (obj->selected) {
			bez = std::dynamic_pointer_cast<Bezier>(obj);
			if (bez) {
				break;
			}
		}
	}

	if (!bez) return;

	for (auto& obj : objects_list)
	{
		if (obj->selected) {
			auto point = std::dynamic_pointer_cast<Point>(obj);
			if (point) {
				if (bez) {
					bez->AddPointToCurve(point);
				}
			}
		}
	}
}

void objects_on_scene_gui() {
	int to_delete = -1;
	if (ImGui::CollapsingHeader("Objects Present on Scene")) {
		int i = 0;
		for (auto& ob : objects_list) {
			ob->CreateMenu();
			ImGui::SameLine();
			if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
				to_delete = i;
			}
			i++;
		}
	}
	if (to_delete > -1) {
		objects_list.erase(objects_list.begin() + to_delete);
		to_delete = -1;
		std::for_each(objects_list.begin(), objects_list.end(), [](std::shared_ptr<Object> obj) {obj->Update(); });
	}
}

void adding_new_objects_gui() {
	if (ImGui::CollapsingHeader("Add New Objects")) {
		adding_menu(objects_list, cursor->GetPosition());
		if (ImGui::Button("Add selected points to curve")) {
			add_selected_points_to_selected_curve();
		}
	}
}

void cursor_position_gui() {
	ImGui::Text("Cursor Position");
	auto pos = cursor->GetPosition();
	ImGui::Text("world Position");
	ImGui::InputFloat("X", &(pos.x));
	ImGui::InputFloat("Y", &(pos.y));
	ImGui::InputFloat("Z", &(pos.z));
	cursor->SetCursorPosition(pos);
	glm::vec4 screen_pos = { pos,1.0f };
	screen_pos = projection * view * screen_pos;
	screen_pos /= screen_pos.w;
	glm::vec2 real_screenpos = {
		(screen_pos.x + 1.0f) * width_ / 2.0f,
		(-screen_pos.y + 1.0f) * height_ / 2.0f,
	};
	glm::vec2 real_screen_pos_tmp = real_screenpos;
	ImGui::Text("screen position");
	ImGui::InputFloat("X##posx", &(real_screenpos.x));
	ImGui::InputFloat("Y##posy", &(real_screenpos.y));
	if (real_screenpos != real_screen_pos_tmp) {
		glm::vec3 start;
		glm::vec3 end;
		transform_screen_coordinates_to_world(end, start, real_screenpos.x, real_screenpos.y);
		glm::vec3 se = end - start;
		glm::vec3 position = start + se * (glm::dot(-cameraFront, start - lookAt) / glm::dot(-se, -cameraFront));
		cursor->SetCursorPosition(position);
	}

	ImGui::Text("Center Cursor positions");
	auto pos2 = center->GetPosition();
	ImGui::Text("world Position");
	ImGui::InputFloat("X##ab", &(pos2.x));
	ImGui::InputFloat("Y##ab", &(pos2.y));
	ImGui::InputFloat("Z##ab", &(pos2.z));
	center->SetCursorPosition(pos2);
}

void choosing_point_of_transformation_gui() {
	ImGui::Text("Select point of transformations");
	ImGui::RadioButton("Center of selected", &e, 0); ImGui::SameLine();
	ImGui::RadioButton("Cursor position", &e, 1);
}

void stereoscopic_settings() {
	if (ImGui::CollapsingHeader("Stereoscopic settings")) {
		ImGui::Checkbox("Enable Anaglyph", &stereoscopic);
		ImGui::SliderFloat("Focus Distance", &d, 0.1f, 10.0f);
		ImGui::SliderFloat("Eye Distance", &ipd, 0.001f, 1.0f);
	}
}

void main_menu() {
	// Menu Bar
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu##main"))
		{
			if (ImGui::MenuItem("Open##main", "Ctrl+O")) {
				std::vector<std::string> node_names = {};
				std::vector<std::shared_ptr<Object>> objects_in_file = {};
				//std::string file_to_open = "C:\\Users\\miles\\Desktop\\MG_CC_sem_1\\MG-1\\laby\\SimpleCAD_CAM\\x64\\Debug\\example.txt";
				std::string file_to_open = "example.txt";
				xml_document<> doc;
				xml_node<>* root_node;
				// Read the xml file into a vector
				ifstream theFile(file_to_open);
				vector<char> buffer((istreambuf_iterator<char>(theFile)), istreambuf_iterator<char>());
				buffer.push_back('\0');
				// Parse the buffer using the xml file parsing library into doc 
				doc.parse<0>(&buffer[0]);
				// Find our root node
				root_node = doc.first_node("Scene");
				for (xml_node<>* point_node = root_node->first_node("Point"); point_node; point_node = point_node->next_sibling("Point"))
				{
					auto position = point_node->first_node("Position");
					glm::vec3 pos = glm::vec3(std::atof(position->first_attribute("X")->value()),
						std::atof(position->first_attribute("Y")->value()),
						std::atof(position->first_attribute("Z")->value()));
					auto point = make_shared<Point>(pos, glm::vec4(0.5f, 0.0, 0.5f, 1), ourShader);
					point->SetName(point_node->first_attribute("Name")->value());
					point->screen_height = &height_;
					point->screen_width = &width_;
					objects_in_file.push_back(point);
				}

				for (xml_node<>* object_node = root_node->first_node(); object_node; object_node = object_node->next_sibling())
				{
					std::string node_type = object_node->name();
					if (node_type == "Point") continue;

					if (node_type == "Torus") {
						float R = std::atof(object_node->first_attribute("MajorRadius")->value());
						float r = std::atof(object_node->first_attribute("MinorRadius")->value());
						int H = std::atoi(object_node->first_attribute("MajorSegments")->value());
						int h = std::atoi(object_node->first_attribute("MinorSegments")->value());
						auto position = object_node->first_node("Position");
						glm::vec3 pos = glm::vec3(std::atof(position->first_attribute("X")->value()),
							std::atof(position->first_attribute("Y")->value()),
							std::atof(position->first_attribute("Z")->value()));
						auto rotation = object_node->first_node("Rotation");
						glm::quat rot = glm::quat(std::atof(rotation->first_attribute("W")->value()),
							std::atof(rotation->first_attribute("X")->value()),
							std::atof(rotation->first_attribute("Y")->value()),
							std::atof(rotation->first_attribute("Z")->value()));
						auto scale = object_node->first_node("Scale");
						glm::vec3 sc = glm::vec3(std::atof(scale->first_attribute("X")->value()),
							std::atof(scale->first_attribute("Y")->value()),
							std::atof(scale->first_attribute("Z")->value()));
						auto object = std::make_shared<Torus>(R, r, h, H, glm::vec4(0.7f, 0.7f, 0.7f, 0.5f), ourShader);
						object->MoveObjectTo(pos);
						object->RotateObject(rot);
						object->ResizeObject(sc);
						object->screen_height = &height_;
						object->screen_width = &width_;
						object->SetName(object_node->first_attribute("Name")->value());
						objects_in_file.push_back(object);
					}
					else if (node_type == "BezierC0") {
						auto object = std::make_shared<BezierC0>(ourShader);
						object->screen_height = &height_;
						object->screen_width = &width_;
						auto points_list = object_node->first_node("Points");
						for (xml_node<>* point_node = points_list->first_node("PointRef"); point_node; point_node = point_node->next_sibling("PointRef")) {
							string ref = point_node->first_attribute("Name")->value();
							for (const auto& obj : objects_in_file) {
								if (obj->CompareName(ref)) {
									auto point = std::dynamic_pointer_cast<Point>(obj);
									object->AddPointToCurve(point);
									break;
								}
							}
						}
						object->SetName(object_node->first_attribute("Name")->value());
						objects_in_file.push_back(object);
					}
					else if (node_type == "BezierC2") {
						auto object = std::make_shared<BezierC2>(ourShader);
						auto points_list = object_node->first_node("Points");
						for (xml_node<>* point_node = points_list->first_node("PointRef"); point_node; point_node = point_node->next_sibling("PointRef")) {
							string ref = point_node->first_attribute("Name")->value();
							for (const auto& obj : objects_in_file) {
								if (obj->CompareName(ref)) {
									auto point = std::dynamic_pointer_cast<Point>(obj);
									object->AddPointToCurve(point);
									break;
								}
							}
						}
						object->screen_height = &height_;
						object->screen_width = &width_;
						object->SetName(object_node->first_attribute("Name")->value());
						objects_in_file.push_back(object);
					}
					else if (node_type == "BezierInter") {
						auto object = std::make_shared<BezierInterpol>(ourShader);
						auto points_list = object_node->first_node("Points");
						for (xml_node<>* point_node = points_list->first_node("PointRef"); point_node; point_node = point_node->next_sibling("PointRef")) {
							string ref = point_node->first_attribute("Name")->value();
							for (const auto& obj : objects_in_file) {
								if (obj->CompareName(ref)) {
									auto point = std::dynamic_pointer_cast<Point>(obj);
									object->AddPointToCurve(point);
									break;
								}
							}
						}
						object->screen_height = &height_;
						object->screen_width = &width_;
						object->SetName(object_node->first_attribute("Name")->value());
						objects_in_file.push_back(object);
					}
					else if (node_type == "PatchC0") {

						auto points_list = object_node->first_node("Points");
						std::vector<std::shared_ptr<Point>> patch_points = {};
						for (xml_node<>* point_node = points_list->first_node("PointRef"); point_node; point_node = point_node->next_sibling("PointRef")) {
							string ref = point_node->first_attribute("Name")->value();
							for (const auto& obj : objects_in_file) {
								if (obj->CompareName(ref)) {
									auto point = std::dynamic_pointer_cast<Point>(obj);
									patch_points.push_back(point);
									break;
								}
							}
						}
						glm::uvec2 flakes = { std::atoi(object_node->first_attribute("M")->value()),std::atoi(object_node->first_attribute("N")->value()) };
						glm::uvec2 divisions = { std::atoi(object_node->first_attribute("MSlices")->value()),std::atoi(object_node->first_attribute("NSlices")->value()) };
						auto object = std::make_shared<BezierFlakeC0>(ourShader, flakes, divisions, patch_points);
						object->screen_height = &height_;
						object->screen_width = &width_;
						for (auto& obj : patch_points) {
							obj->AddOwner(object);
						}
						object->SetName(object_node->first_attribute("Name")->value());
						objects_in_file.push_back(object);
					}
					else if (node_type == "PatchC2") {
						auto points_list = object_node->first_node("Points");
						std::vector<std::shared_ptr<Point>> patch_points = {};
						for (xml_node<>* point_node = points_list->first_node("PointRef"); point_node; point_node = point_node->next_sibling("PointRef")) {
							string ref = point_node->first_attribute("Name")->value();
							for (const auto& obj : objects_in_file) {
								if (obj->CompareName(ref)) {
									auto point = std::dynamic_pointer_cast<Point>(obj);
									patch_points.push_back(point);
									break;
								}
							}
						}
						glm::uvec2 flakes = { std::atoi(object_node->first_attribute("M")->value()),std::atoi(object_node->first_attribute("N")->value()) };
						glm::uvec2 divisions = { std::atoi(object_node->first_attribute("MSlices")->value()),std::atoi(object_node->first_attribute("NSlices")->value()) };
						auto object = std::make_shared<BezierFlakeC2>(ourShader, flakes, divisions, patch_points);
						object->screen_height = &height_;
						object->screen_width = &width_;
						for (auto& obj : patch_points) {
							obj->AddOwner(object);
						}
						object->SetName(object_node->first_attribute("Name")->value());
						objects_in_file.push_back(object);
					}
				}
				objects_list.insert(objects_list.end(), objects_in_file.begin(), objects_in_file.end());
			}
			if (ImGui::MenuItem("Save##main", "Ctrl+O")) {
				xml_document<char> document;
				xml_node<>* scene = document.allocate_node(node_element, "Scene");
				for (auto& obj : objects_list) {
					obj->Serialize(document, scene);
				}
				document.append_node(scene);

				ofstream myfile;
				myfile.open("example.txt");
				myfile << document;
				myfile.close();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void move_selected_to_cursor() {
	if (ImGui::Button("Move Selected to cursor possition")) {
		glm::vec3 odn = center->GetPosition();
		switch (e) {
		case 0:
			odn = center->GetPosition();
			break;
		case 1:
			odn = cursor->GetPosition();
			break;
		default:
			return;
			break;
		}

		for (auto& obj : objects_list) {
			if (obj->selected) {
				obj->MoveObjectTo(odn);
			}
			for (auto& vir : obj->GetVirtualObjects()) {
				if (vir->selected) {
					vir->MoveObjectTo(odn);
				}
			}
		}
	}
}

void remove_unnecessary_points() {
	int point_to_erase = -1;
	for (int i = 0; i < objects_list.size(); i++) {
		if (objects_list[i].use_count() > 1) {
			point_to_erase = i;
			break;
		}
	}
	if (point_to_erase > -1)
		objects_list.erase(objects_list.begin() + point_to_erase);
	else return;
	remove_unnecessary_points();
}

void merge_selected_points() {
	auto selected_points = std::vector<std::shared_ptr<Point>>();
	glm::vec3 center_pos = { 0,0,0 };
	for (auto& obj : objects_list) {
		if (dynamic_cast<Point*>(obj.get())) {
			if (obj->selected) {
				selected_points.push_back(std::dynamic_pointer_cast<Point>(obj));
				center_pos += obj->GetPosition();
			}
		}
		for (auto& virt : obj->GetVirtualObjects()) {
			auto vr_point = dynamic_pointer_cast<Point>(virt);
			if (vr_point->selected) {
				selected_points.push_back(vr_point);
				center_pos += vr_point->GetPosition();
			}
		}
	}

	if (selected_points.size() == 0) return;

	center_pos /= selected_points.size();

	auto& left = selected_points.front();
	left->MoveObjectTo(center_pos);
	for (int i = 1; i < selected_points.size();i++) {
		auto& pointPtr = selected_points[i];
		auto point = pointPtr.get();
		point->UpdateOwners(left);
		point->Update();
		left->AddUniqueOwners(point->owners);
		for (int j = 0; j < objects_list.size(); j++) {
			auto point_o = objects_list[j];
			if (point_o->CompareName(point->constname)) {
				objects_list.erase(objects_list.begin() + j);
			}
		}
	}
	left->Update();
	left->InformOwners();
}

void fill_the_selected_hole() {
	std::vector<shared_ptr<BezierFlakeC0>> bezier_patches = {};
	for (auto& obj : objects_list) {
		auto br = std::dynamic_pointer_cast<BezierFlakeC0>(obj);
		if (br && br->selected) {
			bezier_patches.push_back(br);
		}
	}
	//if (bezier_patches.size() > 3) return;

	auto patches = std::vector<std::vector<std::vector<std::shared_ptr<Point>>>>();

	for (auto& patch : bezier_patches) {
		auto res = patch->GetAllPatches();
		patches.insert(patches.end(), res.begin(), res.end());
	}

	auto gregory = std::make_shared<TriangularGregoryPatch>(patches, ourShader);

	if (gregory->IsProper()) {
		gregory->UpdateOwnership();
		objects_list.push_back(gregory);
	}
}

void create_gui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
	bool open;
	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_MenuBar;
	ImGui::Begin("Main Menu##uu", &open, flags);
	main_menu();
	stereoscopic_settings();
	cursor_position_gui();
	choosing_point_of_transformation_gui();
	move_selected_to_cursor();
	adding_new_objects_gui();
	if (ImGui::Button("Erase unused points")) remove_unnecessary_points();
	if (ImGui::Button("Merge Selected points")) merge_selected_points();
	if (ImGui::Button("Fill hole with gregory patch")) fill_the_selected_hole();
	objects_on_scene_gui();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}
