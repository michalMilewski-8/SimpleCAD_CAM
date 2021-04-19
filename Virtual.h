#pragma once
#include <glm/glm.hpp>
class VirtualObject
{
public:
	virtual void UnSelectVirt() = 0;
	virtual void SelectVirt() = 0;
	virtual bool SelectedVirt() = 0;
	virtual glm::vec3 getPosition() = 0;
	virtual void MoveVirtObject(glm::vec3) {};
	virtual void MoveVirtObjectTo(glm::vec3) {};
};