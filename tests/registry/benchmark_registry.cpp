#include "registry.h"
#include "smkregistry.h"
#include <engine/entity/entityregistry.h>
#include <engine/entityV2/EntityRegistry.h>

#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <spdlog/fmt/fmt.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <fstream>
#include <filesystem>
#include <iostream>
#include <random>
#include <chrono>
#include <array>

// CRT's memory leak detection
#ifndef NDEBUG
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>

#endif
#endif

using namespace game;
using namespace components;
using namespace glm;

namespace chrono = std::chrono;

namespace tests {

    using game::Entity;
    using game::EntityRef;

    template<typename Component>
    class StaticAccess {
    public:
        explicit StaticAccess(utils::SlotMap<Entity::BaseType, Component> &_storage) noexcept
                : m_targetStorage(_storage) {}

        // Retrieve a component of _ent.
        // @return A pointer to the associated component or nullptr if it does not exist.
        Component *at(Entity _ent) { return m_targetStorage.contains(_ent) ? &m_targetStorage[_ent] : nullptr; }

        const Component *at(Entity _ent) const { return m_targetStorage.contains(_ent) ? &m_targetStorage[_ent] : nullptr; }

        template<typename... Args>
        Component &insert(Entity _ent, Args &&... _args) {
            return m_targetStorage.emplace(_ent.toIndex(), std::forward<Args>(_args)...);
        }

        void erase(Entity _ent) { m_targetStorage.erase(_ent.toIndex()); }

    private:
        utils::SlotMap<Entity::BaseType, Component> &m_targetStorage;
    };


    template<typename BaseRegistry, template<class> class CompAccess>
    class RegistryWrapper {
    public:
        Entity create() { return m_registry.create(); }

        void erase(Entity _ent) { return m_registry.erase(_ent); }

        EntityRef getRef(Entity _ent) const { return m_registry.getRef(_ent); }

        std::optional<Entity> getEntity(EntityRef _ent) const {
            Entity ent = m_registry.getEntity(_ent);
            if (ent != game::INVALID_ENTITY)
                return ent;
            return std::nullopt;
        }

        // Retrieve the component container for the specified type.
        template<typename Component>
        CompAccess<Component> getComponents() { return CompAccess<Component>(m_registry.getContainer<Component>()); }

        template<typename Component>
        const CompAccess<Component> getComponents() const { return CompAccess<Component>(m_registry.getContainer<Component>()); }

        template<typename ...T_Components, typename Action>
        void execute(const Action &_action) {
            m_registry.template execute<T_Components...>(_action, false);
        }

    private:
        BaseRegistry m_registry;
    };
}

namespace game {
    namespace components {
        using Color = glm::vec4;
        using Alignment = glm::vec2;

        struct TestComponent {
            TestComponent() {}

            TestComponent(const std::string &_text,
                          float _fontSize = 10.f,
                          const glm::vec3 &_pos = glm::vec3(0.f, 0.f, -0.5f),
                          const Color &_color = Color(1.f),
                          const Alignment &_alignment = Alignment(0.f, 0.f),
                          float _rotation = 0.f,
                          bool _roundToPixel = false)
                    : size(_text.length()), fontSize(_fontSize), position(_pos), color(_color),
                      rotation(_rotation), alignment(_alignment),
                      roundToPixel(_roundToPixel) {}

            size_t size;
            float fontSize;
            glm::vec3 position;
            Color color;
            float rotation;
            Alignment alignment;
            bool roundToPixel;
        };

        struct Position {
            Position() {}

            Position(const glm::vec3 &v) : value{v} {}

            glm::vec3 value;
        };

        struct Velocity {
            Velocity() {}

            Velocity(const glm::vec3 &v) : value{v} {}

            glm::vec3 value;
        };

        struct PositionAlt {
            PositionAlt(const glm::vec3 &v) : value{v} {}

            glm::vec3 value;
        };

        struct VelocityAlt {
            VelocityAlt(const glm::vec3 &v) : value{v} {}

            glm::vec3 value;
        };

        struct Position2D {
            Position2D(const glm::vec2 v) : value{v} {}

            glm::vec2 value;
        };

        struct Rotation2D {
            Rotation2D(float v) : value{v} {}

            float value;
        };

        struct Transform {
            Transform() {}

            Transform(const glm::mat4 &m) : value{m} {}

            glm::mat4 value;
        };
    }
}

class TestOperation {
public:
    void operator()(components::TestComponent &_label,
                    const components::Transform &_transform,
                    const components::Position &_position,
                    const components::Velocity &_velocity) const {
        const glm::vec4 v = _transform.value * glm::vec4(_position.value, 1.f);
        _label.size = (std::to_string(v.x) + ", " + std::to_string(v.y) + std::to_string(_velocity.value.z)).size();
    }
};

class ApplyVelocity {
public:
    ApplyVelocity(float _deltaTime) : m_deltaTime(_deltaTime) {}

    void operator()(const components::Velocity &_velocity, components::Position &_position) const {
        _position.value += m_deltaTime * _velocity.value;
    }

private:
    float m_deltaTime;
};

using Results = std::unordered_map<std::string, float>;

using Results = std::unordered_map<std::string, float>;

template<typename... Comps, typename Registry, typename Action>
void execute(Registry &registry, const Action &action) {
    registry.template execute<Comps...>(action);
}

template<typename Registry>
Results benchmarkRegistry(int numEntities, int numRuns) {
    namespace comps = components;
    auto run = [&](Results &results) {
        Registry registry;

        auto ent = registry.create();
        std::vector<decltype(ent)> entities;
        entities.reserve(numEntities);

        for (int i = 0; i < numEntities; ++i)
            entities.push_back(registry.create());

        // prefetch all containers once so that the Access iterators remain valid
        registry.template getComponents<comps::Position2D>();
        registry.template getComponents<comps::Rotation2D>();
        registry.template getComponents<comps::Position>();
        registry.template getComponents<comps::Velocity>();
        registry.template getComponents<comps::PositionAlt>();
        registry.template getComponents<comps::VelocityAlt>();
        registry.template getComponents<comps::TestComponent>();
        registry.template getComponents<comps::Transform>();

        auto pos2Comps = registry.template getComponents<comps::Position2D>();
        auto rotComps = registry.template getComponents<comps::Rotation2D>();

        pos2Comps.insert(entities.front(), glm::vec2(0.2f));
        rotComps.insert(entities.front(), 42.f);

        auto posComps = registry.template getComponents<comps::Position>();
        auto velComps = registry.template getComponents<comps::Velocity>();

        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numEntities; ++i) {
            posComps.insert(entities[i], glm::vec3(static_cast<float>(i)));
            velComps.insert(entities[i], glm::vec3(1.f, 0.f, static_cast<float>(i)));
        }
        auto end = chrono::high_resolution_clock::now();
        results["insert"] += chrono::duration<float>(end - start).count();

        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numEntities; ++i) {
            auto posAltComps = registry.template getComponents<comps::PositionAlt>();
            auto velAltComps = registry.template getComponents<comps::VelocityAlt>();
            posAltComps.insert(entities[i], glm::vec3(static_cast<float>(i)));
            velAltComps.insert(entities[i], glm::vec3(1.f, 0.f, static_cast<float>(i)));
        }
        end = chrono::high_resolution_clock::now();
        results["insert_con"] += chrono::duration<float>(end - start).count();

        std::default_random_engine rng(13567u);
        std::shuffle(entities.begin(), entities.end(), rng);

        auto testComps = registry.template getComponents<comps::TestComponent>();
        auto transformComps = registry.template getComponents<comps::Transform>();

        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numEntities; i += 7)
            testComps.insert(entities[i], std::to_string(i) + "2poipnrpuipo");
        for (int i = 0; i < numEntities; i += 3)
            transformComps.insert(entities[i], glm::identity<glm::mat4>());
        for (int i = 0; i < numEntities; i += 6)
            rotComps.insert(entities[i], 1.f / i);
        for (int i = 0; i < numEntities; i += 11)
            pos2Comps.insert(entities[i], glm::vec2(2.f / i, i / 2.f));
        end = chrono::high_resolution_clock::now();
        results["insert_big"] += chrono::duration<float>(end - start).count();

        std::uniform_real_distribution<float> dt(0.01f, 0.5f);
        start = chrono::high_resolution_clock::now();
        execute<comps::Velocity, comps::Position>(registry, ApplyVelocity(dt(rng)));
        end = chrono::high_resolution_clock::now();
        results["simple_op"] += chrono::duration<float>(end - start).count();

        start = chrono::high_resolution_clock::now();
        execute<comps::TestComponent, comps::Transform, comps::Position, comps::Velocity>(registry, TestOperation());
        end = chrono::high_resolution_clock::now();
        results["complex_op"] += chrono::duration<float>(end - start).count();

        std::shuffle(entities.begin(), entities.end(), rng);

        auto posAltComps = registry.template getComponents<comps::PositionAlt>();
        auto velAltComps = registry.template getComponents<comps::VelocityAlt>();
        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numEntities; ++i) {
            posAltComps.erase(entities[i]);
            velAltComps.erase(entities[i]);
        }
        end = chrono::high_resolution_clock::now();
        results["erase comp"] += chrono::duration<float>(end - start).count();

        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numEntities; ++i)
            registry.erase(entities[i]);
        end = chrono::high_resolution_clock::now();
        results["remove ent"] += chrono::duration<float>(end - start).count();
    };

    Results tempResults;
    run(tempResults);
    run(tempResults);

    Results results;
    for (int j = 0; j < numRuns; ++j) {
        run(results);
    }

    for (auto&[_, f]: results)
        f /= numRuns;

    return results;
}

Results benchmarkCustomRegistry(int numEntities, int numRuns) {
    namespace comps = components;
    auto run = [&](Results &results) {
        auto &registry = entityV2::EntityRegistry::getInstance();

        entityV2::EntityReference *firstEntity = registry.createEntity();
        std::vector<entityV2::EntityReference *> entities = {};
        entities.reserve(numEntities);
        std::vector<entityV2::ComponentReference *> components = {};

        for (int i = 0; i < numEntities; ++i) {
            entities.push_back(registry.createEntity());
        }

        /*components.push_back(registry.addOrSetComponent<comps::Position2D>(entities.front(), new comps::Position2D(glm::vec2(0.2f))));
        components.push_back(registry.addOrSetComponent<comps::Rotation2D>(entities.front(), new comps::Rotation2D(42.f)));*/

        auto start = chrono::high_resolution_clock::now();
        auto posType = std::type_index(typeid(comps::Position));
        auto velocityType = std::type_index(typeid(comps::Velocity));
        auto *posRegistry = entityV2::ComponentRegistry::getInstance(posType);
        auto *velocityRegistry = entityV2::ComponentRegistry::getInstance(velocityType);
        for (int i = 0; i < numEntities; ++i) {
            components.push_back(
                    registry.addOrSetComponent<comps::Position>(posRegistry, posType, entities[i], new comps::Position(glm::vec3(static_cast<float>(i))))
            );
            components.push_back(
                    registry.addOrSetComponent<comps::Velocity>(velocityRegistry, velocityType, entities[i],
                                                                new comps::Velocity(glm::vec3(1.f, 0.f, static_cast<float>(i))))
            );
        }
        auto end = chrono::high_resolution_clock::now();
        results["insert"] += chrono::duration<float>(end - start).count();

        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numEntities; ++i) {
            components.push_back(
                    registry.addOrSetComponent<comps::PositionAlt>(entities[i], new comps::PositionAlt(glm::vec3(static_cast<float>(i))))
            );
            components.push_back(
                    registry.addOrSetComponent<comps::VelocityAlt>(entities[i], new comps::VelocityAlt(glm::vec3(1.f, 0.f, static_cast<float>(i))))
            );
        }
        end = chrono::high_resolution_clock::now();
        results["insert_con"] += chrono::duration<float>(end - start).count();

        std::default_random_engine rng(13567u);
        std::shuffle(entities.begin(), entities.end(), rng);

        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numEntities; i += 7)
            components.push_back(registry.addOrSetComponent<comps::TestComponent>(entities[i], new TestComponent(std::to_string(i) + "2poipnrpuipo")));
        for (int i = 0; i < numEntities; i += 3)
            components.push_back(registry.addOrSetComponent<comps::Transform>(entities[i], new comps::Transform(glm::identity<glm::mat4>())));
        for (int i = 0; i < numEntities; i += 6)
            components.push_back(registry.addOrSetComponent<comps::Rotation2D>(entities[i], new comps::Rotation2D(1.f / i)));
        for (int i = 0; i < numEntities; i += 11)
            components.push_back(registry.addOrSetComponent<comps::Position2D>(entities[i], new comps::Position2D(glm::vec2(2.f / i, i / 2.f))));
        end = chrono::high_resolution_clock::now();
        results["insert_big"] += chrono::duration<float>(end - start).count();

        class ApplyVelocity2 {
        public:
            ApplyVelocity2(float _deltaTime, entityV2::EntityRegistry &_registry) : m_deltaTime(_deltaTime), registry(_registry) {}

            void operator()(entityV2::EntityReference *entity, components::Velocity *_velocity, components::Position *_position) const {
                _position->value += m_deltaTime * _velocity->value;
            }

        private:
            float m_deltaTime;
            entityV2::EntityRegistry &registry;
        };

        std::uniform_real_distribution<float> dt(0.01f, 0.5f);
        start = chrono::high_resolution_clock::now();
        registry.execute(ApplyVelocity2(dt(rng), registry));
        end = chrono::high_resolution_clock::now();
        results["simple_op"] += chrono::duration<float>(end - start).count();

        class TestOperation2 {
        public:
            TestOperation2(entityV2::EntityRegistry &_registry) : registry(_registry) {}

            void operator()(components::TestComponent *_label, components::Transform *_transform,
                            components::Position *_position, components::Velocity *_velocity) const {
                const glm::vec4 v = _transform->value * glm::vec4(_position->value, 1.f);
                _label->size = (std::to_string(v.x) + ", " + std::to_string(v.y) + std::to_string(_velocity->value.z)).size();
            }

        private:
            entityV2::EntityRegistry &registry;
        };

        start = chrono::high_resolution_clock::now();
        registry.execute(TestOperation2(registry));
        end = chrono::high_resolution_clock::now();
        results["complex_op"] += chrono::duration<float>(end - start).count();

        std::shuffle(entities.begin(), entities.end(), rng);

        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numEntities; ++i) {
            registry.removeComponent<comps::PositionAlt>(entities[i]);
            registry.removeComponent<comps::VelocityAlt>(entities[i]);
        }
        end = chrono::high_resolution_clock::now();
        results["erase comp"] += chrono::duration<float>(end - start).count();

        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numEntities; ++i) {
            registry.eraseEntity<comps::Position2D, comps::Rotation2D, comps::Position, comps::Velocity, comps::PositionAlt, comps::VelocityAlt, comps::TestComponent, comps::Transform>(
                    entities[i]);
        }
        end = chrono::high_resolution_clock::now();
        results["remove ent"] += chrono::duration<float>(end - start).count();
        
        registry.eraseEntity<comps::Position2D, comps::Rotation2D, comps::Position, comps::Velocity, comps::PositionAlt, comps::VelocityAlt, comps::TestComponent, comps::Transform>(firstEntity);
        spdlog::info("remaining entities: {}", registry.getEntities().size());
        for (auto *comp: components) {
            delete comp;
        }
        components.clear();
        delete firstEntity;
        for (auto entity: entities) {
            delete entity;
        }
        entities.clear();
    };

    Results tempResults;
    run(tempResults);
    run(tempResults);

    Results results;
    for (int j = 0; j < numRuns; ++j) {
        run(results);
    }

    for (auto&[_, f]: results) {
        f /= numRuns;
    }

    return results;
}

template<typename Warmup, typename... RegistryTypes>
void runComparison(int numEntities, int runs, const std::array<std::string, sizeof...(RegistryTypes) + 1> &_names) {
    benchmarkRegistry<Warmup>(numEntities, runs);

    std::vector<Results> results;
    (results.push_back(benchmarkRegistry<RegistryTypes>(numEntities, runs)), ...);
    results.push_back(benchmarkCustomRegistry(numEntities, runs));

    for (const auto&[name, f]: results.front())
        fmt::print("{:<10}: {}\n", name, f);

    size_t maxLen = 0;
    for (const auto &name: _names)
        if (name.length() > maxLen)
            maxLen = name.length();

    fmt::print("\n{:<{}}", "registry", maxLen);
    for (const auto&[name, f]: results.front())
        fmt::print(" {:<10}", name);
    fmt::print("\n");
    for (size_t i = 0; i < _names.size(); ++i) {
        fmt::print("{:<{}} ", _names[i], maxLen);
        for (auto&[name, f]: results.front()) {
            fmt::print("{:<11.3f}", results[i][name] / f);
        }
        fmt::print("\n");
    }

}

int main(int argc, char *argv[]) {
#ifndef NDEBUG
#if defined(_MSC_VER)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//	_CrtSetBreakAlloc(2760);
#endif
#endif

    int numEntities = 2 << 16;
    int runs = 4;
    if (argc >= 3) {
        numEntities = std::stoi(argv[1]);
        runs = std::stoi(argv[2]);
    }
    std::cout << "num entities: " << numEntities << "; num runs: " << runs << "\n";

    using GameRegistry = tests::RegistryWrapper<
            game::Registry<
                    components::Position,
                    components::Velocity,
                    components::Transform,
                    components::TestComponent,
                    components::Position2D,
                    components::Rotation2D,
                    components::PositionAlt,
                    components::VelocityAlt>,
            tests::StaticAccess>;

    runComparison<GameRegistry, GameRegistry, smk::Registry>(numEntities,
                                                             runs,
                                                             {"reference", "smk", "custom"});

    return 0;
}
