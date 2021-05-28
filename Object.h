#pragma once
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
#include <algorithm>
#include <memory>

#include "Shader.h"
#include "Camera.h"
#include "imgui.h"
#include "Virtual.h"
#include "Dependencies/include/rapidxml-1.13/rapidxml.hpp"

using namespace rapidxml;

class Object
	
{
public:
	Object(Shader& shader_, int number);
	Object() = default;
	~Object();
	virtual void DrawObject(glm::mat4 mvp);
	virtual void CreateMenu() {};
	void MoveObject(glm::vec3 movement);
	void MoveObjectTo(glm::vec3 movement);
	void RotateObject(glm::vec3 movement);
	void RotateObject(glm::quat rotation);
	void ResizeObject(glm::vec3 movement);
	void Select();
	void UnSelect();
	void SetName(std::string name_);
	bool CompareName(std::string name_);
	virtual void Update() {};
	virtual std::vector<Object*> GetVirtualObjects() { return std::vector<Object*>(); };
	virtual void Serialize(xml_document<>& document, xml_node<>* scene) {};

	static glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest);
	
	glm::vec3 GetPosition();

	unsigned int* screen_width;
	unsigned int* screen_height;

	bool selected;
	char name[512];

	std::string constname;

protected:
	virtual void update_object() {};
	virtual void inform_owner_of_change() {};
	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;
	

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
	glm::quat quaternion_rotation;


	glm::vec4 color;

	Shader shader;
};

