#pragma once
#include "Object.h"
class Cursor :
    public Object
{
public:

    Cursor(Shader sh);
    void SetCursorPosition(glm::vec3 position);
    virtual void DrawObject(glm::mat4 mvp);

private:
    std::vector<float> points;
    std::vector<unsigned int> lines;
};

