#include "BezierFlakeC2.h"

BezierFlakeC2::BezierFlakeC2(Shader sh, int type, glm::uvec2 flakes_count, glm::vec2 sizes):
	BezierFlakeC0(sh)
{
	create_vertices(type, flakes_count, sizes);
	sprintf_s(name, 512, ("BezierFlakeC2 " + std::to_string(counter)).c_str());
	constname = "BezierFlakeC2 " + std::to_string(counter++);
	number_of_divisions[0] = 4;
	number_of_divisions[1] = 4;
	this->color = { 1.0f,1.0f,1.0f,1.0f };
	update_object();
	shader = Shader("tes_shader.vs", "tes_shader.fs", "tes_shader.tcs", "tes_shader_C2.tes");
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
				auto point = std::make_shared<VirtualPoint>(glm::vec3(xpos, 0.0f, ypos), shader);
				point->AddOwner(this);
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
		for (int i = 0; i < flakes_count.x; i++) {
			float xpos = sizes.x * std::cos(i * stridex);
			float ypos = sizes.x * std::sin(i * stridex);
			polygons.push_back(std::make_shared<Line>(shader));
			for (int j = 0; j < flakes_count.y + 3; j++) {
				float zpos = j * stridez;
				auto point = std::make_shared<VirtualPoint>(glm::vec3(xpos, ypos, zpos), shader);
				point->AddOwner(this);
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
