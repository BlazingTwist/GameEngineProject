#ifndef ACAENGINE_ENTITYREGISTRY_H
#define ACAENGINE_ENTITYREGISTRY_H

#include <vector>
#include <optional>
#include <utility>
#include <typeindex>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include "entityreference.h"
#include "componentregistry.h"
#include "entity.h"
#include <engine/utils/metaproghelpers.hpp>

namespace entity {
    class EntityRegistry {
    public:
        typedef std::pair<EntityReference *, Entity> EntityDataPair;

        static EntityRegistry &getInstance() {
            static EntityRegistry instance;
            return instance;
        }

        EntityRegistry(EntityRegistry const &) = delete;

        void operator=(EntityRegistry const &) = delete;

    private:
        template<typename T, typename ...T_Args>
        void prepareComponents(int entityRefID, std::unordered_map<std::type_index, int> &componentMap, T const &firstComponent, T_Args const &...otherComps) {
            auto typeIndex = std::type_index(typeid(T));
            ComponentRegistry *componentRegistry = ComponentRegistry::getInstance(typeIndex);
            int componentReference = componentRegistry->addComponent(entityRefID, firstComponent);
            componentMap[typeIndex] = componentReference;
            if constexpr(sizeof ...(T_Args) > 0) {
                prepareComponents<T_Args...>(entityRefID, componentMap, otherComps...);
            }
        }

    public:
        /**
         * Creates and registers a new Entity with the specified Components.
         * @tparam T_Components varArg of ComponentTypes to add to the Entity
         * @param args varArg of Component-Data to add to the Entity
         * @return new EntityReference, Caller needs to delete the pointer when done working with it 
         */
        template<typename... T_Components>
        EntityReference *createEntity(T_Components...args) {
            auto *ref = new EntityReference((int) entities.size());
            Entity entityData;
            if constexpr(sizeof ...(T_Components) > 0) {
                prepareComponents(ref->getReferenceID(), entityData.componentMap, args...);
            }

            entities.emplace_back(ref, entityData);
            return ref;
        }

        /**
         * Deletes an existing Entity. Does nothing, if the Entity does not exist.
         * @param reference Reference to the Entity to be removed.
         */
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
            entities[refID].second.componentMap.clear();

            // update entity list
            if (refID == entities.size() - 1) {
                entities.pop_back();
            } else { // move last entry into open slot
                EntityDataPair lastDataPair = entities.back();
                const auto refIterator = entities.begin() + refID;
                lastDataPair.first->updateReferenceID(refID);
                for (const auto &idxToComponentEntry: lastDataPair.second.componentMap) {
                    ComponentRegistry *registry = ComponentRegistry::getInstance(idxToComponentEntry.first);
                    registry->setEntityID(idxToComponentEntry.second, refID);
                }
                *refIterator = lastDataPair;
                entities.pop_back();
            }
        }

        /**
         * Retrieve stored data for an Entity.
         * @param reference Reference of the Entity to look up
         * @return The Entity-Data or an empty optional
         */
        [[nodiscard]] std::optional<const Entity> getEntityData(const EntityReference *reference) const {
            if (reference->isExpired()) {
                return std::nullopt;
            }
            return {entities[reference->getReferenceID()].second};
        }

        /**
         * Add a new component to an existing Entity.
         * Update the component, if the entity already has a component of this type.
         * @tparam T_component Type of the component to add
         * @param reference Reference to the Entity to modify
         * @param component Component-data to set
         */
        template<typename T_component>
        void addOrSetComponent(const EntityReference *reference, const T_component &component) {
            if (reference->isExpired()) {
                return;
            }
            auto typeIndex = std::type_index(typeid(T_component));
            _addOrSetComponent(ComponentRegistry::getInstance(typeIndex), typeIndex, reference, component);
        }

        template<typename T_component>
        void addOrSetComponent(ComponentRegistry *componentRegistry, const EntityReference *reference, const T_component &component) {
            if (reference->isExpired()) {
                return;
            }
            auto typeIndex = std::type_index(typeid(T_component));
            _addOrSetComponent(componentRegistry, typeIndex, reference, component);
        }

        template<typename T_component>
        void addOrSetComponent(ComponentRegistry *componentRegistry, const std::type_index &typeIndex,
                               const EntityReference *reference, const T_component &component) {
            if (reference->isExpired()) {
                return;
            }
            _addOrSetComponent(componentRegistry, typeIndex, reference, component);
        }

    private:
        template<typename T_component>
        void _addOrSetComponent(ComponentRegistry *componentRegistry, const std::type_index &typeIndex,
                                const EntityReference *reference, const T_component &component) {
            int entityRefID = reference->getReferenceID();
            auto &componentMap = entities[entityRefID].second.componentMap;
            auto findResult = componentMap.find(typeIndex);
            if (findResult == componentMap.end()) {
                int componentReference = componentRegistry->template addComponent<T_component>(entityRefID, component);
                componentMap[typeIndex] = componentReference;
            } else {
                componentRegistry->template setComponentData<T_component>(findResult->second, component);
            }
        }

    public:

        /**
         * Remove a component from an existing entity.
         * Does nothing, if entity/component does not exist.
         * @tparam T_component Type of the Component to remove
         * @param reference Reference to the Entity to modify
         */
        template<typename T_component>
        void removeComponent(const EntityReference *reference) {
            if (reference->isExpired()) {
                return;
            }
            removeComponent(reference->getReferenceID(), std::type_index(typeid(T_component)));
        }

        /**
         * Retrieve the Component-Data of an Entity.
         * Returns an empty optional, if the Entity does not contain the component.
         * @tparam T_component Type of the Component to get
         * @param reference Reference to the Entity to query
         * @return Component-Data or empty optional
         */
        template<typename T_component>
        [[nodiscard]] std::optional<T_component> getComponentData(const EntityReference *reference) const {
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
            componentMap.erase(componentTypeID);
        }

        void removeComponent(std::type_index componentTypeID, int componentID) {
            int movedEntityID = -1;
            int movedComponentID = -1;
            ComponentRegistry::getInstance(componentTypeID)->removeComponent(componentID, movedEntityID, movedComponentID);
            if (movedEntityID >= 0 && movedComponentID >= 0) {
                updateComponentID(movedEntityID, componentTypeID, movedComponentID);
            }
        }

    public:
        /**
         * Execute an Action on all entities having the components expected by Action::operator(TComponent ...).
         * In addition, the EntityReference is provided, if the first parameter is of type EntityReference. (const EntityReference *)
         * @tparam Action deducted Functor type
         * @param action Functor to call once for every matching Entity
         */
        template<typename Action>
        void execute(const Action &action) {
            _execute(action, &Action::operator());
        }

        template<typename ...Args>
        void execute(void(*action)(Args...)) {
            _execute<Args...>(action);
        }

    private:
        // gathers argument types of Action and forwards them to _execute2
        template<typename Action, typename Functor, typename ...Args>
        void _execute(Action &&action, void(Functor::*)(Args...) const) {
            _execute2<Action, Args...>(std::forward<Action>(action));
        }

        // gathers argument types of Action and forwards them to _execute2
        template<typename Action, typename Functor, typename ...Args>
        void _execute(Action &&action, void(Functor::*)(Args...)) {
            _execute2<Action, Args...>(std::forward<Action>(action));
        }
        
        template<typename ...Args, typename Action>
        void _execute(Action &&action){
            _execute2<Action, Args...>(std::forward<Action>(action));
        }

        template<typename Action, typename Arg1, typename ...Args>
        void _execute2(Action &&action) {
            constexpr bool ProvideEntity = std::is_same<Arg1, const entity::EntityReference *>::value;
            constexpr std::size_t ComponentCount = ([]() { if constexpr(ProvideEntity) { return sizeof...(Args); } else { return sizeof ...(Args) + 1; }})();

            std::vector<std::type_index> typeIndices = {};
            if constexpr(ProvideEntity) {
                utils::gatherTypeIDs<Args...>(typeIndices);
            } else {
                utils::gatherTypeIDs<Arg1, Args...>(typeIndices);
            }

            ASSERT(!typeIndices.empty(), "specified Action takes no parameters!");
            ASSERT(typeIndices.size() == ComponentCount, "failed to deduce Component-Types for Action!");

            // TODO find component with lowest proportion -> only consider entities contained in that component-Registry

            for (const auto &entityDataPair: entities) {
                if (!entityDataPair.second.containsAllComponents(typeIndices)) {
                    continue;
                }
                if constexpr(ProvideEntity) {
                    _executeWithEntity<Args...>(std::forward<Action>(action), typeIndices, entityDataPair, std::make_index_sequence<ComponentCount>{});
                } else {
                    _executeComponentsOnly<Arg1, Args...>(std::forward<Action>(action), typeIndices, entityDataPair,
                                                          std::make_index_sequence<ComponentCount>{});
                }
            }
        }

        template<typename ...TComponents, typename Action, std::size_t ...Idx>
        static void _executeComponentsOnly(const Action &action, const std::vector<std::type_index> &typeIndices,
                                           const EntityDataPair &entityDataPair, std::index_sequence<Idx...>) {
            std::vector<ComponentRegistry *> registries = {ComponentRegistry::getInstance(typeIndices[Idx])...};
            action(registries[Idx]->template getComponentData<TComponents>(entityDataPair.second.componentMap.at(typeIndices[Idx]))...);
        }

        template<typename ...TComponents, typename Action, std::size_t ...Idx>
        static void _executeWithEntity(const Action &action, const std::vector<std::type_index> &typeIndices,
                                       const EntityDataPair &entityDataPair, std::index_sequence<Idx...>) {
            std::vector<ComponentRegistry *> registries = {ComponentRegistry::getInstance(typeIndices[Idx])...};
            action(entityDataPair.first, (registries[Idx]->template getComponentData<TComponents>(entityDataPair.second.componentMap.at(typeIndices[Idx])))...);
        }

    private:

        EntityRegistry() = default;

        std::vector<EntityDataPair> entities = {};

    };
}

#endif //ACAENGINE_ENTITYREGISTRY_H
