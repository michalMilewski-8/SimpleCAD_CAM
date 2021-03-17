#include "Object.h"

Object::Object(Shader shader_, int number) :
	shader(shader_),
	description_number(number),
	model(glm::mat4(1.0f)),
	mvp(glm::mat4(1.0f)),
	selected(false)
{
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
}

void Object::DrawObject(glm::mat4 mvp_)
{	
	mvp = mvp_ * model;
	int projectionLoc = glGetUniformLocation(shader.ID, "mvp");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));

	shader.use();
	glBindVertexArray(VAO);
}

void Object::MoveObject(glm::vec3 movement)
{
	glm::mat4 translate = glm::mat4(1.0f);
	translate[3][0] = movement.x;
	translate[3][1] = movement.y;
	translate[3][2] = movement.z;

	model = translate* model;
}
