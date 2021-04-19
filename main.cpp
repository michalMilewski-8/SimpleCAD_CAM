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
#include "Virtual.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define EPS 0.1
#define PRECISION 1.0f

glm::vec3 cameraPos, cameraFront, cameraUp, lookAt, moving_up;
unsigned int width_, height_;

int e = 0;
glm::mat4 projection, view, model, mvp;
glm::mat4 projection_i, view_i, model_i, mvp_i;
glm::vec2 mousePosOld, angle;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
Camera cam;
Shader ourShader;
Cursor cursor, center;

std::vector<std::shared_ptr<Object>> objects_list = {};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void adding_menu(std::vector<std::shared_ptr<Object>>& objects, glm::vec3 starting_pos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void transform_screen_coordinates_to_world(glm::vec3& world_coordinates_end, glm::vec3& world_coordinates_start, float x_pos, float y_pos);
void add_selected_points_to_selected_curve();
void create_gui();

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
	cam.SetPerspective(glm::radians(45.0f), DEFAULT_WIDTH / (float)DEFAULT_HEIGHT, 1.0f, 20.0f);
	//cam->SetOrthographic(-1, 1, 1, -1, -1, 1);

	objects_list.push_back(std::make_shared<Torus>(Torus(0.5, 0.1, 10, 10, { 1,1,0,1 }, ourShader)));
	objects_list.back()->screen_height = &height_;
	objects_list.back()->screen_width = &width_;

	glEnable(GL_DEPTH_TEST);

	cursor = Cursor(ourShader);
	center = Cursor(ourShader);

	cursor.SetCursorPosition(lookAt);
	float vertices[] = {
	  -0.5f, 0.0f, -0.5f,
	  -0.5f, 0.0f, +0.5f,
	  +0.5f, 0.0f, +0.5f,
	  +0.5f, 0.0f, -0.5f,
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


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

		//int projectionLoc = glGetUniformLocation(gridShader.ID, "proj");
		//glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		//int viewLoc = glGetUniformLocation(gridShader.ID, "vieww");
		//glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(view));

		//gridShader.use();
		//glBindVertexArray(VAO);
		//glDrawArrays(GL_QUADS, 0, 4);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		cursor.DrawObject(mvp);


		int number_of_selected = 0;
		glm::vec3 center_point = glm::vec3(0.0f);
		for (auto& ob : objects_list) {
			if (ob->selected) {
				if (std::dynamic_pointer_cast<Bezier>(ob)) continue;
				number_of_selected++;
				center_point += ob->GetPosition();
			}
		}
		if (number_of_selected > 0) {
			center_point /= number_of_selected;
			center.SetCursorPosition(center_point);
			center.DrawObject(mvp);
		}

		for (auto& ob : objects_list) {
			ob->DrawObject(mvp);
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
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	cam.SetPerspective(glm::radians(45.0f), width / (float)height, 1.0f, 20.0f);
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
				if (virt->SelectedVirt())
				{
					virt->UnSelectVirt();
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
		glm::vec3 odn = center.GetPosition();
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			switch (e) {
			case 0:
				odn = center.GetPosition();
				break;
			case 1:
				odn = cursor.GetPosition();
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
					if (virt->SelectedVirt())
					{
						glm::vec4 pos2 = { virt->getPosition() - odn, 0.0f };
						virt->MoveVirtObjectTo(odn + static_cast<glm::vec3>(roation * pos2));
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
		float cameraSpeed = 0.4f * deltaTime;

		glm::vec2 movement = diff * cameraSpeed;

		glm::vec3 right_movement = cam.GetRightVector() * movement.x;
		glm::vec3 up_movement = cam.GetUpVector() * -movement.y;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			for (auto& obj : objects_list) {
				if (obj->selected)
					obj->MoveObject(-right_movement + -up_movement);
				for (auto& virt : obj->GetVirtualObjects()) {
					if (virt->SelectedVirt())
						virt->MoveVirtObject(-right_movement + -up_movement);
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
		cursor.MoveObject(right_movement + up_movement);
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
				auto vr_point = dynamic_cast<VirtualPoint*>(virt);
				if (vr_point) {
					glm::vec3 point2 = vr_point->GetPosition();
					glm::vec3 toPoint2(point2 - glm::vec3(lRayStart_world));
					glm::vec3 crossp2 = glm::cross(lRayDir_world, toPoint2);
					float length = glm::length(crossp2);
					if (length < minLength && length < 0.1f)
					{
						minLength = length;
						closest = vr_point;
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
		//TODO: przerobi� to na co� �adniejszego;
		//cam.ScaleWorld({ movement,movement,movement });
		cameraFront = glm::normalize(lookAt - cameraPos);
		float dist = glm::length(lookAt - cameraPos);
		cameraPos = lookAt - (cameraFront * dist * movement);
		cam.LookAt(cameraPos, cameraFront, cameraUp);
	}

	//cam.Zoom(yoffset * precision);

}

void adding_menu(std::vector<std::shared_ptr<Object>>& objects, glm::vec3 starting_pos) {
	if (ImGui::Button("Torus")) {
		objects.push_back(std::make_shared<Torus>(Torus(0.5, 0.1, 10, 10, { 1,1,0,1 }, ourShader)));
		objects.back()->screen_height = &height_;
		objects.back()->screen_width = &width_;
		objects.back()->MoveObject(starting_pos);
	}
	ImGui::SameLine();
	if (ImGui::Button("Point")) {
		auto point = std::make_shared<Point>(Point(starting_pos, { 1,1,0,1 }, ourShader));
		point->screen_height = &height_;
		point->screen_width = &width_;
		for(auto& obj : objects)
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
		auto sh = std::make_shared<BezierC0>(BezierC0(ourShader));
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
		auto sh = std::make_shared<BezierC2>(BezierC2(ourShader));
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
		adding_menu(objects_list, cursor.GetPosition());
		if (ImGui::Button("Add selected points to curve")) {
			add_selected_points_to_selected_curve();
		}
	}
}

void cursor_position_gui() {
	ImGui::Text("Cursor Position");
	auto pos = cursor.GetPosition();
	ImGui::Text("world Position");
	ImGui::InputFloat("X", &(pos.x));
	ImGui::InputFloat("Y", &(pos.y));
	ImGui::InputFloat("Z", &(pos.z));
	cursor.SetCursorPosition(pos);
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
		cursor.SetCursorPosition(position);
	}
}

void choosing_point_of_transformation_gui() {
	ImGui::Text("Select point of transformations");
	ImGui::RadioButton("Center of selected", &e, 0); ImGui::SameLine();
	ImGui::RadioButton("Cursor position", &e, 1);
}

void create_gui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();
	ImGui::Begin("Main Menu");
	cursor_position_gui();
	choosing_point_of_transformation_gui();
	adding_new_objects_gui();
	objects_on_scene_gui();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}
