#pragma once
#include "Bezier.h"
#include "Point.h"
#include "Line.h"
class BezierC0 :
    public Bezier,
    public std::enable_shared_from_this<BezierC0>
{
public:
    BezierC0(Shader& sh);

    void DrawObject(glm::mat4 mvp) override;
    void CreateMenu() override;

    void AddPointToCurve(std::shared_ptr<Point>& point) override;
    void Update() override;
    void Serialize(xml_document<>& document, xml_node<>* scene) override;
    void UpdateMyPointer(std::string constname_, std::shared_ptr<Object> new_point) override;

    static unsigned int counter;
private:

    void update_object() override;

    void create_curve();

    std::shared_ptr<Line> polygon;

    std::vector<float> points_on_curve;
    std::vector<unsigned int> lines;

    int number_of_divisions;
    bool draw_polygon{ false };
    bool was_draw_polygon;

    glm::vec3 sub_sum[5];
    std::vector<std::weak_ptr<Point>> points;
    std::vector<glm::vec3> points_;
    glm::vec3 compute_bezier_curve_at_point(int start,int end, float t);
};

