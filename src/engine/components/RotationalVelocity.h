#ifndef ACAENGINE_ROTATIONALVELOCITY_H
#define ACAENGINE_ROTATIONALVELOCITY_H

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <engine/entity/entityregistry.h>
#include "transform.h"

namespace components {
    struct RotationalVelocity {
    public:
        RotationalVelocity() = default;

        explicit RotationalVelocity(const glm::vec3 &_eulerAngleVelocity) : eulerAngleVelocity(_eulerAngleVelocity) {}

        glm::vec3 eulerAngleVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

        void applyRotation(components::Transform &transform, const double &deltaSeconds, const double &deltaSecondsSquared) const {
            transform.setRotation(transform.getRotation() * glm::quat(eulerAngleVelocity * static_cast<float>(deltaSeconds)));
        }
    };

    class ApplyRotationalVelocitySystem {
    public:
        ApplyRotationalVelocitySystem(entity::EntityRegistry &_registry, double _deltaSeconds, double _deltaSecondsSquared)
                : registry(_registry), deltaSeconds(_deltaSeconds), deltaSecondsSquared(_deltaSecondsSquared) {}

        void execute() {
            registry.execute([this]
                                     (const entity::EntityReference *entity, components::Transform transform, components::RotationalVelocity rotVelocity) {
                rotVelocity.applyRotation(transform, deltaSeconds, deltaSecondsSquared);
                registry.addOrSetComponent(entity, transform);
            });
        }

    private:
        entity::EntityRegistry &registry;
        double deltaSeconds;
        double deltaSecondsSquared;
    };
}

#endif //ACAENGINE_ROTATIONALVELOCITY_H
