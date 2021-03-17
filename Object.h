#pragma once
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
#include "imgui.h"
class Object
{
public:
	Object(Shader shader_, int number);
	virtual void DrawObject(glm::mat4 mvp);
	virtual void CreateMenu() {};
	void MoveObject(glm::vec3 movement);

	glm::vec3 GetPosition();

	bool selected;
protected:
	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;
	char name[512];
	std::string constname;

	bool was_selected_in_last_frame{false};

	unsigned int description_number;
	glm::mat4 model;
	glm::mat4 mvp;

	glm::vec3 position;

	glm::vec4 color;

	Shader shader;
};

