#ifndef ACAENGINE_AABBCOLLIDER_H
#define ACAENGINE_AABBCOLLIDER_H

#include <glm/glm.hpp>
#include <engine/entity/entityregistry.h>
#include <engine/math/geometrictypes.hpp>
#include "transform.h"
#include <algorithm>

namespace components {
    struct AABBCollider {
    public:
        AABBCollider() = default;

        explicit AABBCollider(glm::vec3 _positiveBoundOffset, glm::vec3 _negativeBoundOffset)
                : positiveBoundOffset(_positiveBoundOffset), negativeBoundOffset(_negativeBoundOffset) {}

        glm::vec3 positiveBoundOffset = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 negativeBoundOffset = glm::vec3(-1.0f, -1.0f, -1.0f);

        [[nodiscard]] math::AABB<3, float> getAABB(components::Transform transform) const {
            // because of rotation, we're not guaranteed that these are the actual min/max points -> gather min components individually
            glm::vec3 pointA = transform.getPosition() + (transform.getRotation() * (transform.getScale() * negativeBoundOffset));
            glm::vec3 pointB = transform.getPosition() + (transform.getRotation() * (transform.getScale() * positiveBoundOffset));
            return {
                    glm::vec3(std::min(pointA.x, pointB.x), std::min(pointA.y, pointB.y), std::min(pointA.z, pointB.z)),
                    glm::vec3(std::max(pointA.x, pointB.x), std::max(pointA.y, pointB.y), std::max(pointA.z, pointB.z))
            };
        }
    };
}

#endif //ACAENGINE_AABBCOLLIDER_H
