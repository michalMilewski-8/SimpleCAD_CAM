#include "GregoryPatch.h"

unsigned int GregoryPatch::counter = 1;

void GregoryPatch::UpdatePoints(glm::vec3 points_new[20])
{
	points.clear();
	points_.clear();
	for (int i = 0; i < 20; i++) {
		if (points_greg.size() < 20)
			points_greg.push_back(std::make_shared<Point>(points_new[i], glm::vec4(196 / 255.0f, 157 / 255.0f, 29 / 255.0f, 1.0f), Shader("shader.vs", "shader.fs")));
		else
			points_greg[i]->MoveObjectTo(points_new[i]);
		points.push_back(points_new[i]);
		points_.push_back(points_new[i].x);
		points_.push_back(points_new[i].y);
		points_.push_back(points_new[i].z);
	}

	update_object();
}

void GregoryPatch::Update()
{
	need_update = true;
}

void GregoryPatch::DrawObject(glm::mat4 mvp_)
{
	moved = false;
	if (need_update) {
		update_object();
		need_update = false;
	}
	if (points.size() == 0) return;

	mvp = mvp_;
	//Object::DrawObject(mvp_);

	if (draw_polygon) {
		for (auto& pol : polygon)
			pol->DrawObject(mvp);
		for (auto& point : points_greg)
			point->DrawObject(mvp);
	}



	shader.use();
	glPatchParameteri(GL_PATCH_VERTICES, 20);
	int projectionLoc = glGetUniformLocation(shader.ID, "mvp");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));

	int xLoc = glGetUniformLocation(shader.ID, "x");
	glUniform1f(xLoc, number_of_divisions[0]);

	int yLoc = glGetUniformLocation(shader.ID, "y");
	glUniform1f(yLoc, number_of_divisions[1]);

	glBindVertexArray(VAO);

	glDrawElements(GL_PATCHES, indexes.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void GregoryPatch::CreateMenu()
{
	char buffer[512];
	char buf[512];
	int to_delete = -1;
	sprintf_s(buffer, "%s###%sdu2p", name, constname);
	if (ImGui::TreeNode(buffer)) {

		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text("Edit name:");
			ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::DragInt2("number of X and Y divisions", number_of_divisions, 1, 4, 50);
		ImGui::Checkbox("Selected", &selected);
		if (selected != was_selected_in_last_frame) {
			if (selected) Select();
			else UnSelect();
			update_object();
			was_selected_in_last_frame = selected;
		}
		ImGui::Checkbox("Draw Polygon", &draw_polygon);
		if (draw_polygon != was_draw_polygon) {
			update_object();
			was_draw_polygon = draw_polygon;
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
}

void GregoryPatch::update_object()
{
	if (polygon.size() < 9 && points_greg.size() == 20) {
		auto shader = Shader("shader.vs", "shader.fs");
		polygon.push_back(std::make_shared<Line>(shader));
		polygon.back()->AddPoint(points_greg[0]);
		polygon.back()->AddPoint(points_greg[1]);
		polygon.back()->AddPoint(points_greg[2]);
		polygon.back()->AddPoint(points_greg[3]);
		polygon.back()->AddPoint(points_greg[9]);
		polygon.back()->AddPoint(points_greg[15]);
		polygon.back()->AddPoint(points_greg[19]);
		polygon.back()->AddPoint(points_greg[18]);
		polygon.back()->AddPoint(points_greg[17]);
		polygon.back()->AddPoint(points_greg[16]);
		polygon.back()->AddPoint(points_greg[10]);
		polygon.back()->AddPoint(points_greg[4]);
		polygon.back()->AddPoint(points_greg[0]);

		polygon.push_back(std::make_shared<Line>(shader));
		polygon.back()->AddPoint(points_greg[1]);
		polygon.back()->AddPoint(points_greg[6]);

		polygon.push_back(std::make_shared<Line>(shader));
		polygon.back()->AddPoint(points_greg[4]);
		polygon.back()->AddPoint(points_greg[5]);

		polygon.push_back(std::make_shared<Line>(shader));
		polygon.back()->AddPoint(points_greg[10]);
		polygon.back()->AddPoint(points_greg[11]);

		polygon.push_back(std::make_shared<Line>(shader));
		polygon.back()->AddPoint(points_greg[17]);
		polygon.back()->AddPoint(points_greg[12]);

		polygon.push_back(std::make_shared<Line>(shader));
		polygon.back()->AddPoint(points_greg[18]);
		polygon.back()->AddPoint(points_greg[13]);

		polygon.push_back(std::make_shared<Line>(shader));
		polygon.back()->AddPoint(points_greg[15]);
		polygon.back()->AddPoint(points_greg[14]);

		polygon.push_back(std::make_shared<Line>(shader));
		polygon.back()->AddPoint(points_greg[9]);
		polygon.back()->AddPoint(points_greg[8]);

		polygon.push_back(std::make_shared<Line>(shader));
		polygon.back()->AddPoint(points_greg[2]);
		polygon.back()->AddPoint(points_greg[7]);
	}
	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points_.size(), points_.data(), GL_DYNAMIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexes.size(), indexes.data(), GL_DYNAMIC_DRAW);
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);
}
