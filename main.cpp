#include <stdio.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <vector>

#include "Shader.h"
#include "Camera.h"
#include "Cursor.h"
#include "Torus.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define EPS 0.1
#define PRECISION 1.0f

glm::vec3 cameraPos, cameraFront, cameraUp, lookAt, moving_up;
unsigned int width_, height_;
glm::mat4 projection, view, model, mvp;
glm::vec2 mousePosOld, angle;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
Camera cam;
Shader ourShader;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void adding_menu(std::vector<std::unique_ptr<Object>>& objects, glm::vec3 starting_pos);


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
	int to_delete = -1;

	model = glm::mat4(1.0f);
	view = glm::mat4(1.0f);

	cameraPos = { 0,0,3 };
	cameraFront = { 0,0,-1 };
	lookAt = { 0,0,0 };
	cameraUp = { 0,1,0 };

	angle = { -90.0f, 0.0f };

	cam = Camera(cameraPos, cameraFront, cameraUp);
	cam.SetPerspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	//cam->SetOrthographic(-1, 1, 1, -1, -1, 1);

	//float *vertices = new float[6*10*10];
	//unsigned int *indices = new unsigned int[6*10*10];

	//create_torus_points(vertices, indices, 0.5, 0.1, 10, 10, {1,0,0});

	std::vector<std::unique_ptr<Object>> objects_list = {};
	objects_list.push_back(std::make_unique<Torus>(Torus(0.5, 0.1, 10, 10, { 1,1,0,1 }, ourShader)));

	//// setting vertex buffer
	//unsigned int VBO;
	//unsigned int VAO;
	//unsigned int EBO;
	//glGenBuffers(1, &EBO);
	//glGenVertexArrays(1, &VAO);
	//glGenBuffers(1, &VBO);
	//// ..:: Initialization code :: ..
	//// 1. bind Vertex Array Object
	//glBindVertexArray(VAO);
	//// 2. copy our vertices array in a vertex buffer for OpenGL to use
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float)*10*10*6, vertices, GL_STATIC_DRAW);
	//// 3. copy our index array in a element buffer for OpenGL to use
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 10 * 10 * 6, indices, GL_STATIC_DRAW);
	//// 4. then set the vertex attributes pointers
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);

	glEnable(GL_DEPTH_TEST);
	
	Cursor cursor = Cursor(ourShader);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		cursor.SetCursorPosition(lookAt);
		// cleaning frame
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// feed inputs to dear imgui, start new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();
		// input
		processInput(window);
		ImGui::Begin("Main Menu");
		if (ImGui::CollapsingHeader("Add New Objects")) {
			adding_menu(objects_list,lookAt);
		}
		if (ImGui::CollapsingHeader("Objects Present on Scene")) {
			int i = 0;
			for (auto& ob : objects_list) {
				ob->CreateMenu();
				ImGui::SameLine();
				if (ImGui::Button(("Delete##"+std::to_string(i)).c_str())) {
					to_delete = i;
				}
				i++;
			}
		}
		if (to_delete > -1) {
			objects_list.erase(objects_list.begin() + to_delete);
			to_delete = -1;
		}
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		projection = cam.GetProjectionMatrix();
		view = cam.GetViewMatrix();
		model = cam.GetWorldModelMatrix();


		//int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		//int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		//int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
		//glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		mvp = projection * view;

		cursor.DrawObject(mvp);

		mvp = mvp * model;

		for (auto& ob : objects_list) {
			ob.get()->DrawObject(mvp);
		}
		//int projectionLoc = glGetUniformLocation(ourShader.ID, "mvp");
		//glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));

		//ourShader.use();
		//glBindVertexArray(VAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glDrawElements(GL_TRIANGLES, 6*10*10 , GL_UNSIGNED_INT, 0);
		//glBindVertexArray(0);

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

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	width_ = width;
	height_ = height;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	glm::vec2 mousePos = { xpos,ypos };
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
	{
		glm::vec2 diff = (mousePosOld - mousePos) * PRECISION;
		float cameraSpeed = 19.0f * deltaTime;

		diff *= cameraSpeed;

		angle += diff;

		if (angle.y > 90.0f) angle.y = 90.0f - EPS;
		if (angle.y < -90.0f) angle.y = -90.0f + EPS;
		if (angle.x > 180.0f) angle.x = -180.0f + EPS;
		if (angle.x < -180.0f) angle.x = 180.0f - EPS;

		float radius = glm::length(cameraPos - lookAt);

		cameraPos.x = lookAt.x + radius * glm::cos(glm::radians(angle.y)) * glm::cos(glm::radians(angle.x));
		cameraPos.z = lookAt.z + radius * -glm::cos(glm::radians(angle.y)) * glm::sin(glm::radians(angle.x));
		cameraPos.y = lookAt.y + radius * glm::sin(glm::radians(-angle.y));

		cameraFront = glm::normalize(lookAt - cameraPos);
		cam.LookAt(cameraPos, cameraFront, cameraUp);

		//cam.RotateWorld({ diff, 0 });
		//cam.Rotate(-diff.x, diff.y);
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		glm::vec2 diff = (mousePosOld - mousePos) * PRECISION;
		float cameraSpeed = 0.4f * deltaTime;

		glm::vec2 movement = diff * cameraSpeed;

		glm::vec3 right_movement = cam.GetRightVector() * movement.x;
		glm::vec3 up_movement = cam.GetUpVector() * -movement.y;

		cameraPos += right_movement + up_movement;
		lookAt += right_movement + up_movement;

		cam.LookAt(cameraPos, cameraFront, cameraUp);

		//cam.TranslateWorld({ movement, 0 });
		//cam.MoveTarget({ movement.x, -movement.y,0,0 });
	}


	mousePosOld = mousePos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	float precision = 0.01f;

	float movement = 1.0f + yoffset * precision;
	if (movement <= 0.0f)
		movement = 0.1f;
	cam.ScaleWorld({ movement,movement,movement });
	//cam.Zoom(yoffset);
}

void adding_menu(std::vector<std::unique_ptr<Object>>& objects, glm::vec3 starting_pos) {
	if (ImGui::Button("Torus")) {
		objects.push_back(std::make_unique<Torus>(Torus(0.5, 0.1, 10, 10, { 1,1,0,1 }, ourShader)));
		objects.back()->MoveObject(starting_pos);
	}
}
