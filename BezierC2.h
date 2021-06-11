#pragma once
#include "Bezier.h"
#include "Line.h"
#include "VirtualPoint.h"

class BezierC2 :
    public Bezier,
    public std::enable_shared_from_this<BezierC2>
{
public:
    BezierC2(Shader& sh);

    void DrawObject(glm::mat4 mvp) override;
    void CreateMenu() override;

    void AddPointToCurve(std::shared_ptr<Point>& point) override;
    void Update() override;
    std::vector<std::shared_ptr<Object>> GetVirtualObjects() override;
    void Serialize(xml_document<>& document, xml_node<>* scene) override;
    void UpdateMyPointer(std::string constname_, std::shared_ptr<Object> new_point) override;

    static unsigned int counter;
private:

    void update_object() override;

    void create_curve();
    void generate_bezier_points();
    void add_bezier_point(glm::vec3 position);
    void translate_bezier_movement_to_de_boor(int point_index);
    Point* get_de_boor_point(int position);

    std::shared_ptr<Line> polygon;
    std::shared_ptr<Line> polygon_bezier;

    std::vector<float> points_on_curve;
    std::vector<unsigned int> lines;

    int number_of_divisions;
    bool draw_polygon{ false };
    bool was_draw_polygon;
    bool show_bezier_points{ false };

    bool draw_polygon_bezier{ false };
    bool was_draw_polygon_bezier;
    bool need_new_bezier_generation{ true };

    Shader geom_shader;

    glm::vec3 sub_sum[5];
    std::vector<std::weak_ptr<Point>> points;
    std::vector<glm::vec3> de_points_;
    std::vector<std::shared_ptr<Point>> bezier_points;
    std::vector<glm::vec3> points_;
};

