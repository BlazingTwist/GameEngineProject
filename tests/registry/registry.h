#pragma once

#include <engine/utils/containers/slotmap.hpp>
#include <engine/utils/metaproghelpers.hpp>
#include <engine/utils/assert.hpp>
#include <tuple>
#include <utility>
#include <type_traits>
#include <optional>
#include <vector>
#include <concepts>
#include <limits>
#include <cstdint>
#include <vector>
#include <concepts>
#include <type_traits>


namespace game {
    struct Entity {
        using BaseType = uint32_t;
        constexpr static BaseType INVALID_ID = std::numeric_limits<Entity::BaseType>::max();

        constexpr explicit Entity(BaseType id) noexcept: m_id(id) {}

        constexpr Entity() noexcept: m_id(INVALID_ID) {}

        /* explicit */
        operator BaseType() const { return m_id; }

        BaseType toIndex() const { return m_id; }

        constexpr operator bool() const { return m_id != INVALID_ID; }

        constexpr Entity operator=(Entity oth) noexcept {
            m_id = oth.m_id;
            return *this;
        }

        constexpr bool operator==(Entity oth) const { return m_id == oth.m_id; }

        constexpr bool operator!=(Entity oth) const { return m_id != oth.m_id; }

    private:
        BaseType m_id;
    };

    constexpr static Entity INVALID_ENTITY(Entity::INVALID_ID);

    // Basically a weak pointer to an Entity.
    struct EntityRef {
        EntityRef() noexcept: entity(), generation(0) {}

    private:
        EntityRef(Entity _ent, unsigned _generation)
                : entity(_ent), generation(_generation) {}

        friend class Registry2;

        Entity entity;
        unsigned generation;
    };

    namespace components {
        // Currently only works correctly together with Position, Rotation, Scale components
        struct Parent {
            Parent(Entity ent) : entity(ent) {}

            Entity entity;
        };

        struct Children {
            std::vector<Entity> entities;
        };
    }

    // Inherit from this to allow multiple components of this type attached to the same entity.
    // depreciated! only used by Registry and still buggy
    class MultiComponent {
    };

    // Inherit from this to create a special component with the following properties:
    // * an entity can possess multiple messages of the same type
    // * no entity wise access, use iterate and clear instead
    class Message {
    };

    // temporary
    class Flag {
    };

    // Requirements that any component type needs to fulfill.
    template<class T>
    concept component_type = std::movable<T>;

    // Exclusive concepts that determine the storage type and access patterns.
    template<class T>
    concept message_component_type = component_type<T> && std::is_base_of_v<Message, T>;

    template<class T>
    concept flag_component_type = component_type<T> && std::is_empty_v<T>;

    template<class T>
    concept data_component_type = component_type<T>
                                  && !message_component_type<T>
                                  && !flag_component_type<T>;

    template<component_type... Components>
    class Registry {
        template<typename Val, bool MultiSlot>
        class SlotMapDecider {
        };

        template<typename Val>
        class SlotMapDecider<Val, false> : public utils::SlotMap<Entity::BaseType, Val> {
        };

        template<typename Val>
        class SlotMapDecider<Val, true> : public utils::MultiSlotMap<Entity::BaseType, Val> {
        };

        template<typename Val>
        using SM = SlotMapDecider<Val, std::is_base_of_v<MultiComponent, Val>>;
    public:
        // Make a new entity managed by this registry.
        Entity create() {
            Entity ent;
            if (!m_unusedEntities.empty()) {
                ent = m_unusedEntities.back();
                m_unusedEntities.pop_back();

                m_generations[ent.toIndex()].entity = ent;
                m_generations[ent.toIndex()].generation++;
            } else {
                ent = Entity(m_maxNumEntities++);
                m_generations.push_back({ent, 0});
            }
            return ent;
        }

        // Remove an entity with all its components.
        void erase(Entity _ent) {
            ASSERT(m_generations[_ent.toIndex()].entity != INVALID_ENTITY, "Attempting to erase a non existent entity.");

            (removeComponent<Components>(_ent), ...);

            // mark invalid before removing components so that Children removal is simpler
            m_generations[_ent.toIndex()].entity = INVALID_ENTITY;
            removeComponent<components::Parent>(_ent);
            removeComponent<components::Children>(_ent);
            m_unusedEntities.push_back(_ent);
        }

        // Add a new component to an existing entity. No changes are done if Component is
        // not a MultiComponent and _ent already has a component of this type.
        // @return A reference to the new component or the already existing component.
        template<component_type Component, typename... Args>
        requires (!std::same_as<Component, components::Parent>)
        Component &addComponent(Entity _ent, Args &&... _args) {
            return getContainer<Component>().emplace(_ent.toIndex(), std::forward<Args>(_args)...);
        }

        template<component_type Component>
        requires std::same_as<Component, components::Parent>
        Component &addComponent(Entity _ent, components::Parent _parent) {
            auto &childs = addComponent<components::Children>(_parent.entity);
            childs.entities.push_back(_ent);
            return getContainer<components::Parent>().emplace(_ent.toIndex(), _parent.entity);
        }

        template<component_type Component>
        requires (!std::same_as<Component, components::Parent>) && (!std::same_as<Component, components::Children>)
        void removeComponent(Entity _ent) {
            auto &comps = std::get<SM<Component>>(m_components);
            if (comps.contains(_ent.toIndex()))
                comps.erase(_ent.toIndex());
        }

        template<component_type Component>
        requires std::same_as<Component, components::Parent>
        void removeComponent(Entity _ent) {
            auto &comps = std::get<SM<Component>>(m_components);
            if (comps.contains(_ent.toIndex())) {
                // remove from parent list
                auto &parent = comps[_ent.toIndex()];
                if (isValid(parent.entity)) {
                    auto &childs = std::get<SM<components::Children>>(m_components)[parent.entity.toIndex()].entities;
                    auto it = std::find(childs.begin(), childs.end(), _ent);
                    ASSERT(it != childs.end(), "Entity not found in childs of parent.");
                    *it = childs.back();
                    childs.pop_back();
                }
                comps.erase(_ent.toIndex());
            }
        }

        template<component_type Component>
        requires std::same_as<Component, components::Children>
        void removeComponent(Entity _ent) {
            auto &comps = std::get<SM<Component>>(m_components);
            if (comps.contains(_ent.toIndex())) {
                auto &childs = comps[_ent.toIndex()].entities;
                // erase all children
                for (Entity ent: childs)
                    erase(ent);
                comps.erase(_ent.toIndex());
            }
        }

        // Remove all components of the specified type.
        template<component_type Component>
        void clearComponent() {
            getContainer<Component>().clear();
        }

        template<component_type Component>
        bool hasComponent(Entity _ent) const { return getContainer<Component>().contains(_ent.toIndex()); }

        // Retrieve a component associated with an entity.
        // @return nullptr if the entity has no component of this type
        /*	template<component_type Component>
            Component* getComponentOpt(Entity _ent) 
            {
                // todo: only do one access
                return getContainer<Component>().contains(_ent.toIndex()) ? &getContainer<Component>()[_ent.toIndex()]; : nullptr;
            }
            template<component_type Component>
            const Component* getComponentOpt(Entity _ent) const
            {
                return getContainer<Component>().contains(_ent.toIndex()) ? &getContainer<Component>()[_ent.toIndex()]; : nullptr;
            }*/

        // Retrieve a component associated with an entity.
        // Does not check whether it exits.
        template<component_type Component>
        Component &getComponent(Entity _ent) { return getContainer<Component>()[_ent.toIndex()]; }

        template<component_type Component>
        const Component &getComponent(Entity _ent) const { return getContainer<Component>()[_ent.toIndex()]; }

        template<component_type Comp>
        SM<Comp> &getContainer() { return std::get<SM<Comp>>(m_components); }

        template<component_type Comp>
        const SM<Comp> &getContainer() const { return std::get<SM<Comp>>(m_components); }

        // Execute an Action on all entities having the components
        // expected by Action::operator(...).
        template<typename Action>
        void execute(Action &_action) { executeUnpack(_action, utils::UnpackFunction(&Action::operator())); }

        // This explicit version is only needed to capture rvalues.
        template<typename Action>
        void execute(const Action &_action) { executeUnpack(_action, utils::UnpackFunction(&Action::operator())); }

        template<typename ...T_Components, typename Action>
        void execute(Action &_action, bool) { execute<Action>(_action); }
        
        template<typename ...T_Components, typename Action>
        void execute(const Action &_action, bool) { execute<Action>(_action); }

        // Basically a weak pointer to an Entity.
        struct EntityRef {
            EntityRef() : entity(INVALID_ENTITY), generation(0) {}

        private:
            EntityRef(Entity _ent, unsigned _generation)
                    : entity(_ent), generation(_generation) {}

            friend class Registry;

            Entity entity;
            unsigned generation;
        };

        EntityRef getRef(Entity _ent) const { return m_generations[_ent.toIndex()]; }

        // Checks whether an entity is managed by this registry.
        // An Entity can be valid even if previously deleted, if the id was reassigned.
        // Use an EntityRef instead to prevent this possibility.
        bool isValid(Entity _ent) const { return _ent && m_generations[_ent.toIndex()].entity == _ent; }

        // Attempt to retrieve the referenced entity.
        // @return The entity or an INVALID_ENTITY if the ref is not valid.
        Entity getEntity(EntityRef _ent) const {
            const EntityRef &ref = m_generations[_ent.entity.toIndex()];
            if (ref.entity == _ent.entity && _ent.generation == ref.generation)
                return _ent.entity;
            else return INVALID_ENTITY;
        }

    private:
        template<typename Action, typename Comp, typename... Comps>
        void executeUnpack(Action &_action, utils::UnpackFunction<std::remove_cv_t<Action>, Comp, Comps...>) {
            if constexpr (std::is_convertible_v<Comp, Entity>)
                executeImpl<true, Action, std::decay_t<Comps>...>(_action);
            else
                executeImpl<false, Action, std::decay_t<Comp>, std::decay_t<Comps>...>(_action);
        }

        template<bool WithEnt, typename Action, component_type Comp, component_type... Comps>
        void executeImpl(Action &_action) {
            auto &mainContainer = std::get<SM<Comp>>(m_components);
            for (auto it = mainContainer.begin(); it != mainContainer.end(); ++it) {
                Entity ent(it.key());
                if ((std::get<SM<Comps>>(m_components).contains(ent.toIndex()) && ...)) {
                    if constexpr (WithEnt)
                        _action(ent, it.value(), std::get<SM<Comps>>(m_components)[ent.toIndex()] ...);
                    else
                        _action(it.value(), std::get<SM<Comps>>(m_components)[ent.toIndex()] ...);
                }
            }
        }

        std::vector<Entity> m_unusedEntities;
        uint32_t m_maxNumEntities = 0u;
        std::tuple<SM<components::Parent>, SM<components::Children>, SM<Components>...> m_components;
        std::vector<EntityRef> m_generations;
    };
}
