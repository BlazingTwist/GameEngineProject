#ifndef ACAENGINE_ORBITALOBJECT_H
#define ACAENGINE_ORBITALOBJECT_H

#include <glm/glm.hpp>
#include <vector>
#include <engine/entity/entityregistry.h>
#include "transform.h"
#include "velocity.h"

namespace components {
    struct OrbitalObject {
    public:
        OrbitalObject() = default;

        explicit OrbitalObject(const double _mass) : mass(_mass) {}

        double mass = 1.0;
    };

    class OrbitalSystem {
    private:
        struct OrbitalQueryEntry {
            OrbitalQueryEntry(const entity::EntityReference *entity, const Transform &transform, const Velocity &velocity, const OrbitalObject &orbital)
                    : entity(entity), transform(transform), velocity(velocity), orbital(orbital) {}

            const entity::EntityReference *entity;
            components::Transform transform;
            components::Velocity velocity;
            components::OrbitalObject orbital;
        };

    public:
        OrbitalSystem(entity::EntityRegistry &_registry, double _deltaSeconds, double _deltaSecondsSquared)
                : registry(_registry), deltaSeconds(_deltaSeconds), deltaSecondsSquared(_deltaSecondsSquared) {}

        void execute() {
            std::vector<OrbitalQueryEntry> orbitalEntities = {};
            registry.execute([this, &orbitalEntities]
                                     (const entity::EntityReference *entity, components::Transform transform, components::Velocity velocity,
                                      components::OrbitalObject orbital) {
                for (OrbitalQueryEntry &other: orbitalEntities) {
                    static constexpr double gravConstant = 6.6743e-5;
                    glm::vec3 vecToOther = other.transform.getPosition() - transform.getPosition();
                    glm::vec3 vecToOtherNormal = glm::normalize(vecToOther);

                    float distanceSquared = glm::dot(vecToOther, vecToOther);
                    double acceleration = gravConstant * other.orbital.mass / distanceSquared;
                    double otherAcceleration = gravConstant * orbital.mass / distanceSquared;
                    
                    float collisionDistanceSquared = transform.getScale().x + other.transform.getScale().x;
                    if (collisionDistanceSquared > distanceSquared){
                        spdlog::error("planet-collision with entity: {} and {}", entity->getReferenceID(), other.entity->getReferenceID());
                    }

                    // it is a lot easier to create stable orbits like this - not entirely sure why
                    // my assumption is this:
                    // when orbiting an object, the orbiting object spends a lot less time nearby the orbited object than far away
                    // but the delta-time is constant so the acceleration effect of being near the orbited object is exaggerated
                    // whereas the acceleration effect of being far away is weakened
                    // as a result objects tend to either be "pulled in" or they drift away
                    velocity.velocity += (vecToOtherNormal * static_cast<float>(acceleration * deltaSeconds));
                    other.velocity.velocity += (vecToOtherNormal * static_cast<float>((-otherAcceleration) * deltaSeconds));
                    
                    /*velocity.applyAcceleration(vecToOtherNormal * static_cast<float>(acceleration), transform, deltaSeconds, deltaSecondsSquared);
                    other.velocity.applyAcceleration(vecToOtherNormal * static_cast<float>(-otherAcceleration), transform, deltaSeconds, deltaSecondsSquared);*/
                }
                orbitalEntities.emplace_back(entity, transform, velocity, orbital);
            });
            for (const auto &queryEntry: orbitalEntities) {
                registry.addOrSetComponent(queryEntry.entity, queryEntry.transform);
                registry.addOrSetComponent(queryEntry.entity, queryEntry.velocity);
            }
        }

    private:
        entity::EntityRegistry &registry;
        double deltaSeconds;
        double deltaSecondsSquared;
    };
}

#endif //ACAENGINE_ORBITALOBJECT_H
