#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>


Camera::Camera(glm::vec3 pos, glm::vec3 forward, glm::vec3 up)
{
	LookAt(pos, forward, up);
	m_world_model_matrix = glm::mat4(1.0f);
}

void Camera::SetOrthographic(float left, float right, float top, float bottom, float near, float far)
{
	glm::mat4 persp = glm::mat4(1.0f);
	persp[0][0] = 2 / (right - left);
	persp[1][1] = 2 / (top - bottom);
	persp[2][2] = 2 / (far - near);
	persp[3][0] = -((right + left) / (right - left));
	persp[3][1] = -((top + bottom) / (top - bottom));
	persp[3][2] = -((far + near) / (far - near));
	persp[3][3] = 1;
	m_projection_matrix = persp;
}

void Camera::SetPerspective(float fov_y, float aspect, float near, float far)
{
	glm::mat4 persp = glm::mat4(1.0f);
	float e = 1 / tan(fov_y / 2);
	persp[1][1] = e;
	persp[0][0] = 1 / (aspect* tan(fov_y / 2));
	persp[2][2] = -((far + near) / (far - near));
	persp[3][2] = -((far * near * 2) / (far - near));
	persp[3][3] = 0;
	persp[2][3] = -1;
	m_projection_matrix = persp;

}

void Camera::LookAt(glm::vec3 pos, glm::vec3 front, glm::vec3 up)
{
	this->pos = pos;
	this->front = front;
	this->up = up;
	right = glm::normalize(glm::cross(front, up));
	up_real = glm::normalize(glm::cross(right, front));

	glm::vec3 const f(glm::normalize(front));
	glm::vec3 const s(glm::normalize(cross(f, up)));
	glm::vec3 const u(glm::cross(s, f));

	glm::mat4 Result(1.0f);
	Result[0][0] = s.x;
	Result[1][0] = s.y;
	Result[2][0] = s.z;
	Result[0][1] = u.x;
	Result[1][1] = u.y;
	Result[2][1] = u.z;
	Result[0][2] = -f.x;
	Result[1][2] = -f.y;
	Result[2][2] = -f.z;
	Result[3][0] = -glm::dot(s, pos);
	Result[3][1] = -glm::dot(u, pos);
	Result[3][2] = glm::dot(f, pos);
	m_view_matrix = Result;
}



void Camera::SetPosFrontUp(glm::vec3& pos, glm::vec3& front, glm::vec3& up)
{
	pos = this->pos;
	front = this->front;
	up = this->up;
}

void Camera::RotateWorld(glm::vec3 angles)
{
	glm::mat4 x_rotate = glm::mat4(1.0f);
	x_rotate[1][1] = glm::cos(glm::radians(-angles.y));
	x_rotate[2][1] = -glm::sin(glm::radians(-angles.y));
	x_rotate[1][2] = glm::sin(glm::radians(-angles.y));
	x_rotate[2][2] = glm::cos(glm::radians(-angles.y));

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

	m_world_model_matrix = z_rotate * y_rotate * x_rotate * m_world_model_matrix;
}

void Camera::TranslateWorld(glm::vec3 movement)
{
	glm::mat4 translate = glm::mat4(1.0f);
	translate[3][0] = -movement.x;
	translate[3][1] = movement.y;
	translate[3][2] = movement.z;

	m_world_model_matrix = translate * m_world_model_matrix;
}

void Camera::ScaleWorld(glm::vec3 movement)
{
	glm::mat4 scale = glm::mat4(1.0f);
	scale[0][0] = movement.x;
	scale[1][1] = movement.y;
	scale[2][2] = movement.z;

	m_world_model_matrix = scale * m_world_model_matrix;
}

