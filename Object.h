#pragma once
#include <stdio.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>

#include "Shader.h"
#include "Camera.h"
class Object
{
public:
	Object(Shader shader_, int number);
	virtual void DrawObject(glm::mat4 mvp);
protected:
	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;

	unsigned int description_number;
	glm::mat4 model;
	glm::mat4 mvp;

	Shader shader;
};

