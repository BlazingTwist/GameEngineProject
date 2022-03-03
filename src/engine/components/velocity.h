#ifndef ACAENGINE_VELOCITY_H
#define ACAENGINE_VELOCITY_H

#include <glm/glm.hpp>
#include <engine/entity/entityregistry.h>
#include "transform.h"

namespace components {
    struct Velocity {
        Velocity() = default;

        explicit Velocity(const glm::vec3 &_velocity) : velocity(_velocity) {}

        glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

        /**
         * For physical (semi) correctness, apply Velocity FIRST, then update velocity values.
         */
        void applyVelocity(components::Transform &transform, const double &deltaSeconds, const double &deltaSecondsSquared) const {
            transform.setPosition(transform.getPosition() + (velocity * static_cast<float>(deltaSeconds)));
        }

        /**
         * this method assumes that the direction and magnitude of the acceleration remains constant for a duration of `deltaSeconds`
         * if that is not your case, update the velocity vector directly instead of using this method.
         */
        void applyAcceleration(const glm::vec3 &acceleration, components::Transform &transform, const double &deltaSeconds, const double &deltaSecondsSquared) {
            transform.setPosition(transform.getPosition() + (acceleration * static_cast<float>(deltaSecondsSquared / 2.0)));
            velocity += (acceleration * static_cast<float>(deltaSeconds));
        }
    };

    class ApplyVelocitySystem {
    public:
        ApplyVelocitySystem(entity::EntityRegistry &_registry, double _deltaSeconds, double _deltaSecondsSquared)
                : registry(_registry), deltaSeconds(_deltaSeconds), deltaSecondsSquared(_deltaSecondsSquared) {}

        void execute() {
            registry.execute([this]
                                     (const entity::EntityReference *entity, components::Transform transform, components::Velocity velocity) {
                velocity.applyVelocity(transform, deltaSeconds, deltaSecondsSquared);
                registry.addOrSetComponent(entity, transform);
            });
        }

    private:
        entity::EntityRegistry &registry;
        double deltaSeconds;
        double deltaSecondsSquared;
    };
}

#endif //ACAENGINE_VELOCITY_H
