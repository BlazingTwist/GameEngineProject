#ifndef ACAENGINE_SCALEVELOCITY_H
#define ACAENGINE_SCALEVELOCITY_H

#include <glm/glm.hpp>
#include <engine/entity/entityregistry.h>
#include "transform.h"

namespace components {
    /**
     * Component that modifies the Scale of a Transform over time
     */
    struct ScaleVelocity {
    public:
        ScaleVelocity() = default;

        explicit ScaleVelocity(const glm::vec3 &_velocity) : velocity(_velocity) {}

        glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

        void applyScale(components::Transform &transform, const double &deltaSeconds, const double &deltaSecondsSquared) const {
            transform.setScale(transform.getScale() + (velocity * static_cast<float>(deltaSeconds)));
        }
    };

    class ApplyScaleVelocitySystem {
    public:
        ApplyScaleVelocitySystem(entity::EntityRegistry &_registry, double _deltaSeconds, double _deltaSecondsSquared)
                : registry(_registry), deltaSeconds(_deltaSeconds), deltaSecondsSquared(_deltaSecondsSquared) {}

        void execute() {
            registry.execute([this]
                                     (const entity::EntityReference *entity, components::Transform transform, components::ScaleVelocity scaleVelocity) {
                scaleVelocity.applyScale(transform, deltaSeconds, deltaSecondsSquared);
                registry.addOrSetComponent(entity, transform);
            });
        }

    private:
        entity::EntityRegistry &registry;
        double deltaSeconds;
        double deltaSecondsSquared;
    };
}

#endif //ACAENGINE_SCALEVELOCITY_H
