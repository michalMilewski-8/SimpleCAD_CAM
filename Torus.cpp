#include "Torus.h"

Torus::Torus(float R, float r, int vertical, int horizontal, glm::vec4 color, Shader sh) :
	Object(sh, 7)
{
	this->R = R;
	this->r = r;
	vertical_points_number = vertical;
	horizontal_points_number = horizontal;
	this->color = color;
	update_object();
}

Torus::~Torus()
{
	if (points)
		delete[] points;
	if (triangles)
		delete[] triangles;
}

void Torus::DrawObject(glm::mat4 mvp)
{
	Object::DrawObject(mvp);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, 6* vertical_points_number*horizontal_points_number, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Torus::CreateMenu()
{

	float R_new;
	float r_new;
	int vertical_points_number_new;
	int horizontal_points_number_new;
	float color_new[4];

	if (ImGui::TreeNode("Torus")) {
		R_new = R;
		r_new = r;
		vertical_points_number_new = vertical_points_number;
		horizontal_points_number_new = horizontal_points_number;
		for (int i = 0; i < 4; i++)
			color_new[i] = color[i];

		ImGui::Text("Set number of divistions:");
		ImGui::SliderInt("Vertical", &vertical_points_number_new, 1, 100);
		ImGui::SliderInt("Horizontal", &horizontal_points_number_new, 1, 100);
		ImGui::Text("Set radiuses:");
		ImGui::SliderFloat("R", &R_new, 0.01, 100);
		ImGui::SliderFloat("r", &r_new, 1, 100);
		ImGui::Text("Set color:");
		ImGui::ColorPicker4("Color", color_new);
		ImGui::TreePop();
		ImGui::Separator();

		if (R_new != R || r_new != r ||
			vertical_points_number_new != vertical_points_number ||
			horizontal_points_number_new != horizontal_points_number)
		{
			R = R_new > 0 ? R_new : R;
			r = r_new > 0 ? r_new : r;
			vertical_points_number = vertical_points_number_new > 0 ? vertical_points_number_new : vertical_points_number;
			horizontal_points_number = horizontal_points_number_new > 0 ? horizontal_points_number_new : horizontal_points_number;
			update_object();
		}
		bool difference = false;
		for (int i = 0; i < 4; i++)
			if(color_new[i] != color[i])
			{
				difference = true;
				break;
			}
		if (difference) {
			color = { color_new[0],color_new[1],color_new[2],color_new[3] };
			update_object();
		}
	}
}

void Torus::SetR(float _R)
{
	R = _R;
	update_object();
}

void Torus::Setr(float _r)
{
	r = _r;
	update_object();
}

void Torus::SetVertical(int _v)
{
	vertical_points_number = _v;
	update_object();
}

void Torus::SetHorizontal(float _h)
{
	horizontal_points_number = _h;
	update_object();
}

void Torus::SetColor(glm::vec4 _c)
{
	color = _c;
	update_object();
}

void Torus::create_torus_points() {
	float vertical_stride = 360.0f / (vertical_points_number);
	float horizontal_stride = 360.0f / (horizontal_points_number);

	for (int i = 0; i < horizontal_points_number; i++) {
		float beta = i * horizontal_stride;
		for (int j = 0; j < vertical_points_number; j++) {
			float alfa = j * vertical_stride;

			glm::vec3 point = torus_point(glm::radians(alfa), glm::radians(beta));
			points[description_number * (i * vertical_points_number + j)] = point.x;
			points[description_number * (i * vertical_points_number + j) + 1] = point.y;
			points[description_number * (i * vertical_points_number + j) + 2] = point.z;
			points[description_number * (i * vertical_points_number + j) + 3] = color.r;
			points[description_number * (i * vertical_points_number + j) + 4] = color.g;
			points[description_number * (i * vertical_points_number + j) + 5] = color.b;
			points[description_number * (i * vertical_points_number + j) + 6] = color.b;

			triangles[6 * (i * vertical_points_number + j)] = i * vertical_points_number + j;
			triangles[6 * (i * vertical_points_number + j) + 1] = ((i + 1) % horizontal_points_number) * vertical_points_number + (vertical_points_number + (j - 1) % vertical_points_number) % vertical_points_number;
			triangles[6 * (i * vertical_points_number + j) + 2] = ((i + 1) % horizontal_points_number) * vertical_points_number + j;
			triangles[6 * (i * vertical_points_number + j) + 3] = i * vertical_points_number + j;
			triangles[6 * (i * vertical_points_number + j) + 4] = ((i + 1) % horizontal_points_number) * vertical_points_number + j;
			triangles[6 * (i * vertical_points_number + j) + 5] = i * vertical_points_number + (j + 1) % vertical_points_number;
		}
	}
}

glm::vec3 Torus::torus_point(float alfa_r, float beta_r) {
	return { glm::cos(beta_r) * R - glm::sin(alfa_r) * glm::cos(beta_r) * r,glm::sin(beta_r) * R - glm::sin(alfa_r) * glm::sin(beta_r) * r,glm::cos(alfa_r) * r };
}

void Torus::update_object()
{
	if (points)
		delete[] points;
	if (triangles)
		delete[] triangles;

	triangles = new unsigned int[description_number * vertical_points_number * horizontal_points_number];
	points = new float[description_number * vertical_points_number * horizontal_points_number];

	create_torus_points();

	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * description_number * vertical_points_number * horizontal_points_number, points, GL_DYNAMIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * description_number * vertical_points_number * horizontal_points_number, triangles, GL_DYNAMIC_DRAW);
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}
