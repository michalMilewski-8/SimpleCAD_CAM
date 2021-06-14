#pragma once
#include"Object.h"
#include "Line.h"
#include "Point.h"
class GregoryPatch : Object
{
public:
	GregoryPatch(Shader sh) :Object(sh, 7){
		sprintf_s(name, 512, ("GregoryPatch " + std::to_string(counter)).c_str());
		constname = "GregoryPatch " + std::to_string(counter++);
		this->color = { 1.0f,1.0f,1.0f,1.0f };
		Update();
		shader = Shader("tes_shader.vs", "tes_shader.fs", "tes_grzegorz_shader.tcs", "tes_grzegorz_shader.tes");
		for (int i = 0; i < 20; i++) {
			indexes.push_back(i);
		}
		number_of_divisions[0] = 4;
		number_of_divisions[1] = 4;
	}

	void UpdatePoints(glm::vec3 points[20]);
	void Update() override;
	void DrawObject(glm::mat4 mvp) override;
	void CreateMenu() override;
	char* Name() { return name; }

	static unsigned int counter;

private:
	std::vector<std::shared_ptr<Line>> polygon {};
	bool draw_polygon{ false };
	bool was_draw_polygon;
	int number_of_divisions[2];
	std::vector<glm::vec3> points;
	std::vector<std::shared_ptr<Point>> points_greg;
	std::vector<float> points_;
	std::vector<unsigned int> indexes;
	void update_object() override;

};

