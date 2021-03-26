#include "BezierC0.h"

unsigned int BezierC0::counter = 1;

BezierC0::BezierC0(Shader sh) : Object(sh,7)
{
    sprintf_s(name, 512, ("BezierC0 " + std::to_string(counter)).c_str());
    constname = "BezierC0 " + std::to_string(counter);
    counter++;
    number_of_divisions = 100;
    this->color = {1.0f,1.0f,1.0f,1.0f};
    update_object();
}

void BezierC0::DrawObject(glm::mat4 mvp_)
{
    if (points.size() < 2) return;

    if (need_update || mvp != mvp_) {
        mvp = mvp_;
        update_object();
        need_update = false;
    }
    else {
        mvp = mvp_;
    }
    int projectionLoc = glGetUniformLocation(shader.ID, "mvp");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    shader.use();
    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_LINES, lines.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void BezierC0::CreateMenu()
{
    float color_new[4];
    char buffer[512];
    char buf[512];
    sprintf_s(buffer, "%s###%sdup", name, constname);
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
        int to_delete = -1;
        if (ImGui::CollapsingHeader("Points on Curve")) {
            for (int i = 0; i < points.size(); i++) {
                auto sp = points[i].lock();
                ImGui::Text(sp->name); ImGui::SameLine();
                sprintf_s(buf, "Remove###%sRm%d", sp->name,i);
                if (ImGui::Button(buf)) {
                    to_delete = i;
                }
            }
        }
        if (to_delete >= 0) {
            points.erase(points.begin() + to_delete);
        }

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

void BezierC0::AddPointToCurve(std::shared_ptr<Point>& point)
{
    if (point.get()){
        points.push_back(point);
        update_object();
    }
}

void BezierC0::Update()
{
    need_update = true;
}

void BezierC0::update_object()
{
    lines.clear();
    points_on_curve.clear();
    points_.clear();

    position = glm::vec3{ 0,0,0 };
    int licznik = 0;
    for (auto& point : points) {
        if (!point.expired()) {
            auto sp = point.lock()->GetPosition();
            points_.push_back(sp);
            position += sp;
            licznik++;
        }
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

void BezierC0::create_curve()
{
    for (int iter = 0; iter+1 < points_.size();) {
        int start = iter;
        int end = iter + 1;
        for (int i = 0; i < 4 && end < points_.size(); i++) { iter++; end++; }
        number_of_divisions = 1;
        for (int odl = start; odl < end-1 ; odl++) {
            glm::vec4 A = { points_[odl],1 };
            glm::vec4 B = { points_[odl + 1],1 };
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
            if(len>0)
            number_of_divisions += len;

        }
        float stride = 1.0f / number_of_divisions;
        float t = 0.0f;
        for (int num = 0; num <= number_of_divisions; num++) {
            auto point = compute_bezier_curve_at_point(start, end, t);
            points_on_curve.push_back(point.x);
            points_on_curve.push_back(point.y);
            points_on_curve.push_back(point.z);
            points_on_curve.push_back(color.r);
            points_on_curve.push_back(color.g);
            points_on_curve.push_back(color.b);
            points_on_curve.push_back(color.a);

            if (num != number_of_divisions) {
                lines.push_back(points_on_curve.size() / (float)description_number - 1);
                lines.push_back(points_on_curve.size() / (float)description_number);
            }

            t += stride;
        }
    }
    if (draw_polygon) {
        int tr = 0;
        for (auto& vec : points_) {
            points_on_curve.push_back(vec.x);
            points_on_curve.push_back(vec.y);
            points_on_curve.push_back(vec.z);
            points_on_curve.push_back(0.0f);
            points_on_curve.push_back(1.0f);
            points_on_curve.push_back(0.0f);
            points_on_curve.push_back(1.0f);
            if (tr != points_.size() - 1) {
                lines.push_back(points_on_curve.size() / (float)description_number - 1);
                lines.push_back(points_on_curve.size() / (float)description_number);
            }
            tr++;
        }
    }
}

glm::vec3 BezierC0::compute_bezier_curve_at_point(int start, int end, float t)
{
    if (end == start) return { 0,0,0 };
    if (t == 0.0f) return points_[start];
    if (t == 1.0f) return points_[--end];
    int sd = 0;
    for (int iter = start; iter+1 < end; iter++) {
        sub_sum[sd] = points_[iter] * (1.0f - t) + points_[iter + 1] * t;
        sd++;
    }

    while (sd > 0) {
        for (int i = 0; i < sd - 1; i++) {
            sub_sum[i] = (sub_sum[i] * (1.0f - t) + sub_sum[i + 1] * t);
        }
        sd--;
    }

    return sub_sum[0];
}
