#pragma once
#include "BezierFlakeC0.h"
class BezierFlakeC2 :
    public BezierFlakeC0
{
public:
    BezierFlakeC2(Shader sh, int type, glm::uvec2 flakes_count, glm::vec2 sizes);
    BezierFlakeC2(Shader sh, glm::uvec2 flakes_count, glm::uvec2 divisions_, std::vector<std::shared_ptr<Point>> points);
private:
    void create_vertices(int type, glm::uvec2 flakes_count, glm::vec2 sizes) override;

    void index_vertices();
};

