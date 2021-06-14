#pragma once
#include "Object.h"
#include "BezierFlakeC0.h"
#include "GregoryPatch.h"
class TriangularGregoryPatch :
    public Object,
    public std::enable_shared_from_this<TriangularGregoryPatch>
{
public:
    TriangularGregoryPatch(std::vector<std::vector<std::vector<std::shared_ptr<Point>>>> patches, Shader sh);
    bool IsProper() { return is_proper; };
    void UpdateOwnership();
    void CreateMenu() override;
    void DrawObject(glm::mat4 mvp) override;
    void Select() override;
    void UnSelect() override;

    void Update() override;
    static unsigned int counter;
private:
    bool is_proper{ false };
    std::weak_ptr<Point> points[3][4][2];
    glm::vec3 points_[3][4][2];
    bool draw_polygon{ false };
    bool was_draw_polygon;
    std::vector<std::unique_ptr<GregoryPatch>> patches_gr{};
    glm::vec3 gregory_points[3][20];
    void update_object() override;

    void find_conected_patches(std::vector<std::vector<std::vector<std::shared_ptr<Point>>>>& patches);

    bool check_patches_connection(std::vector<std::vector<std::shared_ptr<Point>>>& patch1,
        std::vector<std::vector<std::shared_ptr<Point>>>& patch2,
        std::vector<std::vector<std::shared_ptr<Point>>>& patch3);


    //TODO move to utils class
    template<class T>
    void RotatePatch(std::vector<std::vector<T>>& patch) {
        auto res = std::vector<std::vector<T>>();
        res.push_back(std::vector<T>(4));
        res.push_back(std::vector<T>(4));
        res.push_back(std::vector<T>(4));
        res.push_back(std::vector<T>(4));

        res[3][0] = patch[0][0];
        res[3][1] = patch[1][0];
        res[3][2] = patch[2][0];
        res[3][3] = patch[3][0];

        res[2][0] = patch[0][1];
        res[2][1] = patch[1][1];
        res[2][2] = patch[2][1];
        res[2][3] = patch[3][1];

        res[1][0] = patch[0][2];
        res[1][1] = patch[1][2];
        res[1][2] = patch[2][2];
        res[1][3] = patch[3][2];

        res[0][0] = patch[0][3];
        res[0][1] = patch[1][3];
        res[0][2] = patch[2][3];
        res[0][3] = patch[3][3];

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                patch[i][j] = res[i][j];
            }
        }
    }

    template<class T>
    void SwitchPatch(std::vector<std::vector<T>>& patch) {
        auto res = std::vector<std::vector<T>>();
        res.push_back(std::vector<T>(4));
        res.push_back(std::vector<T>(4));
        res.push_back(std::vector<T>(4));
        res.push_back(std::vector<T>(4));


        res[3][0] = patch[0][0];
        res[2][0] = patch[1][0];
        res[1][0] = patch[2][0];
        res[0][0] = patch[3][0];

        res[3][1] = patch[0][1];
        res[2][1] = patch[1][1];
        res[1][1] = patch[2][1];
        res[0][1] = patch[3][1];

        res[3][2] = patch[0][2];
        res[2][2] = patch[1][2];
        res[1][2] = patch[2][2];
        res[0][2] = patch[3][2];

        res[3][3] = patch[0][3];
        res[2][3] = patch[1][3];
        res[1][3] = patch[2][3];
        res[0][3] = patch[3][3];

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                patch[i][j] = res[i][j];
            }
        }
    }
};

