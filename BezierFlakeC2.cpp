#include "BezierFlakeC2.h"

BezierFlakeC2::BezierFlakeC2(Shader sh, int type, glm::uvec2 flakes_count, glm::vec2 sizes):
	BezierFlakeC0(sh)
{
	create_vertices(type, flakes_count, sizes);
	sprintf_s(name, 512, ("BezierFlakeC2 " + std::to_string(counter)).c_str());
	constname = "BezierFlakeC2 " + std::to_string(counter++);
	num_of_flakes = flakes_count;
	number_of_divisions[0] = 4;
	number_of_divisions[1] = 4;
	this->color = { 1.0f,1.0f,1.0f,1.0f };
	type_ = type;
	Update();
	shader = Shader("tes_shader.vs", "tes_shader.fs", "tes_shader.tcs", "tes_shader_C2.tes");
}

BezierFlakeC2::BezierFlakeC2(Shader sh, glm::uvec2 flakes_count, glm::uvec2 divisions_, std::vector<std::shared_ptr<Point>> points_) :
	BezierFlakeC0(sh)
{
	num_of_flakes = flakes_count;
	number_of_divisions[0] = divisions_.x;
	number_of_divisions[1] = divisions_.y;
	points.insert(points.end(), points_.begin(), points_.end());
	sprintf_s(name, 512, ("BezierFlakeC2 " + std::to_string(counter)).c_str());
	constname = "BezierFlakeC2 " + std::to_string(counter++);
	index_vertices();

	this->color = { 1.0f,1.0f,1.0f,1.0f };
	Update();
	shader = Shader("tes_shader.vs", "tes_shader.fs", "tes_shader.tcs", "tes_shader_C2.tes");
}

void BezierFlakeC2::Serialize(xml_document<>& document, xml_node<>* scene)
{
	auto figure = document.allocate_node(node_element, "PatchC2");
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
		for (int j = 0; j < 3 * (num_of_flakes.y + 3); j++) {
			auto point = points[j];
			auto pointRef = document.allocate_node(node_element, "PointRef");
			pointRef->append_attribute(document.allocate_attribute("Name", document.allocate_string(point->constname.c_str())));
			pointsNode->append_node(pointRef);
		}
	}
	figure->append_node(pointsNode);
	scene->append_node(figure);
}

void BezierFlakeC2::UpdateMyPointer(std::string constname_, const std::shared_ptr<Object> new_point)
{
	for (int i = 0; i < points.size(); i++) {
		auto point = points[i];
		if (point->CompareName(constname_)) {
			points.erase(points.begin() + i);
			points.insert(points.begin() + i, std::dynamic_pointer_cast<Point>(new_point));
		}
	}
}

void BezierFlakeC2::create_vertices(int type, glm::uvec2 flakes_count, glm::vec2 sizes)
{
	switch (type) {
	case 0: { //plain
		float stridex = sizes.x / (flakes_count.x + 2);
		float stridey = sizes.y / (flakes_count.y + 2);
		for (int i = 0; i < flakes_count.x + 3; i++) {
			float xpos = i * stridex;
			polygons.push_back(std::make_shared<Line>(shader));
			for (int j = 0; j < flakes_count.y + 3; j++) {
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
				polygons[j+1]->AddPoint(point);
			}
		}
		for (int i = 0; i < flakes_count.x; i++) {

			for (int j = 0; j < flakes_count.y; j++) {

				patches.push_back((flakes_count.y + 3) * i + j + 0);
				patches.push_back((flakes_count.y + 3) * i + j + 1);
				patches.push_back((flakes_count.y + 3) * i + j + 2);
				patches.push_back((flakes_count.y + 3) * i + j + 3);
													
				patches.push_back((flakes_count.y + 3) * ( i + 1) + j + 0);
				patches.push_back((flakes_count.y + 3) * ( i + 1) + j + 1);
				patches.push_back((flakes_count.y + 3) * ( i + 1) + j + 2);
				patches.push_back((flakes_count.y + 3) * ( i + 1) + j + 3);
													
				patches.push_back((flakes_count.y + 3) * ( i + 2) + j + 0);
				patches.push_back((flakes_count.y + 3) * ( i + 2) + j + 1);
				patches.push_back((flakes_count.y + 3) * ( i + 2) + j + 2);
				patches.push_back((flakes_count.y + 3) * ( i + 2) + j + 3);
													
				patches.push_back((flakes_count.y + 3) * ( i + 3) + j + 0);
				patches.push_back((flakes_count.y + 3) * ( i + 3) + j + 1);
				patches.push_back((flakes_count.y + 3) * ( i + 3) + j + 2);
				patches.push_back((flakes_count.y + 3) * ( i + 3) + j + 3);
			}
		}

		break;
	}
	case 1: { //barrel
		float stridex = glm::two_pi<float>() / (flakes_count.x);
		float stridez = sizes.y / (flakes_count.y + 2);
		for (int i = 0; i < flakes_count.x ; i++) {
			float xpos = sizes.x * std::cos(i * stridex);
			float ypos = sizes.x * std::sin(i * stridex);
			polygons.push_back(std::make_shared<Line>(shader));
			for (int j = 0; j < flakes_count.y + 3; j++) {
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
		for (int j = 0; j < flakes_count.y + 3; j++) {
			polygons[j + 1]->AddPoint(points[j]);
		}
		for (int i = 0; i < flakes_count.x; i++) {

			for (int j = 0; j < flakes_count.y; j++) {

				patches.push_back((flakes_count.y + 3) * i + j + 0);
				patches.push_back((flakes_count.y + 3) * i + j + 1);
				patches.push_back((flakes_count.y + 3) * i + j + 2);
				patches.push_back((flakes_count.y + 3) * i + j + 3);



				if (i == flakes_count.x -1) {
					patches.push_back(j + 0);
					patches.push_back(j + 1);
					patches.push_back(j + 2);
					patches.push_back(j + 3);

					patches.push_back((flakes_count.y + 3) * (1) + j + 0);
					patches.push_back((flakes_count.y + 3) * (1) + j + 1);
					patches.push_back((flakes_count.y + 3) * (1) + j + 2);
					patches.push_back((flakes_count.y + 3) * (1) + j + 3);

					patches.push_back((flakes_count.y + 3) * (2) + j + 0);
					patches.push_back((flakes_count.y + 3) * (2) + j + 1);
					patches.push_back((flakes_count.y + 3) * (2) + j + 2);
					patches.push_back((flakes_count.y + 3) * (2) + j + 3);
				}
				else if (i == flakes_count.x - 2) {
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 0);
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 1);
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 2);
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 3);

					patches.push_back(j + 0);
					patches.push_back(j + 1);
					patches.push_back(j + 2);
					patches.push_back(j + 3);

					patches.push_back((flakes_count.y + 3) * (1) + j + 0);
					patches.push_back((flakes_count.y + 3) * (1) + j + 1);
					patches.push_back((flakes_count.y + 3) * (1) + j + 2);
					patches.push_back((flakes_count.y + 3) * (1) + j + 3);
				}
				else if (i == flakes_count.x - 3) {
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 0);
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 1);
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 2);
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 3);

					patches.push_back((flakes_count.y + 3) * (i + 2) + j + 0);
					patches.push_back((flakes_count.y + 3) * (i + 2) + j + 1);
					patches.push_back((flakes_count.y + 3) * (i + 2) + j + 2);
					patches.push_back((flakes_count.y + 3) * (i + 2) + j + 3);

					patches.push_back(j + 0);
					patches.push_back(j + 1);
					patches.push_back(j + 2);
					patches.push_back(j + 3);
				}
				else {
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 0);
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 1);
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 2);
					patches.push_back((flakes_count.y + 3) * (i + 1) + j + 3);

					patches.push_back((flakes_count.y + 3) * (i + 2) + j + 0);
					patches.push_back((flakes_count.y + 3) * (i + 2) + j + 1);
					patches.push_back((flakes_count.y + 3) * (i + 2) + j + 2);
					patches.push_back((flakes_count.y + 3) * (i + 2) + j + 3);

					patches.push_back((flakes_count.y + 3) * (i + 3) + j + 0);
					patches.push_back((flakes_count.y + 3) * (i + 3) + j + 1);
					patches.push_back((flakes_count.y + 3) * (i + 3) + j + 2);
					patches.push_back((flakes_count.y + 3) * (i + 3) + j + 3);
				}
			}
		}
		break;
	}
	default: return;
	}
}

void BezierFlakeC2::index_vertices()
{
	for (int i = 0; i < num_of_flakes.x; i++) {

		for (int j = 0; j < num_of_flakes.y; j++) {

			patches.push_back((num_of_flakes.y + 3) * i + j + 0);
			patches.push_back((num_of_flakes.y + 3) * i + j + 1);
			patches.push_back((num_of_flakes.y + 3) * i + j + 2);
			patches.push_back((num_of_flakes.y + 3) * i + j + 3);

			patches.push_back((num_of_flakes.y + 3) * (i + 1) + j + 0);
			patches.push_back((num_of_flakes.y + 3) * (i + 1) + j + 1);
			patches.push_back((num_of_flakes.y + 3) * (i + 1) + j + 2);
			patches.push_back((num_of_flakes.y + 3) * (i + 1) + j + 3);

			patches.push_back((num_of_flakes.y + 3) * (i + 2) + j + 0);
			patches.push_back((num_of_flakes.y + 3) * (i + 2) + j + 1);
			patches.push_back((num_of_flakes.y + 3) * (i + 2) + j + 2);
			patches.push_back((num_of_flakes.y + 3) * (i + 2) + j + 3);

			patches.push_back((num_of_flakes.y + 3) * (i + 3) + j + 0);
			patches.push_back((num_of_flakes.y + 3) * (i + 3) + j + 1);
			patches.push_back((num_of_flakes.y + 3) * (i + 3) + j + 2);
			patches.push_back((num_of_flakes.y + 3) * (i + 3) + j + 3);
		}
	}
}
