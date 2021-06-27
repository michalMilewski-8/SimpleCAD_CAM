#include "BezierInterpol.h"

unsigned int BezierInterpol::counter = 1;

BezierInterpol::BezierInterpol(Shader& sh) :
	Bezier(sh),
	polygon(std::make_shared<Line>(sh)),
	polygon_bezier(std::make_shared<Line>(sh)),
	geom_shader("shader_bezier_c0.vs", "shader.fs", "shader_bezier_c0.gs")
{
	sprintf_s(name, 512, ("BezierInterpolated " + std::to_string(counter)).c_str());
	constname = "BezierInterpolated " + std::to_string(counter);
	counter++;
	number_of_divisions = 100;
	points = std::vector<std::weak_ptr<Point>>();
	this->color = { 1.0f,1.0f,1.0f,1.0f };
	update_object();
}

void BezierInterpol::DrawObject(glm::mat4 mvp_)
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
	if (show_de_boor_points) {
		for (auto point : de_boor_points) {
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

void BezierInterpol::CreateMenu()
{
	float color_new[4];
	char buffer[512];
	char buf[512];
	int to_delete = -1;
	sprintf_s(buffer, "%s###%sdup22", name, constname);
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
		ImGui::Checkbox("Show De Boor Points", &show_de_boor_points);
		ImGui::Checkbox("Draw De Boor Polygon", &draw_polygon);
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

		if (ImGui::CollapsingHeader("Points on Curve")) {
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

void BezierInterpol::AddPointToCurve(std::shared_ptr<Point>& point)
{
	if (point.get()) {
		points.push_back(point);
		point->AddOwner(shared_from_this());
		Update();
	}
}

void BezierInterpol::Update()
{
	need_update = true;
	polygon->Update();
	polygon_bezier->Update();
}

void BezierInterpol::Serialize(xml_document<>& document, xml_node<>* scene)
{
	auto figure = document.allocate_node(node_element, "BezierInter");
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

std::vector<std::shared_ptr<Object>> BezierInterpol::GetVirtualObjects()
{
	auto res = std::vector<std::shared_ptr<Object>>();
	// TODO zastanowiæ siê czy coœ zwracaæ
	return res;
}

void BezierInterpol::UpdateMyPointer(std::string constname_, const std::shared_ptr<Object> new_point)
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

void BezierInterpol::Reverse()
{
	std::reverse(points.begin(), points.end());
	Update();
}

void BezierInterpol::update_object()
{
	points_.clear();
	for (auto& point : points) {
		if (!point.expired()) {
			auto point_ = point.lock()->GetPosition();
			if(points_.size() == 0 || point_ != points_.back())
				points_.push_back(point_);
		}
	}
	// TODO: zrobiæ dobre genereowanie
	if (points_.size() < 3) return;
	generate_bezier_points();

	// wrzucanie punktów beziera w nasze punkty do rysowania
	bezier_points_.clear();
	position = glm::vec3{ 0,0,0 };
	int licznik = 0;
	for (auto& point : bezier_points) {
		auto sp = point->GetPosition();
		bezier_points_.push_back(sp);
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

void BezierInterpol::create_curve()
{
	points_on_curve.clear();
	lines.clear();
	// Liczenie koniecznej iloœci podzia³ów ¿eby by³o git
	for (int iter = 0; iter + 1 < bezier_points_.size();) {
		int start = iter;
		int end = iter + 1;
		for (int i = 0; i < 3 && end < bezier_points_.size(); i++) { iter++; end++; }
		int number_of_divisions_loc = 1;
		for (int odl = start; odl < end - 1; odl++) {
			glm::vec4 A = { bezier_points_[odl],1 };
			glm::vec4 B = { bezier_points_[odl + 1],1 };
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

	// Wrzucanie punktów beziera, tak by malowaniem zaj¹ siê geometry shader
	int k = 0;
	for (k = 0; k < bezier_points_.size(); k += 3) {
		for (int j = 0; j < 4; j++) {
			lines.push_back(k + j);
		}
	}

	for (auto& vec : bezier_points_) {
		points_on_curve.push_back(vec.x);
		points_on_curve.push_back(vec.y);
		points_on_curve.push_back(vec.z);
		points_on_curve.push_back(color.r);
		points_on_curve.push_back(color.g);
		points_on_curve.push_back(color.b);
		points_on_curve.push_back(color.a);
	}
	if (lines.size() == 0) return;
	int left = (lines.back() - bezier_points_.size()) + 1;

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

void scale_bezier_points(float t, glm::vec3& b0, glm::vec3& b1, glm::vec3& b2, glm::vec3& b3) {
	glm::vec3 l10, l11, l12,
			  l20, l21,
			  l30;
	float tminus = 1.0f - t;
	l10 = b0 * tminus + b1 * t;
	l11 = b1 * tminus + b2 * t;
	l12 = b2 * tminus + b3 * t;

	l20 = l10 * tminus + l11 * t;
	l21 = l11 * tminus + l12 * t;

	l30 = l20 * tminus + l21 * t;

	b0 = b0;
	b1 = l10;
	b2 = l20;
	b3 = l30;
}

void change_base(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, float di, glm::vec3& b0, glm::vec3& b1, glm::vec3& b2, glm::vec3& b3) {

	glm::mat4 base_change_mtx = {
	1.0f, 1.0f, 1.0f , 1.0f,
	0 , (1.0f / 3.0f) , (2.0f / 3.0f) , 1.0f,
	0, 0, (1.0f / 3.0f) , 1.0f,
	0, 0,0,1.0f
	};

	glm::vec4 x, y, z;
	x = base_change_mtx * glm::vec4{ a.x,b.x,c.x,d.x };
	y = base_change_mtx * glm::vec4{ a.y,b.y,c.y,d.y };
	z = base_change_mtx * glm::vec4{ a.z,b.z,c.z,d.z };
	b0 = { x.x,y.x,z.x };
	b1 = { x.y,y.y,z.y };
	b2 = { x.z,y.z,z.z };
	b3 = { x.w,y.w,z.w };

	scale_bezier_points(di, b0, b1, b2, b3);

}

void solve_strip_matrix(const std::vector<float>& a, const std::vector<float>& c, const std::vector<glm::vec3>& d, std::vector<glm::vec3>& x) {
	std::vector<float> cp = std::vector<float>();
	std::vector<glm::vec3> dp = std::vector<glm::vec3>();
	cp.push_back(0);
	cp.push_back(c[1] / 2.0f);
	dp.push_back({ 0,0,0 });
	dp.push_back(d[1] / 2.0f);
	for (size_t i = 2; i < d.size() - 1; i++) {
		cp.push_back(c[i] / (2.0f - a[i] * cp[i - 1]));
	}
	for (size_t i = 2; i < d.size(); i++) {
		dp.push_back((d[i] - a[i] * dp[i - 1]) / (2.0f - a[i] * cp[i - 1]));
	}

	std::vector<glm::vec3> xtmp = std::vector<glm::vec3>();
	xtmp.push_back(dp.back());
	for (int i = d.size() - 2; i > 0; i--) {
		xtmp.push_back(dp[i] - cp[i] * xtmp.back());
	}

	while (xtmp.size() > 0) {
		x.push_back(xtmp.back());
		xtmp.pop_back();
	}
}

void BezierInterpol::generate_bezier_points()
{
	de_boor_points.clear();
	de_points_.clear();

	bezier_points.clear();

	polygon_bezier->ClearPoints();
	polygon->ClearPoints();

	std::vector<glm::vec3> a = std::vector<glm::vec3>();
	std::vector<glm::vec3> b = std::vector<glm::vec3>();
	std::vector<glm::vec3> c = std::vector<glm::vec3>();
	std::vector<glm::vec3> d = std::vector<glm::vec3>();
	std::vector<float> di = std::vector<float>();
	std::vector<float> alpha = std::vector<float>();
	std::vector<float> beta = std::vector<float>();
	std::vector<glm::vec3> R = std::vector<glm::vec3>();

	for (auto& point : points_) {
		a.push_back(point);
	}

	for (size_t i = 0; i < points_.size()-1; i++) {
		di.push_back(glm::distance(points_[i], points_[i + 1]));
	}

	alpha.push_back(0);
	beta.push_back(0);
	R.push_back({ 0, 0, 0 });
	for (size_t i = 1; i < points_.size() - 1; i++) {
		alpha.push_back(di[i - 1] / (di[i - 1] + di[i]));
		beta.push_back(di[i] / (di[i - 1] + di[i]));
		R.push_back(3.0f*(((points_[i + 1] - points_[i]) / di[i]) - ((points_[i] - points_[i - 1]) / di[i - 1])) / (di[i - 1] + di[i]));
	}

	c.push_back({ 0,0,0 });
	solve_strip_matrix(alpha, beta, R, c);
	c.push_back({ 0,0,0 });

	for (size_t i = 1; i < points_.size(); i++) {
		d.push_back((c[i] - c[i - 1]) / (3 * di[i - 1]));
	}
	d.push_back({ 0,0,0 });

	for (size_t i = 1; i < points_.size(); i++) {
		b.push_back((a[i] - a[i-1] - c[i-1]*di[i-1]*di[i-1] - d[i-1] * di[i - 1] * di[i - 1] * di[i - 1]) / di[i - 1]);
	}
	b.push_back({ 0,0,0 });

	glm::vec3 b0, b1, b2, b3;

	change_base(a[0], b[0], c[0], d[0], di[0], b0, b1, b2, b3);
	add_bezier_point(b0);
	add_bezier_point(b1);
	add_bezier_point(b2);
	add_bezier_point(b3);

	for (size_t i = 1; i < points_.size() - 1; i++) {
		change_base(a[i], b[i], c[i], d[i], di[i], b0, b1, b2, b3);
		add_bezier_point(b1);
		add_bezier_point(b2);
		add_bezier_point(b3);
	}
}

void BezierInterpol::add_bezier_point(glm::vec3 position)
{
	auto point = std::make_shared<Point>(position, shader);
	bezier_points.push_back(point);
	point->AddOwner(shared_from_this());
	polygon_bezier->AddPoint(point);
	point->AddOwner(polygon_bezier);
}

void BezierInterpol::add_de_boor_point(glm::vec3 position)
{
	auto point = std::make_shared<Point>(position, shader);
	de_boor_points.push_back(point);
	point->AddOwner(shared_from_this());
	polygon->AddPoint(point);
	point->AddOwner(polygon);
}

