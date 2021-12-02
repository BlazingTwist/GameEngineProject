#ifndef ACAENGINE_ENTITYREGISTRY_H
#define ACAENGINE_ENTITYREGISTRY_H

#include <vector>
#include <optional>
#include <typeindex>
#include <spdlog/spdlog.h>
#include "entityreference.h"

namespace entity {

    // TODO figure out the requirements for Entity and Components and extract them to a separate file once that happened
    struct Entity {
        std::vector<std::type_index> components;
    };

    struct Component {};
    struct ExComp1 : public Component{};
    struct ExComp2 : public Component{};
    struct ExComp3 : public Component{};

    class EntityRegistry {

    private:
        template<typename T, typename ...T_Args>
        void gatherTypeIndices(std::vector<std::type_index> *vector) {
            vector->push_back(std::type_index(typeid(T)));
            if constexpr(sizeof ...(T_Args) > 0) {
                gatherTypeIndices<T_Args...>(vector);
            }
        }

    public:
        typedef std::pair<EntityReference *, Entity *> entityDataPair;

        static EntityRegistry &getInstance() {
            static EntityRegistry instance;
            return instance;
        }

        EntityRegistry(EntityRegistry const &) = delete;

        void operator=(EntityRegistry const &) = delete;

        // Caller needs to delete the pointer when done working with it
        template<typename... T_Args>
        EntityReference *createEntity() {
            std::vector<std::type_index> typeIndices = {};
            gatherTypeIndices<T_Args...>(&typeIndices);

            auto *ref = new EntityReference((int) entities.size());
            auto *entityData = new Entity();
            entityData->components = typeIndices;
            entities.emplace_back(ref, entityData);
            return ref;
        }

        void eraseEntity(EntityReference *reference) {
            if (reference->isExpired()) {
                return;
            }

            reference->setExpired();
            const int refID = reference->getReferenceID();
            if (refID == entities.size() - 1) {
                entities.pop_back();
            } else { // move last entry into open slot
                entityDataPair lastDataPair = entities.back();
                const auto refIterator = entities.begin() + refID;
                *refIterator = lastDataPair;
                lastDataPair.first->updateReferenceID(refID);
                entities.pop_back();
            }
        }

        [[nodiscard]] std::optional<Entity *> getEntityData(const EntityReference *reference) const {
            if (reference->isExpired()) {
                return std::nullopt;
            }
            return {entities[reference->getReferenceID()].second};
        }

        void test(const EntityReference *reference) const {
            std::optional<Entity *> data = getEntityData(reference);
            spdlog::info("refID: {} | isExpired: {}", reference->getReferenceID(), reference->isExpired());
        }

        void test2() const {
            spdlog::info("EntityRegistry entities:");
            for (const auto &entity: entities) {
                spdlog::info("  refID = {} | expired = {}", entity.first->getReferenceID(), entity.first->isExpired());
            }
            spdlog::info("-----");
        }

    private:
        EntityRegistry() = default;

        std::vector<entityDataPair> entities = {};

    };

}

#endif //ACAENGINE_ENTITYREGISTRY_H
