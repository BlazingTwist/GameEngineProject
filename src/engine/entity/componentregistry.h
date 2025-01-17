﻿#ifndef ACAENGINE_COMPONENTREGISTRY_H
#define ACAENGINE_COMPONENTREGISTRY_H

#include <vector>
#include <optional>
#include <typeindex>
#include <unordered_map>
#include "entityreference.h"
#include <engine/utils/assert.hpp>

namespace entity {
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

        /**
         * Store Component-Data for a given Entity.
         * @tparam T_component Type of the Component to store
         * @param entityID EntityID to the Entity the Component belongs to
         * @param componentData Component-Data to store
         * @return The Component-ID the Component was stored under
         */
        template<typename T_component>
        int addComponent(int entityID, const T_component &componentData) {
            // http://www.cplusplus.com/forum/beginner/155821/
            // http://en.cppreference.com/w/cpp/types/is_trivially_copyable
            static_assert(std::is_trivially_copyable<T_component>::value, "not a TriviallyCopyable type");
            checkComponentSize((int) sizeof(T_component));

            std::size_t dataSize = componentsBytes.size();
            componentsBytes.resize(dataSize + intByteSize + componentByteSize);

            // copy entityID to componentsBytes
            *reinterpret_cast<int *>(componentsBytes.data() + dataSize) = entityID;

            // copy componentData to componentsBytes
            *reinterpret_cast<T_component *>(componentsBytes.data() + dataSize + intByteSize) = componentData;

            int componentID = componentCount;
            componentCount++;
            return componentID;
        }

        /**
         * Update the EntityID of a stored component.
         * @param componentID ComponentID of the component whose EntityID changed
         * @param newEntityID new value of the EntityID
         */
        void setEntityID(int componentID, int newEntityID) {
            *reinterpret_cast<int *>(componentsBytes.data() + (componentID * (intByteSize + componentByteSize))) = newEntityID;
        }

        /**
         * Update the Component-Data of a stored component
         * @tparam T_component Type of the Component to store.
         * @param componentID ComponentID of the component to modify.
         * @param componentData Component-Data to store.
         */
        template<typename T_component>
        void setComponentData(int componentID, const T_component &componentData) {
            *reinterpret_cast<T_component *>(componentsBytes.data() + (componentID * (intByteSize + componentByteSize)) + intByteSize) = componentData;
        }

        /**
         * Remove a component from this Registry.
         * @param componentId ID of the component to be removed
         * @param movedEntityID if a component was moved, will be assigned the ID of the entity containing the moved component (if assigned, value is in range [0,])
         * @param movedComponentID if a component was moved, will be assigned the new ID of the moved component (if assigned, value is in range [0,])
         */
        void removeComponent(int componentId, int &movedEntityID, int &movedComponentID) {
            const auto removeComponent_begin = componentsBytes.begin() + (componentId * (intByteSize + componentByteSize));
            if (componentId == componentCount - 1) { // removing last component requires no moves
                componentsBytes.erase(removeComponent_begin, componentsBytes.end());
            } else {
                // copy last component to the slot of component that is being removed
                const auto moveComponent_begin = componentsBytes.begin() + ((componentCount - 1) * (intByteSize + componentByteSize));
                const auto moveComponent_end = componentsBytes.end();
                std::copy(moveComponent_begin, moveComponent_end, removeComponent_begin);

                // get the EntityID of the moved component
                auto *movedEntityID_begin = reinterpret_cast<std::byte *>(&movedEntityID);
                std::copy(moveComponent_begin, moveComponent_begin + intByteSize, movedEntityID_begin);

                // since the last component is moved into the slot of the component that is removed, the new movedComponent-ID is the ID of the removed component
                movedComponentID = componentId;

                // remove moved component from end of componentsBytes
                componentsBytes.erase(moveComponent_begin, moveComponent_end);
            }
            componentCount--;
        }

        /**
         * Retrieve the Component-Data of a ComponentID.
         * @tparam T_component Type of the Component to retrieve
         * @param componentId ComponentID to look up
         * @return ComponentData stored for this ID
         */
        template<typename T_component>
        [[nodiscard]] T_component getComponentData(int componentId) const {
            T_component result;
            auto *result_begin = reinterpret_cast<std::byte * > (std::addressof(result));
            const auto componentData_begin = componentsBytes.begin() + (componentId * (intByteSize + componentByteSize)) + intByteSize;
            std::copy(componentData_begin, componentData_begin + componentByteSize, result_begin);
            return result;
        }

    private:
        void checkComponentSize(const int byteSize) {
            if (componentByteSize >= 0) {
                ASSERT(byteSize == componentByteSize, "sizeof(T_Component) of ComponentRegistry changed during execution!");
            } else {
                componentByteSize = byteSize;
            }
        }

        static constexpr int intByteSize = (int) sizeof(int);
        int componentCount = 0;
        int componentByteSize = -1; // size of the component stored in this registry, assigned when the first component is added
        std::vector<std::byte> componentsBytes = {}; // contains a sequence of {entityReferenceID, component} pairs, e.g. {id,data,id,data,...,id,data}
    };
}

#endif //ACAENGINE_COMPONENTREGISTRY_H
