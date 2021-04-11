#pragma once
#include "Object.h"
#include "Point.h"
class Line :
    public Object
{
public:
    Line(Shader sh);

    void DrawObject(glm::mat4 mvp) override;
    void CreateMenu() override {}

    void AddPoint(std::shared_ptr<Point>& point);
    void Update() override;

    static unsigned int counter;
private:

    void update_object() override;

    std::vector<float> points_on_curve;
    std::vector<unsigned int> lines;

    std::vector<std::weak_ptr<Point>> points;
    std::vector<glm::vec3> points_;
};

