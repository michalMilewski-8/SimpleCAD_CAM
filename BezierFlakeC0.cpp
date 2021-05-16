#include "BezierFlakeC0.h"

unsigned int BezierFlakeC0::counter = 1;

void BezierFlakeC0::DrawObject(glm::mat4 mvp_)
{
	if (need_update || mvp != mvp_) {
		mvp = mvp_;
		update_object();
		need_update = false;
	}
	else {
		mvp = mvp_;
	}

	if (draw_polygon)
		for (auto& pol : polygons)
			pol->DrawObject(mvp_);

	//shader.use();
	//int projectionLoc = glGetUniformLocation(shader.ID, "mvp");
	//glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));
}

std::vector<VirtualObject*> BezierFlakeC0::GetVirtualObjects()
{
	auto res = std::vector<VirtualObject*>();
	for (auto& pt : points) {
		res.push_back(pt.get());
	}
	return res;
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
		for (int i = 0; i < 4; i++)
			color_new[i] = color[i];
		ImGui::Checkbox("Selected", &selected);
		if (selected != was_selected_in_last_frame) {
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
}

void BezierFlakeC0::create_vertices(int type, glm::uvec2 flakes_count, glm::vec2 sizes)
{
	switch (type) {
	case 0: { //plain
		float stridex = sizes.x / (flakes_count.x * 3);
		float stridey = sizes.y / (flakes_count.y * 3);
		for (int i = 0; i < flakes_count.x * 3 + 1; i++) {
			float xpos = i * stridex;
			for (int j = 0; j < flakes_count.y * 3 + 1; j++) {
				float ypos = j * stridey;
				points.push_back(std::make_shared<VirtualPoint>(glm::vec3(xpos, 0.0f, ypos), shader));
				//points.back()->AddOwner(shared_from_this());
			}
		}
		for (int i = 0; i < flakes_count.x; i++) {

			for (int j = 0; j < flakes_count.y; j++) {
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));

				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 0);
				polygons[polygons.size() - 4]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 1);
				polygons[polygons.size() - 3]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 2);
				polygons[polygons.size() - 2]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 3);
				polygons[polygons.size() - 1]->AddPoint(points[patches.back()]);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 0);
				polygons[polygons.size() - 4]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 1);
				polygons[polygons.size() - 3]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 2);
				polygons[polygons.size() - 2]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 3);
				polygons[polygons.size() - 1]->AddPoint(points[patches.back()]);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 0);
				polygons[polygons.size() - 4]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 1);
				polygons[polygons.size() - 3]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 2);
				polygons[polygons.size() - 2]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 3);
				polygons[polygons.size() - 1]->AddPoint(points[patches.back()]);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 0);
				polygons[polygons.size() - 4]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 1);
				polygons[polygons.size() - 3]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 2);
				polygons[polygons.size() - 2]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 3);
				polygons[polygons.size() - 1]->AddPoint(points[patches.back()]);


				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));

				polygons[polygons.size() - 4]->AddPoint(points[(3 * flakes_count.y + 1) * 3 * i + 3 * j + 0]);
				polygons[polygons.size() - 4]->AddPoint(points[(3 * flakes_count.y + 1) * 3 * i + 3 * j + 1]);
				polygons[polygons.size() - 4]->AddPoint(points[(3 * flakes_count.y + 1) * 3 * i + 3 * j + 2]);
				polygons[polygons.size() - 4]->AddPoint(points[(3 * flakes_count.y + 1) * 3 * i + 3 * j + 3]);

				polygons[polygons.size() - 3]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 0]);
				polygons[polygons.size() - 3]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 1]);
				polygons[polygons.size() - 3]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 2]);
				polygons[polygons.size() - 3]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 3]);

				polygons[polygons.size() - 2]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 0]);
				polygons[polygons.size() - 2]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 1]);
				polygons[polygons.size() - 2]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 2]);
				polygons[polygons.size() - 2]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 3]);

				polygons[polygons.size() - 1]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 0]);
				polygons[polygons.size() - 1]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 1]);
				polygons[polygons.size() - 1]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 2]);
				polygons[polygons.size() - 1]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 3]);
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
			for (int j = 0; j < flakes_count.y * 3 + 1; j++) {
				float zpos = j * stridez;
				points.push_back(std::make_shared<VirtualPoint>(glm::vec3(xpos, ypos, zpos), shader));
				//points.back()->AddOwner(shared_from_this());
			}
		}
		for (int i = 0; i < flakes_count.x; i++) {

			for (int j = 0; j < flakes_count.y; j++) {
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));

				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 0);
				polygons[polygons.size() - 4]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 1);
				polygons[polygons.size() - 3]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 2);
				polygons[polygons.size() - 2]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * 3 * i + 3 * j + 3);
				polygons[polygons.size() - 1]->AddPoint(points[patches.back()]);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 0);
				polygons[polygons.size() - 4]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 1);
				polygons[polygons.size() - 3]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 2);
				polygons[polygons.size() - 2]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 3);
				polygons[polygons.size() - 1]->AddPoint(points[patches.back()]);

				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 0);
				polygons[polygons.size() - 4]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 1);
				polygons[polygons.size() - 3]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 2);
				polygons[polygons.size() - 2]->AddPoint(points[patches.back()]);
				patches.push_back((3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 3);
				polygons[polygons.size() - 1]->AddPoint(points[patches.back()]);

				if (i != flakes_count.x - 1) {
					patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 0);
					polygons[polygons.size() - 4]->AddPoint(points[patches.back()]);
					patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 1);
					polygons[polygons.size() - 3]->AddPoint(points[patches.back()]);
					patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 2);
					polygons[polygons.size() - 2]->AddPoint(points[patches.back()]);
					patches.push_back((3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 3);
					polygons[polygons.size() - 1]->AddPoint(points[patches.back()]);
				}
				else {
					patches.push_back(3 * j + 0);
					polygons[polygons.size() - 4]->AddPoint(points[patches.back()]);
					patches.push_back(3 * j + 1);
					polygons[polygons.size() - 3]->AddPoint(points[patches.back()]);
					patches.push_back(3 * j + 2);
					polygons[polygons.size() - 2]->AddPoint(points[patches.back()]);
					patches.push_back(3 * j + 3);
					polygons[polygons.size() - 1]->AddPoint(points[patches.back()]);
				}


				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));
				polygons.push_back(std::make_shared<Line>(shader));

				polygons[polygons.size() - 4]->AddPoint(points[(3 * flakes_count.y + 1) * 3 * i + 3 * j + 0]);
				polygons[polygons.size() - 4]->AddPoint(points[(3 * flakes_count.y + 1) * 3 * i + 3 * j + 1]);
				polygons[polygons.size() - 4]->AddPoint(points[(3 * flakes_count.y + 1) * 3 * i + 3 * j + 2]);
				polygons[polygons.size() - 4]->AddPoint(points[(3 * flakes_count.y + 1) * 3 * i + 3 * j + 3]);

				polygons[polygons.size() - 3]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 0]);
				polygons[polygons.size() - 3]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 1]);
				polygons[polygons.size() - 3]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 2]);
				polygons[polygons.size() - 3]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 1) + 3 * j + 3]);

				polygons[polygons.size() - 2]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 0]);
				polygons[polygons.size() - 2]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 1]);
				polygons[polygons.size() - 2]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 2]);
				polygons[polygons.size() - 2]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 2) + 3 * j + 3]);

				if (i != flakes_count.x - 1) {
					polygons[polygons.size() - 1]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 0]);
					polygons[polygons.size() - 1]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 1]);
					polygons[polygons.size() - 1]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 2]);
					polygons[polygons.size() - 1]->AddPoint(points[(3 * flakes_count.y + 1) * (3 * i + 3) + 3 * j + 3]);
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
}

void BezierFlakeC0::create_curve()
{
}
