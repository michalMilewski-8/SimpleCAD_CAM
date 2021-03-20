#include "Object.h"

Object::Object(Shader shader_, int number) :
	shader(shader_),
	description_number(number),
	translate(glm::mat4(1.0f)),
	resize(glm::mat4(1.0f)),
	rotate(glm::mat4(1.0f)),
	mvp(glm::mat4(1.0f)),
	selected(false),
	position(glm::vec3(0.0f))
{
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
}

void Object::DrawObject(glm::mat4 mvp_)
{	
	mvp = mvp_ * translate * rotate * resize;
	int projectionLoc = glGetUniformLocation(shader.ID, "mvp");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));

	shader.use();
	glBindVertexArray(VAO);
}

void Object::MoveObject(glm::vec3 movement)
{
	position += movement;
	glm::mat4 translate_ = glm::mat4(1.0f);
	translate_[3][0] = movement.x;
	translate_[3][1] = movement.y;
	translate_[3][2] = movement.z;

	translate = translate_ * translate;
}

void Object::MoveObjectTo(glm::vec3 movement)
{
	position = movement;
	glm::mat4 translate_ = glm::mat4(1.0f);
	translate_[3][0] = movement.x;
	translate_[3][1] = movement.y;
	translate_[3][2] = movement.z;

	translate = translate_;
}

void Object::RotateObject(glm::vec3 angles)
{
	glm::mat4 x_rotate = glm::mat4(1.0f);
	x_rotate[1][1] = glm::cos(glm::radians(angles.y));
	x_rotate[2][1] = glm::sin(glm::radians(angles.y));
	x_rotate[1][2] = -glm::sin(glm::radians(angles.y));
	x_rotate[2][2] = glm::cos(glm::radians(angles.y));

	glm::mat4 y_rotate = glm::mat4(1.0f);
	y_rotate[0][0] = glm::cos(glm::radians(angles.x));
	y_rotate[2][0] = -glm::sin(glm::radians(angles.x));
	y_rotate[0][2] = glm::sin(glm::radians(angles.x));
	y_rotate[2][2] = glm::cos(glm::radians(angles.x));

	glm::mat4 z_rotate = glm::mat4(1.0f);
	z_rotate[0][0] = glm::cos(glm::radians(angles.z));
	z_rotate[1][0] = -glm::sin(glm::radians(angles.z));
	z_rotate[0][1] = glm::sin(glm::radians(angles.z));
	z_rotate[1][1] = glm::cos(glm::radians(angles.z));

	rotate = z_rotate * y_rotate * x_rotate * rotate;
}

void Object::ResizeObject(glm::vec3 movement)
{
	glm::mat4 scale = glm::mat4(1.0f);
	scale[0][0] = movement.x;
	scale[1][1] = movement.y;
	scale[2][2] = movement.z;

	resize = scale * resize;
}

void Object::Select()
{
	selected = true;
	if (selected != was_selected_in_last_frame) {
		update_object();
		was_selected_in_last_frame = selected;
	}
}

glm::vec3 Object::GetPosition()
{
	return position;
}
