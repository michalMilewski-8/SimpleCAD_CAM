#pragma once
#include "Object.h"
#include "Point.h"
class BezierC0 :
    public Object
{
public:
    BezierC0(Shader sh);

    void DrawObject(glm::mat4 mvp) override;
    void CreateMenu() override;

    void AddPointToCurve(std::shared_ptr<Point>& point);
    void Update() override;

    static unsigned int counter;
private:

    void update_object() override;

    void create_curve();

    std::vector<float> points_on_curve;
    std::vector<unsigned int> lines;

    int number_of_divisions;
    bool draw_polygon;
    bool was_draw_polygon;

    glm::vec3 sub_sum[5];
    std::vector<std::weak_ptr<Point>> points;
    std::vector<glm::vec3> points_;
    glm::vec3 compute_bezier_curve_at_point(int start,int end, float t);
};

