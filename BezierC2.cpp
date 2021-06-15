#include "BezierC2.h"

unsigned int BezierC2::counter = 1;

BezierC2::BezierC2(Shader& sh) :
	Bezier(sh),
	polygon(std::make_shared<Line>(sh)),
	polygon_bezier(std::make_shared<Line>(sh)),
	geom_shader("shader_bezier_c0.vs", "shader.fs", "shader_bezier_c0.gs")
{
	sprintf_s(name, 512, ("BezierC2 " + std::to_string(counter)).c_str());
	constname = "BezierC2 " + std::to_string(counter);
	counter++;
	number_of_divisions = 100;
	points = std::vector<std::weak_ptr<Point>>();
	this->color = { 0.7f,0.7f,0.7f,0.7f };
	update_object();
}

void BezierC2::DrawObject(glm::mat4 mvp_)
{
	moved = false;
	if (points.size() < 2) return;

	if (need_update) {
		mvp = mvp_;
		update_object();
		need_update = false;
	}
	else {
		mvp = mvp_;
	}
	if (draw_polygon)
		polygon->DrawObject(mvp_);
	if (draw_polygon_bezier) {
		polygon_bezier->DrawObject(mvp_);
	}
	if (show_bezier_points) {
		for (auto point : bezier_points) {
			point->DrawObject(mvp_);
		}
	}

	geom_shader.use();
	int projectionLoc = glGetUniformLocation(geom_shader.ID, "mvp");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));
	float start = 0.0f;
	float end = 0.0f;
	int number_of_divisions_greater = std::ceil(number_of_divisions / 120.0f);
	float stride = 1.0f / number_of_divisions_greater;
	glBindVertexArray(VAO);
	for (int i = 0; i <= number_of_divisions_greater; i++) {
		start = end;
		end = start + stride;
		int startLoc = glGetUniformLocation(geom_shader.ID, "start");
		int endLoc = glGetUniformLocation(geom_shader.ID, "end");
		glUniform1f(startLoc, start);
		glUniform1f(endLoc, end);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_LINES_ADJACENCY, lines.size(), GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
}

void BezierC2::CreateMenu()
{
	float color_new[4];
	char buffer[512];
	char buf[512];
	int to_delete = -1;
	sprintf_s(buffer, "%s###%sdup2", name, constname);
	if (ImGui::TreeNode(buffer)) {

		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text("Edit name:");
			ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		for (int i = 0; i < 4; i++)
			color_new[i] = color[i];
		ImGui::Checkbox("Selected", &selected);
		if (selected != was_selected_in_last_frame) {
			Update();
			was_selected_in_last_frame = selected;
		}
		ImGui::Checkbox("Draw De Bohr Polygon", &draw_polygon);
		if (draw_polygon != was_draw_polygon) {
			Update();
			was_draw_polygon = draw_polygon;
		}
		ImGui::Checkbox("Show Bezier Points", &show_bezier_points);
		ImGui::Checkbox("Draw Bezier Polygon", &draw_polygon_bezier); 
		if (draw_polygon_bezier != was_draw_polygon_bezier) {
			Update();
			was_draw_polygon_bezier = draw_polygon_bezier;
		}
		ImGui::Text("Set color:");
		ImGui::ColorPicker4("Color", color_new);

		if (ImGui::CollapsingHeader("De Bohr Points on Curve")) {
			for (int i = 0; i < points.size(); i++) {
				if (points[i].expired())
				{
					to_delete = i;
					continue;
				}
				auto sp = points[i].lock();
				ImGui::Text(sp->name); ImGui::SameLine();
				sprintf_s(buf, "Remove###%sRm%d", sp->name, i);
				if (ImGui::Button(buf)) {
					to_delete = i;
				}
			}
		}
		if (to_delete >= 0) {
			points.erase(points.begin() + to_delete);
			polygon->DeletePoint(to_delete);
			Update();
		}

		/*if (ImGui::CollapsingHeader("Bezier Points on Curve")) {
			for (int i = 0; i < bezier_points.size(); i++) {
				auto sp = bezier_points[i];
				ImGui::Text(sp->name);
				sprintf_s(buf, "Select###%sSel%d", sp->name, i);
				if (ImGui::Button(buf)) {
					sp->Select();
				}
			}
		}*/

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
			Update();
		}
	}
}

void BezierC2::AddPointToCurve(std::shared_ptr<Point>& point)
{
	if (point.get()) {
		points.push_back(point);
		point->AddOwner(shared_from_this());
		polygon->AddPoint(point);
		Update();
	}
}

void BezierC2::Update()
{
	need_update = true;
	polygon->Update();
	polygon_bezier->Update();
}

std::vector<std::shared_ptr<Object>> BezierC2::GetVirtualObjects()
{
	auto res = std::vector<std::shared_ptr<Object>>();
	for (auto& pt : bezier_points) {
		res.push_back(pt);
	}
	return res;
}

void BezierC2::Serialize(xml_document<>& document, xml_node<>* scene)
{
	auto figure = document.allocate_node(node_element, "BezierC2");
	figure->append_attribute(document.allocate_attribute("Name", document.allocate_string(constname.c_str())));
	auto pointsNode = document.allocate_node(node_element, "Points");
	for (auto& point : points) {
		if (!point.expired()) {
			auto point_rel = point.lock();
			auto pointRef = document.allocate_node(node_element, "PointRef");
			pointRef->append_attribute(document.allocate_attribute("Name", document.allocate_string(point_rel->constname.c_str())));
			pointsNode->append_node(pointRef);
		}
	}
	figure->append_node(pointsNode);
	scene->append_node(figure);
}

void BezierC2::UpdateMyPointer(std::string constname_, const std::shared_ptr<Object> new_point)
{
	for (int i = 0; i < points.size(); i++) {
		if (points[i].expired()) continue;
		auto point = points[i].lock();
		if (point->CompareName(constname_)) {
			points.erase(points.begin() + i);
			points.insert(points.begin() + i, std::dynamic_pointer_cast<Point>(new_point));
		}
	}                                       
}

void BezierC2::update_object()
{
	lines.clear();
	points_on_curve.clear();
	

	int k = 0;
	for (auto& pt : de_points_) {
		while (k < points.size() && points[k].expired()) k++;
		if (k >= points.size() || pt != points[k].lock()->GetPosition()) {
			need_new_bezier_generation = true;
			break;
		}
		k++;
	}
	if( k != points.size()) need_new_bezier_generation = true;
	if (need_new_bezier_generation) {
		need_new_bezier_generation = false;
		generate_bezier_points();
	}
	else {
		k = 0;
		int moved_point_index = -1;
		for (auto& pt : points_) {
			if (k >= bezier_points.size() || pt != bezier_points[k]->GetPosition()) {
				moved_point_index = k;
				break;
			}
			k++;
		}
		if (moved_point_index >= 0) {
			translate_bezier_movement_to_de_boor(moved_point_index);
			generate_bezier_points();
			bezier_points[moved_point_index]->Select();
		}
	}
		

	points_.clear();
	position = glm::vec3{ 0,0,0 };
	int licznik = 0;
	for (auto& point : bezier_points) {
		auto sp = point->GetPosition();
		points_.push_back(sp);
		position += sp;
		licznik++;
	}
	position /= licznik;

	create_curve();

	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points_on_curve.size(), points_on_curve.data(), GL_DYNAMIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * lines.size(), lines.data(), GL_DYNAMIC_DRAW);
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void BezierC2::create_curve()
{

	for (int iter = 0; iter + 1 < points_.size();) {
		int start = iter;
		int end = iter + 1;
		for (int i = 0; i < 3 && end < points_.size(); i++) { iter++; end++; }
		int number_of_divisions_loc = 1;
		for (int odl = start; odl < end - 1; odl++) {
			glm::vec4 A = { points_[odl],1 };
			glm::vec4 B = { points_[odl + 1],1 };
			A = mvp * A;
			A /= A.w;
			B = mvp * B;
			B /= B.w;
			glm::vec2 screenA = {
				(A.x + 1.0f) * *screen_width / 2.0f,
				(B.y + 1.0f) * *screen_height / 2.0f,
			};
			glm::vec2 screenB = {
				(B.x + 1.0f) * *screen_width / 2.0f,
				(B.y + 1.0f) * *screen_height / 2.0f,
			};
			float len = glm::length(screenA - screenB);
			if (len > 0)
				number_of_divisions_loc += len;
		}
		if (number_of_divisions_loc > number_of_divisions) number_of_divisions = number_of_divisions_loc;
	}

	int k = 0;
	for (k = 0; k < points_.size(); k += 3) {
		for (int j = 0; j < 4; j++) {
			lines.push_back(k + j);
		}
	}

	for (auto& vec : points_) {
		points_on_curve.push_back(vec.x);
		points_on_curve.push_back(vec.y);
		points_on_curve.push_back(vec.z);
		points_on_curve.push_back(color.r);
		points_on_curve.push_back(color.g);
		points_on_curve.push_back(color.b);
		points_on_curve.push_back(color.a);
	}
	if (lines.size() == 0) return;
	int left = (lines.back() - points_.size()) + 1;

	while (left > 0) {
		points_on_curve.push_back(0.0f);
		points_on_curve.push_back(0.0f);
		points_on_curve.push_back(0.0f);
		points_on_curve.push_back(-1.0f);
		points_on_curve.push_back(-1.0f);
		points_on_curve.push_back(-1.0f);
		points_on_curve.push_back(-1.0f);
		left--;
	}
}

void BezierC2::generate_bezier_points()
{
	bezier_points.clear();
	de_points_.clear();
	polygon_bezier->ClearPoints();
	for (auto& point : points) {
		if (!point.expired()) {
			de_points_.push_back(point.lock()->GetPosition());
		}
	}
	if (de_points_.size() < 4) {
		need_new_bezier_generation = true;
		return;
	}
	glm::vec3 first_pos = de_points_[0];
	glm::vec3 second_pos = de_points_[1];
	glm::vec3 third_pos = de_points_[2];
	glm::vec3 fourth_pos = de_points_[3];
	glm::vec3 second_point, third_point, fourth_point;
	glm::vec3 first_vec, second_vec, third_vec; 
	glm::vec3 helper_fifth;
	first_vec = second_pos - first_pos;
	second_vec = third_pos - second_pos;
	third_vec = fourth_pos - third_pos;

	third_point = second_pos - (first_vec / 3.0f);
	helper_fifth = second_pos + (second_vec / 3.0f);
	fourth_point = third_point + (helper_fifth - third_point) / 2.0f;

	add_bezier_point(fourth_point);

	for (int i = 4; i < de_points_.size(); i++) {

		second_point = helper_fifth;
		third_point = third_pos - (second_vec / 3.0f);
		helper_fifth = third_pos + (third_vec / 3.0f);
		fourth_point = third_point + (helper_fifth- third_point) / 2.0f;

		add_bezier_point(second_point);
		add_bezier_point(third_point);
		add_bezier_point(fourth_point);

		third_pos = fourth_pos;
		fourth_pos = de_points_[i];
		
		first_vec = second_vec;
		second_vec = third_vec;
		third_vec = fourth_pos - third_pos;
	}

	second_point = helper_fifth;
	third_point = third_pos - (second_vec / 3.0f);
	helper_fifth = third_pos + (third_vec / 3.0f);
	fourth_point = third_point + (helper_fifth - third_point) / 2.0f;

	add_bezier_point(second_point);
	add_bezier_point(third_point);
	add_bezier_point(fourth_point);

	bezier_points.shrink_to_fit();
}

void BezierC2::add_bezier_point(glm::vec3 position)
{
	auto point = std::make_shared<Point>(position, shader);
	bezier_points.push_back(point);
	point->AddOwner(shared_from_this());
	polygon_bezier->AddPoint(point);
	point->AddOwner(polygon_bezier);
}

void BezierC2::translate_bezier_movement_to_de_boor(int point_index)
{
	int moving_de_boor_point_index = 1 + point_index / 3;
	Point* moving_de_boor_point = get_de_boor_point(moving_de_boor_point_index);
	Point* right = get_de_boor_point(moving_de_boor_point_index + 1);
	if (!moving_de_boor_point || !right) return;
	glm::vec3 right_pos = right->GetPosition();
	glm::vec3 sr;
	glm::vec3 current_bezier_point_pos = bezier_points[point_index]->GetPosition();
	float multiplication_number = 1.5f;

	if (point_index % 3 == 0) {
		Point* left = get_de_boor_point(moving_de_boor_point_index - 1);
		if (!left ) return;
		glm::vec3 left_pos = left->GetPosition();
		sr = left_pos + (right_pos - left_pos) / 2.0f;
	}
	else {
		sr = right_pos;
		if (point_index % 3 == 2) multiplication_number = 3.0f;
	}
	glm::vec3 intended_position = sr + (current_bezier_point_pos - sr) * multiplication_number;
	moving_de_boor_point->MoveObjectTo(intended_position);
}

Point* BezierC2::get_de_boor_point(int position)
{
	int r = 0;
	for (auto& pt : points) {
		if (pt.expired()) continue;
		if (r == position) return pt.lock().get();
		r++;
	}
	return nullptr;
}
