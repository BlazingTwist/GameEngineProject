#ifndef ACAENGINE_COMPONENTREGISTRY_H
#define ACAENGINE_COMPONENTREGISTRY_H

#include <vector>
#include <optional>
#include <typeindex>
#include <unordered_map>
#include "component.h"

namespace entity {
    struct ExComp1 {
        int data = 1337;
    };
    struct ExComp2 {
        float data = 69.420f;
    };
    struct ExComp3 {
        std::string data = "hello world";
    };

    class ComponentRegistry {
    public:
        static ComponentRegistry *getInstance(std::type_index typeIndex) {
            static std::unordered_map<std::type_index, ComponentRegistry *> registryInstances = {};
            if (registryInstances.find(typeIndex) == registryInstances.end()) {
                auto *registry = new ComponentRegistry();
                registryInstances[typeIndex] = registry;
                return registry;
            }
            return registryInstances[typeIndex];
        }

        template<typename T_component>
        int addComponent(int entityReferenceID, T_component componentData) {
            auto componentId = (int) components.size();
            Component component = Component(entityReferenceID, componentData);
            components.push_back(component);
            return componentId;
        }
        
        template<typename T_component>
        void setComponentData(int componentID, T_component componentData){
            components[componentID].template setData(componentData);
        }

        void removeComponent(int componentId, int& movedEntityID, int& movedComponentID) {
            if (componentId == components.size() - 1) {
                components.pop_back();
            } else {
                const auto componentIterator = components.begin() + componentId;
                Component component = components.back();
                *componentIterator = component;
                components.pop_back();
                
                movedEntityID = component.entityReferenceID;
                movedComponentID = componentId;
            }
        }

        template<typename T_component>
        [[nodiscard]] T_component getComponentData(const int componentId) const {
            return components[componentId].template getComponentData<T_component>();
        }

    private:
        std::vector<Component> components = {};
    };
}

#endif //ACAENGINE_COMPONENTREGISTRY_H
