#pragma once
#include "Object.h"
#include "Point.h"

class Bezier :
    public Object
{
public:
    Bezier(Shader& sh) : Object(sh, 7) {};

    virtual void AddPointToCurve(std::shared_ptr<Point>& point) {};
}