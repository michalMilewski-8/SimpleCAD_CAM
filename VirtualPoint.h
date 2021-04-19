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
    void UnSelectVirt() override;
    void SelectVirt() override;
    glm::vec3 getPosition() override;
    void MoveVirtObject(glm::vec3) override;
    void MoveVirtObjectTo(glm::vec3) override;

    // Inherited via VirtualObject
    bool SelectedVirt() override;
};

