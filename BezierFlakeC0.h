#pragma once
#include "Bezier.h"
#include "Line.h"
#include "VirtualPoint.h"

class BezierFlakeC0 :
    public Bezier
{
public:
    BezierFlakeC0(Shader sh, int type, glm::uvec2 flakes_count, glm::vec2 sizes) :Bezier(sh), polygons() {
        create_vertices(type, flakes_count, sizes);
        sprintf_s(name, 512, ("BezierFlakeC0 " + std::to_string(counter)).c_str());
        constname = "BezierFlakeC0 " + std::to_string(counter++);
        num_of_flakes = flakes_count;
        number_of_divisions[0] = 4;
        number_of_divisions[1] = 4;
        type_ = type;
        this->color = { 1.0f,1.0f,1.0f,1.0f };
        update_object();
        shader = Shader("tes_shader.vs", "tes_shader.fs", "tes_shader.tcs", "tes_shader.tes");
    }

    BezierFlakeC0(Shader sh) :Bezier(sh), polygons() {}
    BezierFlakeC0(Shader sh, glm::uvec2 flakes_count, glm::uvec2 divisions_, std::vector<std::shared_ptr<Point>> points);

    void Serialize(xml_document<>& document, xml_node<>* scene) override;

    void DrawObject(glm::mat4 mvp) override;
    void CreateMenu() override;

    std::vector<Object*> GetVirtualObjects();

    void Update() override;

    static unsigned int counter;
protected:
    std::vector<unsigned int> patches;
    std::vector<std::shared_ptr<Line>> polygons;
    std::vector<std::shared_ptr<Point>> points;
    int number_of_divisions[2];
    glm::uvec2 num_of_flakes;
    void update_object() override;
    int type_{ 0 };

private:
    virtual void create_vertices(int type, glm::uvec2 flakes_count, glm::vec2 sizes);


    void create_curve();
    void index_vertices();


    std::vector<float> points_on_curve;

    bool draw_polygon{ false };
    bool was_draw_polygon;

    std::vector<glm::vec3> points_;
};

