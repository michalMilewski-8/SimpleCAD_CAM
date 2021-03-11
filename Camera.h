#pragma once
#include <glm/glm.hpp>

class Camera
{
public:
	Camera(glm::vec3 pos, glm::vec3 forward, glm::vec3 up);
	~Camera() = default;

	glm::mat4 GetViewMatrix() const { return m_view_matrix; }
	glm::mat4 GetWorldModelMatrix() const { return m_world_model_matrix; }
	glm::mat4 GetProjectionMatrix() const { return m_projection_matrix; };
	glm::vec3 GetRightVector() const { return right; };
	glm::vec3 GetUpVector() const { return up_real; };

	void SetOrthographic(float left, float right, float top, float bottom, float near, float far);
	void SetPerspective(float fov, float aspect, float near, float far);

	void LookAt(glm::vec3 pos, glm::vec3 front, glm::vec3 up);
	void SetPosFrontUp(glm::vec3& pos, glm::vec3& front, glm::vec3& up);

	void RotateWorld(glm::vec3 angles);
	void TranslateWorld(glm::vec3 movement);
	void ScaleWorld(glm::vec3 movement);


private:
	glm::vec3 m_position;
	glm::vec3 m_up_vector;
	glm::vec3 m_forward_vector;

	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 pos;
	glm::vec3 right;
	glm::vec3 up_real;


	glm::mat4 m_view_matrix;
	glm::mat4 m_world_model_matrix;
	glm::mat4 m_projection_matrix;
};
