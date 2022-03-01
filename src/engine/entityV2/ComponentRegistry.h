#ifndef ACAENGINE_COMPONENTREGISTRY_v2_H
#define ACAENGINE_COMPONENTREGISTRY_v2_H

#include <typeindex>
#include <unordered_map>
#include <vector>
#include "ComponentReference.h"
#include "EntityReference.h"

namespace entityV2 {
    class ComponentRegistry {

    public:
        /**
         * Find or Create a Component-Registry for the specified Component-Type.
         * @param typeIndex type_index of the Component-Type
         * @return A ComponentRegistry instance for the Component-Type
         */
        static ComponentRegistry *getInstance(std::type_index typeIndex) {
            static std::unordered_map<std::type_index, ComponentRegistry *> registryInstances = {};
            auto findResult = registryInstances.find(typeIndex);
            if (findResult == registryInstances.end()) {
                auto *registry = new ComponentRegistry();
                registryInstances[typeIndex] = registry;
                return registry;
            }
            return findResult->second;
        }

        ComponentReference *addComponent(unsigned int entityID, void *componentPtr) {
            auto *reference = new ComponentReference(entityID, components.size(), componentPtr);
            components.push_back(reference);
            return reference;
        }

        void removeComponent(ComponentReference *component) {
            const unsigned int componentID = component->_componentID;
            if (componentID == components.size() - 1) {
                components.pop_back();
            } else {
                ComponentReference *moveComponent = components.back();
                moveComponent->_componentID = componentID;
                const auto componentIterator = components.begin() + (int) componentID;
                *componentIterator = moveComponent;
                components.pop_back();
            }
        }

    private:
        explicit ComponentRegistry() = default;

        std::vector<ComponentReference *> components = {};

    };
}

#endif //ACAENGINE_COMPONENTREGISTRY_v2_H
