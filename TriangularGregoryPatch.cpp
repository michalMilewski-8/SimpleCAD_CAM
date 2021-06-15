#include "TriangularGregoryPatch.h"

unsigned int TriangularGregoryPatch::counter = 1;

TriangularGregoryPatch::TriangularGregoryPatch(std::vector<std::vector<std::vector<std::shared_ptr<Point>>>> patches_, Shader sh) : Object(sh, 7)
{
	find_conected_patches(patches_);
	sprintf_s(name, 512, ("TriangularGregoryPatch " + std::to_string(counter)).c_str());
	constname = "TriangularGregoryPatch " + std::to_string(counter++);
	this->color = { 1.0f,1.0f,1.0f,1.0f };
	patches_gr.emplace_back(std::make_unique<GregoryPatch>(sh));
	patches_gr.emplace_back(std::make_unique<GregoryPatch>(sh));
	patches_gr.emplace_back(std::make_unique<GregoryPatch>(sh));
	Update();
}

void TriangularGregoryPatch::UpdateOwnership()
{
	for (int z = 0; z < 2; z++) {
		for (int g = 0; g < 4; g++) {
			points[0][g][z].lock()->AddOwner(shared_from_this());
			points[1][g][z].lock()->AddOwner(shared_from_this());
			points[2][g][z].lock()->AddOwner(shared_from_this());
			points[0][g][z].lock()->UnSelect();
			points[1][g][z].lock()->UnSelect();
			points[2][g][z].lock()->UnSelect();
		}
	}
}

void TriangularGregoryPatch::update_object()
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 2; k++) {
				if (points[i][j][k].expired()) {
					is_proper = false;
					return;
				}
				points_[i][j][k] = points[i][j][k].lock()->GetPosition();
			}
		}
	}

	glm::vec3 Q[3];

	for (int i = 0; i < 3; i++) {
		glm::vec3 R11, R12, R13, R21, R22, R23;
		glm::vec3 S11, S12, S21, S22;
		glm::vec3 T11, T21;


		R21 = points_[i][0][1] + (points_[i][1][1] - points_[i][0][1]) / 2.0f;
		R22 = points_[i][1][1] + (points_[i][2][1] - points_[i][1][1]) / 2.0f;
		R23 = points_[i][2][1] + (points_[i][3][1] - points_[i][2][1]) / 2.0f;

		R11 = points_[i][0][0] + (points_[i][1][0] - points_[i][0][0]) / 2.0f;
		R12 = points_[i][1][0] + (points_[i][2][0] - points_[i][1][0]) / 2.0f;
		R13 = points_[i][2][0] + (points_[i][3][0] - points_[i][2][0]) / 2.0f;


		S11 = R11 + (R12 - R11) / 2.0f;
		S12 = R12 + (R13 - R12) / 2.0f;

		S21 = R21 + (R22 - R21) / 2.0f;
		S22 = R22 + (R23 - R22) / 2.0f;


		T11 = S11 + (S12 - S11) / 2.0f;

		T21 = S21 + (S22 - S21) / 2.0f;

		gregory_points[i][0] = T11;
		gregory_points[i][1] = T11 + (T11 - T21);

		gregory_points[i][4] = S12;
		gregory_points[i][5] = S12 + (S12 - S22);
		gregory_points[i][6] = S12 + (S12 - S22);
		gregory_points[i][10] = R13;
		gregory_points[i][11] = R13 + (R13 - R23);

		Q[i] = (3.0f * gregory_points[i][1] - T11) / 2.0f;

		int g = (i + 2) % 3;

		gregory_points[g][16] = points_[i][0][0];
		gregory_points[g][19] = T11;
		gregory_points[g][15] = T11 + (T11 - T21);
		gregory_points[g][18] = S11;
		gregory_points[g][13] = S11 + (S11 - S21);
		gregory_points[g][14] = S11 + (S11 - S21);
		gregory_points[g][17] = R11;
		gregory_points[g][12] = R11 + (R11 - R21);
	}

	glm::vec3 P = (Q[0] + Q[1] + Q[2]) / 3.0f;

	for (int i = 0; i < 3; i++) {
		gregory_points[i][3] = P;
		gregory_points[i][2] = (2.0f * Q[i] + P) / 3.0f;
		gregory_points[i][7] = gregory_points[i][5] + (gregory_points[i][2] - gregory_points[i][1]);
		int g = (i + 2) % 3;
		gregory_points[g][9] = (2.0f * Q[i] + P) / 3.0f;
		gregory_points[g][8] = gregory_points[g][13] + (gregory_points[g][9] - gregory_points[g][15]);

	}
	int i = 0;
	for (auto& greg : patches_gr) {
		if (greg)
		greg->UpdatePoints(gregory_points[i]);
		i++;
	}
}

void TriangularGregoryPatch::find_conected_patches(std::vector<std::vector<std::vector<std::shared_ptr<Point>>>>& patches)
{
	for (int i = 0; i < patches.size(); i++) {
		auto patch1 = patches[i];
		for (int j = i + 1; j < patches.size(); j++) {
			auto patch2 = patches[j];
			for (int k = j + 1; k < patches.size(); k++) {
				auto patch3 = patches[k];
				if (check_patches_connection(patch1, patch2, patch3)) {
					for (int z = 0; z < 2; z++) {
						for (int g = 0; g < 4; g++) {
							points[0][g][z] = patch1[g][z];
							points[1][g][z] = patch2[g][z];
							points[2][g][z] = patch3[g][z];
						}
					}
					is_proper = true;
					return;
				}

				//if (check_patches_connection(patch2, patch1, patch3)) {
				//	for (int z = 0; z < 2; z++) {
				//		for (int g = 0; g < 4; g++) {
				//			points[0][g][z] = patch2[g][z];
				//			points[1][g][z] = patch1[g][z];
				//			points[2][g][z] = patch3[g][z];
				//		}
				//	}
				//	is_proper = true;
				//	return;
				//}
			}
		}
	}
}

bool TriangularGregoryPatch::check_patches_connection(std::vector<std::vector<std::shared_ptr<Point>>>& patch1, std::vector<std::vector<std::shared_ptr<Point>>>& patch2, std::vector<std::vector<std::shared_ptr<Point>>>& patch3)
{
	for (int i = 0; i <= 8; i++) {
		for (int j = 0; j <= 8; j++) {
			for (int k = 0; k <= 8; k++) {
				if (patch1[0][0].get() ==patch3[3][0].get() && patch1[3][0].get() == patch2[0][0].get() && patch2[3][0].get() == patch3[0][0].get()) {
					std::vector<void*> names1 = {};
					std::vector<void*> namesin2 = {};
					bool true_to_return = true;
					for (auto& owner : patch1[0][0]->owners) {
						if (owner.expired()) continue;
						auto ow = std::dynamic_pointer_cast<TriangularGregoryPatch>(owner.lock());
						if(ow)
						names1.push_back(ow.get());
					}
					for (auto& owner : patch1[3][0]->owners) {
						if (owner.expired()) continue;
						auto ow = std::dynamic_pointer_cast<TriangularGregoryPatch>(owner.lock());
						if (ow)
						for (auto& name : names1) {
							if (ow.get() == name) {
								namesin2.push_back(ow.get());
								break;
							}
						}
					}
					for (auto& owner : patch2[3][0]->owners) {
						if (owner.expired()) continue;
						auto ow = std::dynamic_pointer_cast<TriangularGregoryPatch>(owner.lock());
						if (ow)
						for (auto& name : namesin2) {
							if (ow.get() == name) {
								true_to_return = false;
								break;
							}
						}
					}
					if(true_to_return)
					return true;
				}
				if (k % 2 == 1)
					RotatePatch(patch3);
				SwitchPatch(patch3);
			}
			if (j % 2 == 1)
				RotatePatch(patch2);
			SwitchPatch(patch2);
		}
		if (i % 2 == 1)
			RotatePatch(patch1);
		SwitchPatch(patch1);
	}
	return false;
}

void TriangularGregoryPatch::CreateMenu()
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

		if (ImGui::CollapsingHeader("De Bohr Points on Curve")) {
			for (int i = 0; i < patches_gr.size(); i++) {
				if (patches_gr[i]) {
					patches_gr[i]->CreateMenu(); ImGui::SameLine();
					sprintf_s(buf, "Remove###%sRm%d", patches_gr[i]->Name(), i);
					if (ImGui::Button(buf)) {
						to_delete = i;
					}
				}
			}
		}
		if (to_delete >= 0) {
			patches_gr[to_delete].release();
			//patches_gr->DeletePoint(to_delete);
			Update();
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
}

void TriangularGregoryPatch::DrawObject(glm::mat4 mvp_)
{
	moved = false;
	if (!is_proper) return;
	if (need_update) {
		update_object();
		need_update = false;
	}

	mvp = mvp_;

	for (auto& greg : patches_gr) {
		if(greg)
		greg->DrawObject(mvp);
	}
}

void TriangularGregoryPatch::Select()
{
	Object::Select();
	//for (auto& point : patches_gr) {
	//	point->Select();
	//}
}

void TriangularGregoryPatch::UnSelect()
{
	Object::UnSelect();
}

void TriangularGregoryPatch::Update()
{
	need_update = true;
	for (auto& polygon : patches_gr)
		if(polygon)
			polygon->Update();
}
