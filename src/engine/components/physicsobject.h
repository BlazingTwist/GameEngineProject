#ifndef ACAENGINE_PHYSICSOBJECT_H
#define ACAENGINE_PHYSICSOBJECT_H

#include <glm/glm.hpp>
#include <engine/entity/entityregistry.h>

namespace components {
    struct PhysicsObject {
    public:
        PhysicsObject() {}

        explicit PhysicsObject(const double mass) : _mass(mass) {}

        PhysicsObject(const double mass, const glm::vec3 &velocity) : _mass(mass), _velocity(velocity) {}

        PhysicsObject(const glm::vec3 &velocity, const math::AABB<3, float> &aabb, glm::vec3 &angularVelocity) : _velocity(velocity), _aabb(aabb),
                                                                                                                 _angularVelocity(angularVelocity) {}

        PhysicsObject(const glm::vec3 &velocity, const math::AABB<3, float> &aabb) : _velocity(velocity), _aabb(aabb) {}

        double _mass = 1.0;
        glm::vec3 _velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        math::AABB<3, float> _aabb;
        glm::vec3 _angularVelocity;

        class CrossObjectGravitySystem {
        public:
            CrossObjectGravitySystem(entity::EntityRegistry &_registry, double _deltaSeconds, double _deltaSecondsSquared)
                    : registry(_registry), deltaSeconds(_deltaSeconds), deltaSecondsSquared(_deltaSecondsSquared) {}

            void operator()(const entity::EntityReference *entity2, components::Transform transform2, components::PhysicsObject phys2) const {
                registry.execute(
                        [this, entity2, transform2, phys2](const entity::EntityReference *entity,
                                                           components::Transform transform,
                                                           components::PhysicsObject phys) {
                            if (entity->getReferenceID() == entity2->getReferenceID()) {
                                return;
                            }
                            static constexpr double gravConstant = 6.6743e-5;
                            glm::vec3 aToB = transform2.getPosition() - transform.getPosition();
                            glm::vec3 aToBNormal = glm::normalize(aToB);
                            float distanceSquared = glm::dot(aToB, aToB);
                            double acceleration = gravConstant * phys2._mass / distanceSquared;
                            double velocityGain = acceleration * deltaSeconds;
                            double accelerationDistance = acceleration * deltaSecondsSquared / 2;
                            transform.setPosition(
                                    transform.getPosition() + (phys._velocity * (float) deltaSeconds) + (aToBNormal * (float) accelerationDistance));
                            phys._velocity = phys._velocity + (aToBNormal * (float) velocityGain);
                            entity::EntityRegistry::getInstance().addOrSetComponent(entity, transform);
                            entity::EntityRegistry::getInstance().addOrSetComponent(entity, phys);
                        });
            }

        private:
            entity::EntityRegistry &registry;
            double deltaSeconds;
            double deltaSecondsSquared;
        };
    };
}

#endif //ACAENGINE_PHYSICSOBJECT_H
