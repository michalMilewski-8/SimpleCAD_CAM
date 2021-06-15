#include "Cursor.h"

Cursor::Cursor(Shader& sh) :Object(sh, 7)
{
	points = {
		0,0,0,1,0,0,1,
		0.1f,0,0,1,0,0,1,
		0,0,0,0,1,0,1,
		0,0.1f,0,0,1,0,1,
		0,0,0,0,0,1,1,
		0,0,0.1f,0,0,1,1
	};

	lines = {
		0,1,
		2,3,
		4,5
	};

	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points.size(), points.data(), GL_DYNAMIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * lines.size(), lines.data(), GL_DYNAMIC_DRAW);
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void Cursor::SetCursorPosition(glm::vec3 position)
{
	this->position = position;
	translate = glm::mat4(1.0f);
	translate[3][0] = position.x;
	translate[3][1] = position.y;
	translate[3][2] = position.z;
}

void Cursor::DrawObject(glm::mat4 mvp)
{
	moved = false;
	Object::DrawObject(mvp);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_LINES, points.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
