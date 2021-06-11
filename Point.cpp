#include "Point.h"

unsigned int Point::counter = 1;

Point::Point(glm::vec3 position, glm::vec4 color, Shader sh):Object(sh,7)
{
	this->color = color;
	update_object();
	sprintf_s(name, 512, ("Point " + std::to_string(counter)).c_str());
	constname = "Point " + std::to_string(counter);
	counter++;
	MoveObject(position);
}

Point::Point(glm::vec3 position, Shader sh) :Object(sh, 7)
{
	this->color = glm::vec4(54/255.0f, 134 / 255.0f, 199 / 255.0f,1.0f);
	update_object();
	sprintf_s(name, 512, ("Point " + std::to_string(counter)).c_str());
	constname = "Point " + std::to_string(counter);
	counter++;
	MoveObject(position);
}

Point::Point(glm::vec3 position, glm::vec4 color, Shader sh, bool virt) :Object(sh, 7)
{
	this->color = color;
	update_object();
	MoveObject(position);
}

void Point::Serialize(xml_document<>& document, xml_node<>* scene)
{
	auto figure = document.allocate_node(node_element, "Point");
	xml_node <>* position = document.allocate_node(node_element, "Position");
	auto pos = this->position;
	position->append_attribute(document.allocate_attribute("X", document.allocate_string(std::to_string(pos.x).c_str())));
	position->append_attribute(document.allocate_attribute("Y", document.allocate_string(std::to_string(pos.y).c_str())));
	position->append_attribute(document.allocate_attribute("Z", document.allocate_string(std::to_string(pos.z).c_str())));
	figure->append_node(position);
	figure->append_attribute(document.allocate_attribute("Name", document.allocate_string(constname.c_str())));
	scene->append_node(figure);
}

void Point::DrawObject(glm::mat4 mvp)
{
	Object::DrawObject(mvp);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPointSize(5);
	glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Point::CreateMenu()
{
	float color_new[4];
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
		for (int i = 0; i < 4; i++)
			color_new[i] = color[i];
		ImGui::Checkbox("Selected", &selected);
		if (selected != was_selected_in_last_frame) {
			update_object();
			was_selected_in_last_frame = selected;
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

void Point::AddOwner(std::shared_ptr<Object> owner)
{
	owners.push_back(owner);
}

void Point::AddUniqueOwners(std::vector<std::weak_ptr<Object>> owner_list)
{
	for (auto& own : owner_list) {
		if (own.expired())continue;
		bool unique = true;
		for (auto& loc : owners) {
			if (loc.expired()) continue;
			if (loc.lock() == own.lock()) {
				unique = false;
				break;
			}
		}
		if (unique)
			owners.push_back(own);
	}
}

void Point::UpdateOwners(std::shared_ptr<Point>& new_point)
{
	for (auto& owner : owners) {
		if (owner.expired()) continue;
		owner.lock()->UpdateMyPointer(constname, new_point);
	}
}

void Point::update_object()
{
	unsigned int pointt = 0;
	float points[7] = { 0,0,0,color.r,color.g,color.b,1 };

	if (selected) {
		points[3] = 1;
		points[4] = 0;
		points[5] = 0;
	}

	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7, points, GL_DYNAMIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int), &pointt, GL_DYNAMIC_DRAW);
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void Point::inform_owner_of_change()
{
	for (auto& owner : owners) {
		if (!owner.expired()) {
			auto o = owner.lock();
			o->Update();
		}
	}
}
