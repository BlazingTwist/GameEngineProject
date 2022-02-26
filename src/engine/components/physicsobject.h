#ifndef ACAENGINE_PHYSICSOBJECT_H
#define ACAENGINE_PHYSICSOBJECT_H

#include <glm/glm.hpp>

namespace components {
    struct PhysicsObject {
    public:
        PhysicsObject() {}

        PhysicsObject(const double mass) : _mass(mass) {}

        PhysicsObject(const double mass, const glm::vec3 &velocity) : _mass(mass), _velocity(velocity) {}

        double _mass;
        glm::vec3 _velocity;
    };
}

#endif //ACAENGINE_PHYSICSOBJECT_H
