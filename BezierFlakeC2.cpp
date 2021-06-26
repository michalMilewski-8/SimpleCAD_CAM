#include "BezierFlakeC2.h"
#include <math.h>

BezierFlakeC2::BezierFlakeC2(Shader sh, int type, glm::uvec2 flakes_count, glm::vec2 sizes) :
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

std::vector<std::function<glm::vec3(double, double)>> BezierFlakeC2::GetParametrisations()
{
	auto result = std::vector<std::function<glm::vec3(double, double)>>();

	auto decastelieu = [](float t, glm::vec3 B0_, glm::vec3 B1_, glm::vec3 B2_, glm::vec3 B3_) {
		float T0 = -1.0f;
		float T1 = 0.0f;
		float T2 = 1.0f;
		float T3 = 2.0f;
		float T4 = 3.0f;
		float Tm1 = -2.0f;

		float A1 = T2 - t;
		float A2 = T3 - t;
		float A3 = T4 - t;
		float B1 = t - T1;
		float B2 = t - T0;
		float B3 = t - Tm1;

		float N1 = 1;
		float N2 = 0;
		float N3 = 0;
		float N4 = 0;

		float saved = 0.0f;
		float term = 0.0f;

		term = N1 / (A1 + B1);
		N1 = saved + A1 * term;
		saved = B1 * term;

		N2 = saved;
		saved = 0.0f;

		term = N1 / (A1 + B2);
		N1 = saved + A1 * term;
		saved = B2 * term;

		term = N2 / (A2 + B1);
		N2 = saved + A2 * term;
		saved = B1 * term;

		N3 = saved;
		saved = 0.0f;

		term = N1 / (A1 + B3);
		N1 = saved + A1 * term;
		saved = B3 * term;

		term = N2 / (A2 + B2);
		N2 = saved + A2 * term;
		saved = B2 * term;

		term = N3 / (A3 + B1);
		N3 = saved + A3 * term;
		saved = B1 * term;

		N4 = saved;

		return N1 * B0_ + N2 * B1_ + N3 * B2_ + N4 * B3_;
	};

	result.push_back([&](double u, double v) {
		
		int unum = std::max(std::min((int)(u * num_of_flakes.x), (int)num_of_flakes.x - 1), 0);
		int vnum = std::max(std::min((int)(v * num_of_flakes.y), (int)num_of_flakes.y - 1), 0);

		double u_ = u * num_of_flakes.x - unum;
		double v_ = v * num_of_flakes.y - vnum;

		int i = 16*((num_of_flakes.y * unum) + vnum);

		auto p00 = points[patches[i]]->GetPosition();
		auto p01 = points[patches[i + 1]]->GetPosition();
		auto p02 = points[patches[i + 2]]->GetPosition();
		auto p03 = points[patches[i + 3]]->GetPosition();

		auto p10 = points[patches[i + 4]]->GetPosition();
		auto p11 = points[patches[i + 5]]->GetPosition();
		auto p12 = points[patches[i + 6]]->GetPosition();
		auto p13 = points[patches[i + 7]]->GetPosition();

		auto p20 = points[patches[i + 8]]->GetPosition();
		auto p21 = points[patches[i + 9]]->GetPosition();
		auto p22 = points[patches[i + 10]]->GetPosition();
		auto p23 = points[patches[i + 11]]->GetPosition();

		auto p30 = points[patches[i + 12]]->GetPosition();
		auto p31 = points[patches[i + 13]]->GetPosition();
		auto p32 = points[patches[i + 14]]->GetPosition();
		auto p33 = points[patches[i + 15]]->GetPosition();

		glm::vec3 bu0 = decastelieu(v_, p00, p01, p02, p03);
		glm::vec3 bu1 = decastelieu(v_, p10, p11, p12, p13);
		glm::vec3 bu2 = decastelieu(v_, p20, p21, p22, p23);
		glm::vec3 bu3 = decastelieu(v_, p30, p31, p32, p33);

		return decastelieu(u_, bu0, bu1, bu2, bu3);
		});

	return result;
}

std::vector<std::function<glm::vec3(double, double)>> BezierFlakeC2::GetUParametrisations()
{
	auto result = std::vector<std::function<glm::vec3(double, double)>>();

	auto decastelieu = [](float t, glm::vec3 B0_, glm::vec3 B1_, glm::vec3 B2_, glm::vec3 B3_) {
		float T0 = -1.0f;
		float T1 = 0.0f;
		float T2 = 1.0f;
		float T3 = 2.0f;
		float T4 = 3.0f;
		float Tm1 = -2.0f;

		float A1 = T2 - t;
		float A2 = T3 - t;
		float A3 = T4 - t;
		float B1 = t - T1;
		float B2 = t - T0;
		float B3 = t - Tm1;

		float N1 = 1;
		float N2 = 0;
		float N3 = 0;
		float N4 = 0;

		float saved = 0.0f;
		float term = 0.0f;

		term = N1 / (A1 + B1);
		N1 = saved + A1 * term;
		saved = B1 * term;

		N2 = saved;
		saved = 0.0f;

		term = N1 / (A1 + B2);
		N1 = saved + A1 * term;
		saved = B2 * term;

		term = N2 / (A2 + B1);
		N2 = saved + A2 * term;
		saved = B1 * term;

		N3 = saved;
		saved = 0.0f;

		term = N1 / (A1 + B3);
		N1 = saved + A1 * term;
		saved = B3 * term;

		term = N2 / (A2 + B2);
		N2 = saved + A2 * term;
		saved = B2 * term;

		term = N3 / (A3 + B1);
		N3 = saved + A3 * term;
		saved = B1 * term;

		N4 = saved;

		return N1 * B0_ + N2 * B1_ + N3 * B2_ + N4 * B3_;
	};

	auto pochodna = [](float t, glm::vec3 B0_, glm::vec3 B1_, glm::vec3 B2_, glm::vec3 B3_) {
		float T0 = -1.0f;
		float T1 = 0.0f;
		float T2 = 1.0f;
		float T3 = 2.0f;
		float T4 = 3.0f;
		float Tm1 = -2.0f;

		float A1 = T2 - t;
		float A2 = T3 - t;
		float A3 = T4 - t;
		float B1 = t - T1;
		float B2 = t - T0;
		float B3 = t - Tm1;

		float N1 = 1;
		float N2 = 0;
		float N3 = 0;
		float N4 = 0;

		float saved = 0.0f;
		float term = 0.0f;

		term = N1 / (A1 + B1);
		N1 = saved + A1 * term;
		saved = B1 * term;

		N2 = saved;
		saved = 0.0f;

		term = N1 / (A1 + B2);
		N1 = saved + A1 * term;
		saved = B2 * term;

		term = N2 / (A2 + B1);
		N2 = saved + A2 * term;
		saved = B1 * term;

		N3 = saved;

		glm::vec3 f1 = 3.0f * ((B1_ - B0_) / (T2 - Tm1));
		glm::vec3 f2 = 3.0f * ((B2_ - B1_) / (T3 - T0));
		glm::vec3 f3 = 3.0f * ((B3_ - B2_) / (T4 - T1));

		return N1 * f1 + N2 * f2 + N3 * f3;
	};

	result.push_back([&](double u, double v) {
		int unum = std::max(std::min((int)(u * num_of_flakes.x), (int)num_of_flakes.x - 1), 0);
		int vnum = std::max(std::min((int)(v * num_of_flakes.y), (int)num_of_flakes.y - 1), 0);

		double u_ = u * num_of_flakes.x - unum;
		double v_ = v * num_of_flakes.y - vnum;

		int i = 16 * ((num_of_flakes.y * unum) + vnum);

		auto p00 = points[patches[i]]->GetPosition();
		auto p01 = points[patches[i + 1]]->GetPosition();
		auto p02 = points[patches[i + 2]]->GetPosition();
		auto p03 = points[patches[i + 3]]->GetPosition();

		auto p10 = points[patches[i + 4]]->GetPosition();
		auto p11 = points[patches[i + 5]]->GetPosition();
		auto p12 = points[patches[i + 6]]->GetPosition();
		auto p13 = points[patches[i + 7]]->GetPosition();

		auto p20 = points[patches[i + 8]]->GetPosition();
		auto p21 = points[patches[i + 9]]->GetPosition();
		auto p22 = points[patches[i + 10]]->GetPosition();
		auto p23 = points[patches[i + 11]]->GetPosition();

		auto p30 = points[patches[i + 12]]->GetPosition();
		auto p31 = points[patches[i + 13]]->GetPosition();
		auto p32 = points[patches[i + 14]]->GetPosition();
		auto p33 = points[patches[i + 15]]->GetPosition();

		glm::vec3 bu0 = decastelieu(v_, p00, p01, p02, p03);
		glm::vec3 bu1 = decastelieu(v_, p10, p11, p12, p13);
		glm::vec3 bu2 = decastelieu(v_, p20, p21, p22, p23);
		glm::vec3 bu3 = decastelieu(v_, p30, p31, p32, p33);

		return (float)num_of_flakes.x * pochodna(u_, bu0, bu1, bu2, bu3);
		});

	return result;
}

std::vector<std::function<glm::vec3(double, double)>> BezierFlakeC2::GetVParametrisations()
{
	auto result = std::vector<std::function<glm::vec3(double, double)>>();

	auto decastelieu = [](float t, glm::vec3 B0_, glm::vec3 B1_, glm::vec3 B2_, glm::vec3 B3_) {
		float T0 = -1.0f;
		float T1 = 0.0f;
		float T2 = 1.0f;
		float T3 = 2.0f;
		float T4 = 3.0f;
		float Tm1 = -2.0f;

		float A1 = T2 - t;
		float A2 = T3 - t;
		float A3 = T4 - t;
		float B1 = t - T1;
		float B2 = t - T0;
		float B3 = t - Tm1;

		float N1 = 1;
		float N2 = 0;
		float N3 = 0;
		float N4 = 0;

		float saved = 0.0f;
		float term = 0.0f;

		term = N1 / (A1 + B1);
		N1 = saved + A1 * term;
		saved = B1 * term;

		N2 = saved;
		saved = 0.0f;

		term = N1 / (A1 + B2);
		N1 = saved + A1 * term;
		saved = B2 * term;

		term = N2 / (A2 + B1);
		N2 = saved + A2 * term;
		saved = B1 * term;

		N3 = saved;
		saved = 0.0f;

		term = N1 / (A1 + B3);
		N1 = saved + A1 * term;
		saved = B3 * term;

		term = N2 / (A2 + B2);
		N2 = saved + A2 * term;
		saved = B2 * term;

		term = N3 / (A3 + B1);
		N3 = saved + A3 * term;
		saved = B1 * term;

		N4 = saved;

		return N1 * B0_ + N2 * B1_ + N3 * B2_ + N4 * B3_;
	};

	auto pochodna = [](float t, glm::vec3 B0_, glm::vec3 B1_, glm::vec3 B2_, glm::vec3 B3_) {
		float T0 = -1.0f;
		float T1 = 0.0f;
		float T2 = 1.0f;
		float T3 = 2.0f;
		float T4 = 3.0f;
		float Tm1 = -2.0f;

		float A1 = T2 - t;
		float A2 = T3 - t;
		float A3 = T4 - t;
		float B1 = t - T1;
		float B2 = t - T0;
		float B3 = t - Tm1;

		float N1 = 1;
		float N2 = 0;
		float N3 = 0;
		float N4 = 0;

		float saved = 0.0f;
		float term = 0.0f;

		term = N1 / (A1 + B1);
		N1 = saved + A1 * term;
		saved = B1 * term;

		N2 = saved;
		saved = 0.0f;

		term = N1 / (A1 + B2);
		N1 = saved + A1 * term;
		saved = B2 * term;

		term = N2 / (A2 + B1);
		N2 = saved + A2 * term;
		saved = B1 * term;

		N3 = saved;

		glm::vec3 f1 = 3.0f * ((B1_ - B0_) / (T2 - Tm1));
		glm::vec3 f2 = 3.0f * ((B2_ - B1_) / (T3 - T0));
		glm::vec3 f3 = 3.0f * ((B3_ - B2_) / (T4 - T1));

		return N1 * f1 + N2 * f2 + N3 * f3;
	};

	result.push_back([&](double u, double v) {

		int unum = std::max(std::min((int)(u * num_of_flakes.x), (int)num_of_flakes.x - 1), 0);
		int vnum = std::max(std::min((int)(v * num_of_flakes.y), (int)num_of_flakes.y - 1), 0);

		double u_ = u * num_of_flakes.x - unum;
		double v_ = v * num_of_flakes.y - vnum;
		int i = 16 * ((num_of_flakes.y * unum) + vnum);

		auto p00 = points[patches[i]]->GetPosition();
		auto p01 = points[patches[i + 1]]->GetPosition();
		auto p02 = points[patches[i + 2]]->GetPosition();
		auto p03 = points[patches[i + 3]]->GetPosition();

		auto p10 = points[patches[i + 4]]->GetPosition();
		auto p11 = points[patches[i + 5]]->GetPosition();
		auto p12 = points[patches[i + 6]]->GetPosition();
		auto p13 = points[patches[i + 7]]->GetPosition();

		auto p20 = points[patches[i + 8]]->GetPosition();
		auto p21 = points[patches[i + 9]]->GetPosition();
		auto p22 = points[patches[i + 10]]->GetPosition();
		auto p23 = points[patches[i + 11]]->GetPosition();

		auto p30 = points[patches[i + 12]]->GetPosition();
		auto p31 = points[patches[i + 13]]->GetPosition();
		auto p32 = points[patches[i + 14]]->GetPosition();
		auto p33 = points[patches[i + 15]]->GetPosition();

		glm::vec3 bu0 = decastelieu(u_, p00, p10, p20, p30);
		glm::vec3 bu1 = decastelieu(u_, p01, p11, p21, p31);
		glm::vec3 bu2 = decastelieu(u_, p02, p12, p22, p32);
		glm::vec3 bu3 = decastelieu(u_, p03, p13, p23, p33);

		return (float)num_of_flakes.y * pochodna(v_, bu0, bu1, bu2, bu3);
		});

	return result;
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
				polygons[j + 1]->AddPoint(point);
			}
		}
		for (int i = 0; i < flakes_count.x; i++) {

			for (int j = 0; j < flakes_count.y; j++) {

				patches.push_back((flakes_count.y + 3) * i + j + 0);
				patches.push_back((flakes_count.y + 3) * i + j + 1);
				patches.push_back((flakes_count.y + 3) * i + j + 2);
				patches.push_back((flakes_count.y + 3) * i + j + 3);

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



				if (i == flakes_count.x - 1) {
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
