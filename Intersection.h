#pragma once
#include "Object.h"
#include "Point.h"
#include "Line.h"
#include "BezierInterpol.h"

class Intersection :
    public Object
{
public:
    Intersection(Shader sh, std::shared_ptr<Object> obj_left, std::shared_ptr<Object> obj_right);
    void DrawObject(glm::mat4 mvp) override;
    void CreateMenu() override;
    void AddPoints(std::shared_ptr<Point> left, std::shared_ptr<Point> right);
    void AddParameters(glm::vec2 left, glm::vec2 right);
    std::shared_ptr<BezierInterpol> CreateInterpolationBezierLine(bool left = true);
    void Reverse();

    void create_texture(bool reverse_left = false, bool reverse_right = false); //move to private

    static unsigned int counter;
private:
    
    void flood_fill(std::vector<std::vector<bool>>& data);

    std::vector<std::shared_ptr<Point>> points_left;
    std::vector<std::shared_ptr<Point>> points_right;
    std::shared_ptr<Line> line_left;
    std::shared_ptr<Line> line_right;

    std::shared_ptr<BezierInterpol> interpolation_left;
    std::shared_ptr<BezierInterpol> interpolation_right;

    std::vector<glm::vec2> parameters_left;
    std::vector<glm::vec2> parameters_right;

    std::weak_ptr<Object> obj_left;
    std::weak_ptr<Object> obj_right;

    unsigned int texture_left_ID;
    unsigned int texture_right_ID;

    bool show_left = false;
    bool show_right = true;
    bool show_points = false;
    bool show_interpolation = false;
    bool testure_was_created = false;

    bool show_testures = false;
    bool apply_trim_to_left = false;
    bool apply_trim_to_right = false;

    bool fill_left_texture = false;
    bool fill_right_texture = false;

    bool flip_right = false;
    bool flip_left = false;

    const int n = 1024;
};

