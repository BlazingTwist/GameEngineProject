#ifndef ACAENGINE_ENTITYREGISTRY_v2_H
#define ACAENGINE_ENTITYREGISTRY_v2_H

#include <vector>
#include <unordered_map>
#include <typeindex>
#include "ComponentRegistry.h"
#include "EntityReference.h"

namespace entityV2 {
    class EntityRegistry {

    private:
        EntityRegistry() = default;

    public:
        std::vector<EntityReference *> &getEntities() {
            return entities;
        }

        static EntityRegistry &getInstance() {
            static EntityRegistry instance;
            return instance;
        }

        EntityRegistry(EntityRegistry const &) = delete;

        void operator=(EntityRegistry const &) = delete;

    private:
        template<typename T, typename ...T_Args>
        void prepareComponents(int entityRefID, EntityReference *entity, T const *firstComponent, T_Args const *...otherComps) {
            auto typeIndex = std::type_index(typeid(T));
            ComponentRegistry *componentRegistry = ComponentRegistry::getInstance(typeIndex);
            ComponentReference *componentReference = componentRegistry->addComponent(entityRefID, firstComponent);
            entity->componentMap[typeIndex] = componentReference;
            if constexpr(sizeof ...(T_Args) > 0) {
                prepareComponents<T_Args...>(entityRefID, entity, otherComps...);
            }
        }

        template<typename T, typename ...T_Args>
        void deleteComponentPointers(EntityReference *entity) {
            auto typeIndex = std::type_index(typeid(T));
            auto findResult = entity->componentMap.find(typeIndex);
            if (findResult != entity->componentMap.end()) {
                ComponentRegistry *componentRegistry = ComponentRegistry::getInstance(typeIndex);
                componentRegistry->removeComponent(findResult->second);
                auto *componentPtr = static_cast<T *>(findResult->second->_componentPtr);
                delete componentPtr;
                entity->componentMap.erase(findResult);
            }
            if constexpr(sizeof ...(T_Args) > 0) {
                deleteComponentPointers<T_Args...>(entity);
            }
        }

    public:
        template<typename... T_Components>
        EntityReference *createEntity(T_Components *...args) {
            auto *ref = new EntityReference(entities.size());
            if constexpr(sizeof ...(T_Components) > 0) {
                prepareComponents(ref->_referenceID, ref, args...);
            }
            entities.push_back(ref);
            return ref;
        }

        template<typename... T_Components>
        void eraseEntity(EntityReference *reference) {
            if (reference->isExpired()) {
                return;
            }

            const unsigned int referenceID = reference->_referenceID;

            if constexpr(sizeof ...(T_Components) > 0) {
                deleteComponentPointers<T_Components...>(reference);
            }

            // update entity list
            if (referenceID == entities.size() - 1) {
                entities.pop_back();
            } else { // move last entity into the now open slot
                EntityReference *movedEntity = entities.back();
                const auto entityIterator = entities.begin() + (int) referenceID;
                movedEntity->_referenceID = referenceID;
                for (const auto &componentPair: movedEntity->componentMap) {
                    componentPair.second->_entityID = referenceID;
                }
                *entityIterator = movedEntity;
                entities.pop_back();
            }
            reference->_isExpired = true;
            ASSERT(reference->componentMap.empty(), "eraseEntity call did not specify all Component types of entity! Unable to free memory.");
        }

        /**
         * Add a new component to an existing Entity.
         * Update the component, if the entity already has a component of this type.
         * @tparam T_component Type of the component to add
         * @param reference Reference to the Entity to modify
         * @param component Component-data to set
         */
        template<typename T_component>
        ComponentReference *addOrSetComponent(EntityReference *entity, T_component *component) {
            if (entity->isExpired()) {
                return nullptr;
            }
            auto typeIndex = std::type_index(typeid(T_component));
            return _addOrSetComponent(ComponentRegistry::getInstance(typeIndex), typeIndex, entity, component);
        }

        template<typename T_component>
        ComponentReference *addOrSetComponent(ComponentRegistry *componentRegistry, EntityReference *entity, T_component *component) {
            if (entity->isExpired()) {
                return nullptr;
            }
            auto typeIndex = std::type_index(typeid(T_component));
            return _addOrSetComponent(componentRegistry, typeIndex, entity, component);
        }

        template<typename T_component>
        ComponentReference *addOrSetComponent(ComponentRegistry *componentRegistry, const std::type_index &typeIndex,
                                              EntityReference *entity, T_component *component) {
            if (entity->isExpired()) {
                return nullptr;
            }
            return _addOrSetComponent(componentRegistry, typeIndex, entity, component);
        }

    private:
        template<typename T_component>
        ComponentReference *_addOrSetComponent(ComponentRegistry *componentRegistry, const std::type_index &typeIndex,
                                               EntityReference *entity, T_component *component) {
            auto &componentMap = entity->componentMap;
            auto findResult = componentMap.find(typeIndex);
            if (findResult == componentMap.end()) {
                ComponentReference *componentRef = componentRegistry->addComponent(entity->_referenceID, component);
                componentMap[typeIndex] = componentRef;
                return componentRef;
            } else {
                auto *currentData = static_cast<T_component *>(findResult->second->_componentPtr);
                delete currentData;
                findResult->second->_componentPtr = component;
                return findResult->second;
            }
        }

    public:
        template<typename T_component>
        void removeComponent(EntityReference *entity) {
            if (entity->isExpired()) {
                return;
            }
            auto componentTypeID = std::type_index(typeid(T_component));
            auto findResult = entity->componentMap.find(componentTypeID);
            if (findResult == entity->componentMap.end()) {
                return;
            }
            ComponentRegistry::getInstance(componentTypeID)->removeComponent(findResult->second);
            auto *componentPtr = static_cast<T_component *>(findResult->second->_componentPtr);
            delete componentPtr;
            entity->componentMap.erase(componentTypeID);
        }

        /**
         * Execute an Action on all entities having the components expected by Action::operator(TComponent *...).
         * In addition, the EntityReference is provided, if the first parameter is of type EntityReference. (EntityReference *)
         * @tparam Action deducted Functor type
         * @param action Functor to call once for every matching Entity
         */
        template<typename Action>
        void execute(const Action &action) {
            _execute(action, &Action::operator());
        }

    private:
        // gathers argument types of Action and forwards them to _execute2
        template<typename Action, typename Functor, typename ...Args>
        void _execute(Action &&action, void(Functor::*)(Args *...) const) {
            _execute2<Action, Args...>(std::forward<Action>(action));
        }

        // gathers argument types of Action and forwards them to _execute2
        template<typename Action, typename Functor, typename ...Args>
        void _execute(Action &&action, void(Functor::*)(Args *...)) {
            _execute2<Action, Args...>(std::forward<Action>(action));
        }

        template<typename Action, typename Arg1, typename ...Args>
        void _execute2(Action &&action) {
            constexpr bool ProvideEntity = std::is_same<Arg1, EntityReference>::value;
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

            for (auto *entity: entities) {
                if (!entity->containsAllComponents(typeIndices)) {
                    continue;
                }
                if constexpr(ProvideEntity) {
                    _executeWithEntity<Args...>(std::forward<Action>(action), typeIndices, entity, std::make_index_sequence<ComponentCount>{});
                } else {
                    _executeComponentsOnly<Arg1, Args...>(std::forward<Action>(action), typeIndices, entity, std::make_index_sequence<ComponentCount>{});
                }
            }
        }

        template<typename ...TComponents, typename Action, std::size_t ...Idx>
        static void _executeComponentsOnly(const Action &action, const std::vector<std::type_index> &typeIndices,
                                           EntityReference *entity, std::index_sequence<Idx...>) {
            action(entity->componentMap[typeIndices[Idx]]->template getComponent<TComponents>()...);
        }

        template<typename ...TComponents, typename Action, std::size_t ...Idx>
        static void _executeWithEntity(const Action &action, const std::vector<std::type_index> &typeIndices,
                                       EntityReference *entity, std::index_sequence<Idx...>) {
            action(entity, entity->componentMap[typeIndices[Idx]]->template getComponent<TComponents>()...);
        }

    private:
        std::vector<EntityReference *> entities = {};

    };
}

#endif //ACAENGINE_ENTITYREGISTRY_v2_H
