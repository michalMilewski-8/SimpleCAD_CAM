#pragma once
#include "Bezier.h"
#include "Line.h"
#include "VirtualPoint.h"

class BezierFlakeC0 :
    public Bezier,
    public std::enable_shared_from_this<BezierFlakeC0>
{
public:
    BezierFlakeC0(Shader sh, int type, glm::uvec2 flakes_count, glm::vec2 sizes) :Bezier(sh), polygons() {
        create_vertices(type, flakes_count, sizes);
        sprintf_s(name, 512, ("BezierFlakeC0 " + std::to_string(counter)).c_str());
        constname = "BezierFlakeC0 " + std::to_string(counter);
        counter++;
        this->color = { 1.0f,1.0f,1.0f,1.0f };
        update_object();
    }

    void DrawObject(glm::mat4 mvp) override;
    void CreateMenu() override;

    std::vector<VirtualObject*> GetVirtualObjects();

    void Update() override;

    static unsigned int counter;
private:
    void create_vertices(int type, glm::uvec2 flakes_count, glm::vec2 sizes);

    void update_object() override;

    void create_curve();

    std::vector<std::shared_ptr<Line>> polygons;

    std::vector<float> points_on_curve;
    std::vector<unsigned int> patches;

    int number_of_divisions_u;
    int number_of_divisions_v;
    bool draw_polygon;
    bool was_draw_polygon;

    std::vector<std::shared_ptr<VirtualPoint>> points;
    std::vector<glm::vec3> points_;
};

