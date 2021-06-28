#include "Intersection.h"

unsigned int Intersection::counter = 1;

Intersection::Intersection(Shader sh, std::shared_ptr<Object> obj_left_, std::shared_ptr<Object> obj_right_) : Object(sh,7)
{
	sprintf_s(name, 512, ("Intersection " + std::to_string(counter)).c_str());
	constname = "Intersection " + std::to_string(counter);
	points_right = {};
	points_left = {};
	line_left = std::make_shared<Line>(sh);
	line_right = std::make_shared<Line>(sh);
	interpolation_left = std::make_shared<BezierInterpol>(sh);
	interpolation_right = std::make_shared<BezierInterpol>(sh);
	parameters_left = {};
	parameters_right = {};
	obj_left = obj_left_;
	obj_right = obj_right_;
	counter++;
}

void Intersection::DrawObject(glm::mat4 mvp)
{
	if (show_left) {
		if (show_interpolation)
			interpolation_left->DrawObject(mvp);
		else
		line_left->DrawObject(mvp);
		if(show_points)
			for (auto& point : points_left) 
				point->DrawObject(mvp);
	}

	if (show_right) {
		if (show_interpolation)
			interpolation_right->DrawObject(mvp);
		else
		line_right->DrawObject(mvp);
		if (show_points)
			for (auto& point : points_right)
				point->DrawObject(mvp);
	}
}

void Intersection::CreateMenu()
{
	char buffer[512];
	sprintf_s(buffer, "%s###%s", name, constname);
	if (ImGui::TreeNode(buffer)) {

		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text("Edit name:");
			ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::Checkbox("Show left", &show_left);
		ImGui::Checkbox("Show right", &show_right);
		ImGui::Checkbox("Show points", &show_points);
		ImGui::Checkbox("Show interpolation", &show_interpolation);
		ImGui::Checkbox("Show parametrization textures", &show_testures);
		if (testure_was_created && show_testures) {
			ImGui::Begin("Left Object parametrisation##uu", &show_testures);
			ImGuiIO& io = ImGui::GetIO();
			ImTextureID my_tex_id = (void*)texture_left_ID;
			float my_tex_w = n;
			float my_tex_h = n;
			{
				ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
				ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
				ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
				ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
				ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					float region_sz = 32.0f;
					float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
					float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
					float zoom = 4.0f;
					if (region_x < 0.0f) { region_x = 0.0f; }
					else if (region_x > my_tex_w - region_sz) { region_x = my_tex_w - region_sz; }
					if (region_y < 0.0f) { region_y = 0.0f; }
					else if (region_y > my_tex_h - region_sz) { region_y = my_tex_h - region_sz; }
					ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
					ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
					ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
					ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
					ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
					ImGui::EndTooltip();
				}
			}
			ImGui::End();
			ImGui::Begin("Right Object parametrisation##uu", &show_testures);
			my_tex_id = (void*)texture_right_ID;
			{
				ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
				ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
				ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
				ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
				ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					float region_sz = 32.0f;
					float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
					float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
					float zoom = 4.0f;
					if (region_x < 0.0f) { region_x = 0.0f; }
					else if (region_x > my_tex_w - region_sz) { region_x = my_tex_w - region_sz; }
					if (region_y < 0.0f) { region_y = 0.0f; }
					else if (region_y > my_tex_h - region_sz) { region_y = my_tex_h - region_sz; }
					ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
					ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
					ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
					ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
					ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
					ImGui::EndTooltip();
				}
			}
			ImGui::End();
		}
		ImGui::TreePop();
		ImGui::Separator();
	}
}

void Intersection::AddPoints(std::shared_ptr<Point> left, std::shared_ptr<Point> right)
{
	points_left.push_back(left);
	line_left->AddPoint(left);
	interpolation_left->AddPointToCurve(left);

	points_right.push_back(right);
	line_right->AddPoint(right);
	interpolation_right->AddPointToCurve(right);

	interpolation_left->screen_height = screen_height;
	interpolation_left->screen_width = screen_width;

	interpolation_right->screen_height = screen_height;
	interpolation_right->screen_width = screen_width;
}

void Intersection::AddParameters(glm::vec2 left, glm::vec2 right)
{
	parameters_left.push_back(left);
	parameters_right.push_back(right);
}

std::shared_ptr<BezierInterpol> Intersection::CreateInterpolationBezierLine(bool left)
{
	auto res = std::make_shared<BezierInterpol>(shader);
	if (left) {
		for (auto& point : points_left)
			res->AddPointToCurve(point);
	}
	else {
		for (auto& point : points_right)
			res->AddPointToCurve(point);
	}
	res->screen_height = screen_height;
	res->screen_width = screen_width;
	return res;
}

void Intersection::Reverse()
{
	std::reverse(points_left.begin(), points_left.end());
	std::reverse(points_right.begin(), points_right.end());
	std::reverse(parameters_left.begin(), parameters_left.end());
	std::reverse(parameters_right.begin(), parameters_right.end());
	line_left->Reverse();
	line_right->Reverse();
	interpolation_left->Reverse();
	interpolation_right->Reverse();
}

void Intersection::create_texture()
{
	if (parameters_left.size() < 2) return;
	
	std::vector<std::vector<bool>> values_left = std::vector<std::vector<bool>>(n);
	std::vector<std::vector<bool>> values_right = std::vector<std::vector<bool>>(n);

	for (int i = 0; i < n; i++) {
		values_left[i] = std::vector<bool>(n);
		values_right[i] = std::vector<bool>(n);
		for (int j = 0; j < n; j++) {
			values_left[i][j] = false;
			values_right[i][j] = false;
		}
	}

	auto last_point = parameters_left.front();
	glm::vec2 current_point;
	glm::vec2 diff;
	glm::vec2 vec;
	glm::vec2 tmp_p;

	int add_x = 0;
	int add_y = 0;

	for (int i = 1; i < parameters_left.size(); i++) {
		add_x = 0;
		add_y = 0;
		current_point = parameters_left[i];
		diff = current_point - last_point;
		if (std::abs(diff.x) > (std::min(current_point.x, last_point.x) + 1.0f - std::max(current_point.x, last_point.x))) {
			if (current_point.x > last_point.x)
				add_x = -1;
			else
				add_x = 1;
		}

		if (std::abs(diff.y) > (std::min(current_point.y, last_point.y) + 1.0f - std::max(current_point.y, last_point.y))) {
			if (current_point.y > last_point.y)
				add_y = -1;
			else
				add_y = 1;
		}
		current_point += glm::vec2(add_x, add_y);
		current_point *= n;
		last_point *= n;

		vec = glm::normalize(current_point - last_point);
		tmp_p = last_point;
		for (int k = 0; k < glm::length(current_point - last_point);k++) {
			values_left[(int)(tmp_p.x >= n ? tmp_p.x - n : tmp_p.x < 0 ? tmp_p.x + n : tmp_p.x)][(int)(tmp_p.y >= n ? tmp_p.y - n : tmp_p.y < 0 ? tmp_p.y + n : tmp_p.y)] = true;
			tmp_p += vec;
		}
		
		last_point = parameters_left[i];
	}

	last_point = parameters_right.front();

	for (int i = 1; i < parameters_right.size(); i++) {
		add_x = 0;
		add_y = 0;
		current_point = parameters_right[i];
		diff = current_point - last_point;
		if (std::abs(diff.x) > (std::min(current_point.x, last_point.x) + 1.0f - std::max(current_point.x, last_point.x))) {
			if (current_point.x > last_point.x)
				add_x = -1;
			else
				add_x = 1;
		}

		if (std::abs(diff.y) > (std::min(current_point.y, last_point.y) + 1.0f - std::max(current_point.y, last_point.y))) {
			if (current_point.y > last_point.y)
				add_y = -1;
			else
				add_y = 1;
		}
		current_point += glm::vec2(add_x, add_y);
		current_point *= n;
		last_point *= n;

		vec = glm::normalize(current_point - last_point);
		tmp_p = last_point;
		for (int k = 0; k < glm::length(current_point - last_point); k++) {
			values_right[(int)(tmp_p.x >= n ? tmp_p.x - n : tmp_p.x < 0 ? tmp_p.x + n : tmp_p.x)][(int)(tmp_p.y >= n ? tmp_p.y - n : tmp_p.y < 0 ? tmp_p.y + n : tmp_p.y)] = true;
			tmp_p += vec;
		}

		last_point = parameters_right[i];
	}

	glGenTextures(1, &texture_left_ID);
	glBindTexture(GL_TEXTURE_2D, texture_left_ID);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	std::vector<unsigned char>data = {};
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (values_left[i][j]) {
				data.push_back(255);
				data.push_back(255);
				data.push_back(255);
			}
			else {
				data.push_back(0);
				data.push_back(0);
				data.push_back(0);
			}
		}
	}
	if (data.size()>0)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, n, n, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
		glGenerateMipmap(GL_TEXTURE_2D);
	}


	glGenTextures(1, &texture_right_ID);
	glBindTexture(GL_TEXTURE_2D, texture_right_ID);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	data.clear();
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (values_right[i][j]) {
				data.push_back(255);
				data.push_back(255);
				data.push_back(255);
			}
			else {
				data.push_back(0);
				data.push_back(0);
				data.push_back(0);
			}
		}
	}
	if (data.size() > 0)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, n, n, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	testure_was_created = true;
}
