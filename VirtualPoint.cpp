#include "VirtualPoint.h"

unsigned int VirtualPoint::counter = 1;

VirtualPoint::VirtualPoint(glm::vec3 position, Shader sh) :
    Point(position, { 0.25f,0.0f,1.0f, 1.0f }, sh, true) {
    sprintf_s(name, 512, ("Point " + std::to_string(counter)).c_str());
    constname = "Point " + std::to_string(counter);
    counter++;
}
void VirtualPoint::UnSelectVirt() {
    UnSelect();
}
void VirtualPoint::SelectVirt(){
    Select();
}

glm::vec3 VirtualPoint::getPosition()
{
    return GetPosition();
}

void VirtualPoint::MoveVirtObject(glm::vec3 pos)
{
    MoveObject(pos);
}

void VirtualPoint::MoveVirtObjectTo(glm::vec3 pos)
{
    MoveObjectTo(pos);
}

bool VirtualPoint::SelectedVirt()
{
    return selected;
}
