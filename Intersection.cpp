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
