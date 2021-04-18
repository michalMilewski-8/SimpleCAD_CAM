#pragma once
#include "Point.h"
#include "Virtual.h"

class VirtualPoint :
    public Point,
    public VirtualObject
{
public:
    VirtualPoint(glm::vec3 position, Shader sh);
    static unsigned int counter;

    // Inherited via VirtualObject
    virtual void UnSelectVirt() override;
    virtual void SelectVirt() override;
};

