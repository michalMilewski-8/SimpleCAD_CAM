#pragma once
#include "Object.h"
class Point :
    public Object
{
public:
    Point(glm::vec3 position, glm::vec4 color, Shader sh);
    Point(glm::vec3 position, glm::vec4 color, Shader sh, bool virt);

    void DrawObject(glm::mat4 mvp) override;
    void CreateMenu() override;
    
    void AddOwner(std::shared_ptr<Object> owner);

    static unsigned int counter;
private:
    void update_object() override;
    void inform_owner_of_change() override;
protected:
    std::vector<std::weak_ptr<Object>> owners = {};

};

