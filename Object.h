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
#include <algorithm>

#include "Shader.h"
#include "Camera.h"
#include "imgui.h"



class Object
{
public:
	Object(Shader shader_, int number);
	Object() = default;
	virtual void DrawObject(glm::mat4 mvp);
	virtual void CreateMenu() {};
	void MoveObject(glm::vec3 movement);
	void MoveObjectTo(glm::vec3 movement);
	void RotateObject(glm::vec3 movement);
	void ResizeObject(glm::vec3 movement);
	void Select();
	virtual void Update() {};
	
	glm::vec3 GetPosition();

	unsigned int* screen_width;
	unsigned int* screen_height;

	bool selected;
	char name[512];
protected:
	virtual void update_object() {};
	virtual void inform_owner_of_change() {};
	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;
	
	std::string constname;

	bool was_selected_in_last_frame{false};
	bool need_update{ false };
	unsigned int description_number;
	 
	glm::mat4 translate;
	glm::mat4 rotate;
	glm::mat4 resize;
	
	glm::mat4 mvp;

	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 angle;


	glm::vec4 color;

	Shader shader;
};

