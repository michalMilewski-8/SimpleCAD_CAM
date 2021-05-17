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

void VirtualPoint::AddOwner(Object* owner)
{
    ownerss.push_back(owner);
}

bool VirtualPoint::SelectedVirt()
{
    return selected;
}

void VirtualPoint::AddOwner(std::shared_ptr<Object> owner)
{
    owners.push_back(owner);
}

void VirtualPoint::inform_owner_of_change()
{
    for (auto& owner : owners) {
        if (!owner.expired()) {
            auto o = owner.lock();
            o->Update();
        }
    }

    for (auto& owner : ownerss) {
        owner->Update();
    }

}
