#pragma once
#include <glm/glm.hpp>
class VirtualObject
{
public:
	virtual void UnSelectVirt() = 0;
	virtual void SelectVirt() = 0;
	virtual void getPosition() {};
	virtual void MoveVirtObject(glm::vec3) {};
};