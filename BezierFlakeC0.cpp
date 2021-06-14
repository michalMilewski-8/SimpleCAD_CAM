#include "BezierFlakeC0.h"

unsigned int BezierFlakeC0::counter = 1;

BezierFlakeC0::BezierFlakeC0(Shader sh, glm::uvec2 flakes_count, glm::uvec2 divisions_, std::vector<std::shared_ptr<Point>> pointss_) :Bezier(sh)
{
	num_of_flakes = flakes_count;
	number_of_divisions[0] = divisions_.x;
	number_of_divisions[1] = divisions_.y;
	points.insert(points.end(), pointss_.begin(), pointss_.end());
	sprintf_s(name, 512, ("BezierFlakeC0 " + std::to_string(counter)).c_str());
	constname = "BezierFlakeC0 " + std::to_string(counter++);
	index_vertices();

	this->color = { 1.0f,1.0f,1.0f,1.0f };
	Update();
	shader = Shader("tes_shader.vs", "tes_shader.fs", "tes_shader.tcs", "tes_shader.tes");
}

void BezierFlakeC0::Serialize(xml_document<>& document, xml_node<>* scene)
{
	auto figure = document.allocate_node(node_element, "PatchC0");
	figure->append_attribute(document.allocate_attribute("Name", document.allocate_string(constname.c_str())));
	figure->append_attribute(document.allocate_attribute("N", document.allocate_string(std::to_string(num_of_flakes.y).c_str())));
	figure->append_attribute(document.allocate_attribute("M", document.allocate_string(std::to_string(num_of_flakes.x).c_str())));
	figure->append_attribute(document.allocate_attribute("NSlices", document.allocate_string(std::to_string(number_of_divisions[1]).c_str())));
	figure->append_attribute(document.allocate_attribute("MSlices", document.allocate_string(std::to_string(number_of_divisions[0]).c_str())));
	auto pointsNode = document.allocate_node(node_element, "Points");
	for (auto& point : points) {
		{
			auto pointRef = document.allocate_node(node_element, "PointRef");
			pointRef->append_attribute(document.allocate_attribute("Name", document.allocate_string(point->constname.c_str())));
			pointsNode->append_node(pointRef);
			point->Serialize(document, scene);
		}
	}
	if (type_ == 1) {
		for (int j = 0; j < 3 * num_of_flakes.y + 1; j++) {
			auto point = points[j];
			auto pointRef = document.allocate_node(node_element, "PointRef");
			pointRef->append_attribute(document.allocate_attribute("Name", document.allocate_string(point->constname.c_str())));
			pointsNode->append_node(pointRef);
		}
	}
	figure->append_node(pointsNode);
	scene->append_node(figure);
}

void BezierFlakeC0::DrawObject(glm::mat4 mvp_)
{
	if (need_update) {
		update_object();
		need_update = false;
	}

	mvp = mvp_;
	//Object::DrawObject(mvp_);

	if (draw_polygon)
		for (auto& pol : polygons)
			pol->DrawObject(mvp);

	for (auto& point : points) {
		point->DrawObject(mvp);
	}

	shader.use();
	glPatchParameteri(GL_PATCH_VERTICES, 16);
	int projectionLoc = glGetUniformLocation(shader.ID, "mvp");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));

	int xLoc = glGetUniformLocation(shader.ID, "x");
	glUniform1f(xLoc, number_of_divisions[0]);

	int yLoc = glGetUniformLocation(shader.ID, "y");
	glUniform1f(yLoc,number_of_divisions[1]);

	glBindVertexArray(VAO);
	
	glDrawElements(GL_PATCHES, patches.size(), GL_UNSIGNED_INT, 0);
	
	glBindVertexArray(0);
}

std::vector<std::shared_ptr<Object>> BezierFlakeC0::GetVirtualObjects()
{
	auto res = std::vector<std::shared_ptr<Object>>();
	for (auto& pt : points) {
		res.push_back(pt);
	}
	return res;
}

void BezierFlakeC0::Select()
{
	Object::Select();
	for (auto& point : points) {
		point->Select();
	}
}

void BezierFlakeC0::UnSelect()
{
	Object::UnSelect();
	for (auto& point : points) {
		point->UnSelect();
	}
}

void BezierFlakeC0::CreateMenu()
{
	float color_new[4];
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
		for (int i = 0; i < 4; i++)
			color_new[i] = color[i];
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
		ImGui::Text("Set color:");
		ImGui::ColorPicker4("Color", color_new);

		ImGui::TreePop();
		ImGui::Separator();

		bool difference = false;
		for (int i = 0; i < 4; i++)
			if (color_new[i] != color[i])
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

void BezierFlakeC0::Update()
{
	need_update = true;
	for (auto& polygon : polygons)
		polygon->Update();
}

void BezierFlakeC0::UpdateMyPointer(std::string constname_,const std::shared_ptr<Object> new_point)
{
	for (int i = 0; i < points.size(); i++) {
		auto point = points[i];
		if (point->CompareName(constname_)) {
			points.erase(points.begin() + i);
			points.insert(points.begin() + i, std::dynamic_pointer_cast<Point>(new_point));
		}
	}
}

std::vector<std::vector<std::vector<std::shared_ptr<Point>>>> BezierFlakeC0::GetAllPatches()
{
	auto res = std::vector<std::vector<std::vector<std::shared_ptr<Point>>>>();
	
	for (int i = 0; i < num_of_flakes.x * num_of_flakes.y; i++) {
		auto patch = std::vector<std::vector<std::shared_ptr<Point>>>();
		for (int j = 0; j < 4; j++) {
			auto line = std::vector<std::shared_ptr<Point>>();
			for (int k = 0; k < 4; k++) {
				line.push_back(points[patches[i * 16 + j * 4 + k]]);
			}
			patch.push_back(line);
		}
		res.push_back(patch);
	}

	return res;
}

void BezierFlakeC0::create_vertices(int type, glm::uvec2 flakes_count, glm::vec2 sizes)
{
	switch (type) {
	case 0: { //plain
		float stridex = sizes.x / (flakes_count.x * 3);
		float stridey = sizes.y / (flakes_count.y * 3);
		for (int i = 0; i < flakes_count.x * 3 + 1; i++) {
			float xpos = i * stridex;
			polygons.push_back(std::make_shared<Line>(shader));
			for (int j = 0; j < flakes_count.y * 3 + 1; j++) {
				float ypos = j * stridey;
				auto point = std::make_shared<Point>(glm::vec3(xpos, 0.0f, ypos), shader);
				//point->AddOwner(shared_from_this());
				points.push_back(point);
				if (i == 0) {
					polygons.push_back(std::make_shared<Line>(shader));
					polygons[0]->AddPoint(point);
				}
				else {
					polygons.back()->AddPoint(point);
				}
				polygons[j + 1]->AddPoint(point);
			}
		}
		for (int i = 0; i < flakes_count.x; i++) {
			for (int j = 0; j < flakes_count.y; j++) {
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 0);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 1);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 2);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 3);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 0);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 1);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 2);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 3);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 0);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 1);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 2);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 3);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 0);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 1);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 2);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 3);
			}
		}

		break;
	}
	case 1: { //barrel
		float stridex = glm::two_pi<float>() / (flakes_count.x * 3);
		float stridez = sizes.y / (flakes_count.y * 3);
		for (int i = 0; i < flakes_count.x * 3; i++) {
			float xpos = sizes.x * std::cos(i * stridex);
			float ypos = sizes.x * std::sin(i * stridex);
			polygons.push_back(std::make_shared<Line>(shader));
			for (int j = 0; j < flakes_count.y * 3 + 1; j++) {
				float zpos = j * stridez;
				auto point = std::make_shared<Point>(glm::vec3(xpos, ypos, zpos), shader);
				//point->AddOwner(shared_from_this());
				points.push_back(point);
				if (i == 0) {
					polygons.push_back(std::make_shared<Line>(shader));
					polygons[0]->AddPoint(point);
				}
				else {
					polygons.back()->AddPoint(point);
				}
				polygons[j + 1]->AddPoint(point);
			}
		}
		for (int j = 0; j < flakes_count.y * 3 + 1; j++) {
			polygons[j + 1]->AddPoint(points[j]);
		}
		for (int i = 0; i < flakes_count.x; i++) {

			for (int j = 0; j < flakes_count.y; j++) {
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 0);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 1);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 2);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 3);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 0);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 1);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 2);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 3);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 0);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 1);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 2);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 3);

				if (i != flakes_count.x - 1) {
					patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 0);
					patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 1);
					patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 2);
					patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 3);
				}
				else {
					patches.push_back(3 * j + 0);
					patches.push_back(3 * j + 1);
					patches.push_back(3 * j + 2);
					patches.push_back(3 * j + 3);
				}
			}
		}
		break;
	}
	default: return;
	}
}

void BezierFlakeC0::update_object()
{
	create_curve();

	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points_on_curve.size(), points_on_curve.data(), GL_DYNAMIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * patches.size(), patches.data(), GL_DYNAMIC_DRAW);
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);
}

void BezierFlakeC0::create_curve()
{
	position = glm::vec3{ 0,0,0 };
	points_.clear();
	points_on_curve.clear();
	int licznik = 0;
	for (auto& point : points) {
		if (!owners_added) point->AddOwner(shared_from_this());
		auto sp = point->GetPosition();
		points_.push_back(sp);
		points_on_curve.push_back(sp.x);
		points_on_curve.push_back(sp.y);
		points_on_curve.push_back(sp.z);
	/*	points_on_curve.push_back(color.r);
		points_on_curve.push_back(color.g);
		points_on_curve.push_back(color.b);
		points_on_curve.push_back(color.a);*/
	
		position += sp;
		licznik++;
	}
	position /= licznik;
	if (!owners_added)
	owners_added = true;
}

void BezierFlakeC0::index_vertices()
{
	for (int i = 0; i < num_of_flakes.x; i++) {
		for (int j = 0; j < num_of_flakes.y; j++) {
			patches.push_back((3 * num_of_flakes.y + 1) * 3 * i + 3 * j + 0);
			patches.push_back((3 * num_of_flakes.y + 1) * 3 * i + 3 * j + 1);
			patches.push_back((3 * num_of_flakes.y + 1) * 3 * i + 3 * j + 2);
			patches.push_back((3 * num_of_flakes.y + 1) * 3 * i + 3 * j + 3);

			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 1) + 3 * j + 0);
			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 1) + 3 * j + 1);
			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 1) + 3 * j + 2);
			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 1) + 3 * j + 3);

			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 2) + 3 * j + 0);
			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 2) + 3 * j + 1);
			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 2) + 3 * j + 2);
			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 2) + 3 * j + 3);

			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 3) + 3 * j + 0);
			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 3) + 3 * j + 1);
			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 3) + 3 * j + 2);
			patches.push_back((3 * num_of_flakes.y + 1) * (3 * i + 3) + 3 * j + 3);
		}
	}
}
