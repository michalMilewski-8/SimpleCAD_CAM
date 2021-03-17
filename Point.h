#pragma once
#include "Object.h"
class Point :
    public Object
{
public:
    Point(glm::vec3 position, glm::vec4 color, Shader sh);

    virtual void DrawObject(glm::mat4 mvp);
    virtual void CreateMenu();

    static unsigned int counter;
private:
    void update_object();

};

