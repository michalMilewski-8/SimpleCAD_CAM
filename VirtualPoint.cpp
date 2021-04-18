#include "VirtualPoint.h"

unsigned int VirtualPoint::counter = 1;

VirtualPoint::VirtualPoint(glm::vec3 position, Shader sh) :
    Point(position, { 0.25f,0.0f,1.0f, 1.0f }, sh, true) {
    sprintf_s(name, 512, ("Point " + std::to_string(counter)).c_str());
    constname = "Point " + std::to_string(counter);
    counter++;
}
void VirtualPoint::UnSelectVirt() {

}
void VirtualPoint::SelectVirt(){

}
