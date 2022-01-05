#ifndef ACAENGINE_ENTITYREGISTRY_H
#define ACAENGINE_ENTITYREGISTRY_H

#include <vector>
#include <optional>
#include <typeindex>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include "entityreference.h"
#include "componentregistry.h"
#include "entity.h"

namespace entity {
    class EntityRegistry {
    public:
        typedef std::pair<EntityReference *, Entity> entityDataPair;

        static EntityRegistry &getInstance() {
            static EntityRegistry instance;
            return instance;
        }

        EntityRegistry(EntityRegistry const &) = delete;

        void operator=(EntityRegistry const &) = delete;

    private:
        template<typename T, typename ...T_Args>
        void prepareComponents(const int &entityRefID,
                               std::unordered_map<std::type_index, int> &componentMap, T const &firstComponent, T_Args const &...otherComps) {
            auto typeIndex = std::type_index(typeid(T));
            ComponentRegistry *componentRegistry = ComponentRegistry::getInstance(typeIndex);
            int componentReference = componentRegistry->addComponent(entityRefID, firstComponent);
            componentMap[typeIndex] = componentReference;
            if constexpr(sizeof ...(T_Args) > 0) {
                prepareComponents<T_Args...>(entityRefID, componentMap, otherComps...);
            }
        }

    public:
        // Caller needs to delete the pointer when done working with it
        template<typename... T_Components>
        EntityReference *createEntity(T_Components...args) {
            auto *ref = new EntityReference((int) entities.size());
            Entity entityData;
            prepareComponents(ref->getReferenceID(), entityData.componentMap, args...);

            entities.emplace_back(ref, entityData);
            return ref;
        }

        void eraseEntity(EntityReference *reference) {
            if (reference->isExpired()) {
                return;
            }

            reference->setExpired();
            const int refID = reference->getReferenceID();

            // remove components
            for (const auto &typeID_componentID_Pair: entities[refID].second.componentMap) {
                removeComponent(typeID_componentID_Pair.first, typeID_componentID_Pair.second);
            }

            // update entity list
            if (refID == entities.size() - 1) {
                entities.pop_back();
            } else { // move last entry into open slot
                entityDataPair lastDataPair = entities.back();
                const auto refIterator = entities.begin() + refID;
                lastDataPair.first->updateReferenceID(refID);
                *refIterator = lastDataPair;
                entities.pop_back();
            }
        }

        [[nodiscard]] std::optional<const Entity> getEntityData(const EntityReference *reference) const {
            if (reference->isExpired()) {
                return std::nullopt;
            }
            return {entities[reference->getReferenceID()].second};
        }

        template<typename T_component>
        void addOrSetComponent(const EntityReference *reference, T_component component) {
            if (reference->isExpired()) {
                return;
            }
            auto typeIndex = std::type_index(typeid(T_component));
            int entityRefID = reference->getReferenceID();
            auto &componentMap = entities[entityRefID].second.componentMap;
            if (componentMap.find(typeIndex) == componentMap.end()) {
                int componentReference = ComponentRegistry::getInstance(typeIndex)->template addComponent(entityRefID, component);
                componentMap[typeIndex] = componentReference;
            } else {
                int componentID = componentMap[typeIndex];
                ComponentRegistry::getInstance(typeIndex)->template setComponentData(componentID, component);
            }
        }

        template<typename T_component>
        void removeComponent(const EntityReference *reference) {
            if (reference->isExpired()) {
                return;
            }
            removeComponent(reference->getReferenceID(), std::type_index(typeid(T_component)));
        }

        template<typename T_component>
        [[nodiscard]] std::optional<const T_component> getComponentData(const EntityReference *reference) const {
            if (reference->isExpired()) {
                return std::nullopt;
            }
            auto typeIndex = std::type_index(typeid(T_component));
            auto &componentMap = entities[reference->getReferenceID()].second.componentMap;
            if (componentMap.find(typeIndex) == componentMap.end()) {
                return std::nullopt;
            } else {
                return ComponentRegistry::getInstance(typeIndex)->template getComponentData<T_component>(componentMap.at(typeIndex));
            }
        }
        
        // Note: "Funktor"
        template<typename Action>
        void execute(const Action &_action) {
            _action();
        }

    private:
        void updateComponentID(int entityRefID, std::type_index compTypeIndex, int componentID) {
            entities[entityRefID].second.componentMap[compTypeIndex] = componentID;
        }

        void removeComponent(int entityID, std::type_index componentTypeID) {
            auto &componentMap = entities[entityID].second.componentMap;
            if (componentMap.find(componentTypeID) == componentMap.end()) {
                return;
            }
            removeComponent(componentTypeID, componentMap[componentTypeID]);
        }

        void removeComponent(std::type_index componentTypeID, int componentID) {
            int movedEntityID = -1;
            int movedComponentID = -1;
            ComponentRegistry::getInstance(componentTypeID)->removeComponent(componentID, movedEntityID, movedComponentID);
            if (movedEntityID >= 0 && movedComponentID >= 0) {
                updateComponentID(movedEntityID, componentTypeID, movedComponentID);
            }
        }

        EntityRegistry() = default;

        std::vector<entityDataPair> entities = {};

    };

}

#endif //ACAENGINE_ENTITYREGISTRY_H
