#pragma once
#include "BezierFlakeC0.h"
class BezierFlakeC2 :
    public BezierFlakeC0
{
public:
    BezierFlakeC2(Shader sh, int type, glm::uvec2 flakes_count, glm::vec2 sizes);
    BezierFlakeC2(Shader sh, glm::uvec2 flakes_count, glm::uvec2 divisions_, std::vector<std::shared_ptr<Point>> points);
    void Serialize(xml_document<>& document, xml_node<>* scene) override;
    void UpdateMyPointer(std::string constname_, std::shared_ptr<Object> new_point) override;
    std::vector<std::function<glm::vec3(double, double)>> GetParametrisations() override;
    std::vector<std::function<glm::vec3(double, double)>> GetUParametrisations() override;
    std::vector<std::function<glm::vec3(double, double)>> GetVParametrisations() override;
private:
    void create_vertices(int type, glm::uvec2 flakes_count, glm::vec2 sizes) override;

    void index_vertices();
};

