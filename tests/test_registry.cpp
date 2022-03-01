#include "testutils.hpp"
#include "registry/registry.h"

#include "engine/entityV2/EntityRegistry.h"
#include <optional>
#include <vector>

struct Foo {
    Foo() {}

    Foo(int _i) : i(_i) {}

    int i;
};

struct Bar {
    float f;
};

int main() {

    // I suspect that the registry using pointers will fail if an entity is removed
    /*game::Registry<Foo> profRegistry;
    auto entity1 = profRegistry.create();
    auto entity2 = profRegistry.create();
    auto entity3 = profRegistry.create();
    auto& fooComps2 = profRegistry.getContainer<Foo>();
    fooComps2.emplace(entity1.toIndex(), Foo(1));
    fooComps2.emplace(entity2.toIndex(), Foo(2));
    fooComps2.emplace(entity3.toIndex(), Foo(3));
    
    Foo &foo1 = profRegistry.getComponent<Foo>(entity1);
    Foo &foo2 = profRegistry.getComponent<Foo>(entity2);
    Foo &foo3 = profRegistry.getComponent<Foo>(entity3);
    spdlog::info("foo1 = {}", foo1.i);
    spdlog::info("foo2 = {}", foo2.i);
    spdlog::info("foo3 = {}", foo3.i);
    profRegistry.erase(entity2);
    spdlog::info("foo1 = {}", foo1.i);
    spdlog::info("foo2 = {}", foo2.i);
    spdlog::info("foo3 = {}", foo3.i);
    
    spdlog::info("foo1? {}", (profRegistry.hasComponent<Foo>(entity1)));
    spdlog::info("foo2? {}", (profRegistry.hasComponent<Foo>(entity2)));
    spdlog::info("foo3? {}", (profRegistry.hasComponent<Foo>(entity3)));*/

    /*exComps1.insert(entity1, entity::ExComp1(1));
    exComps1.insert(entity2, entity::ExComp1(2));
    exComps1.insert(entity3, entity::ExComp1(3));
    
    entity::ExComp1* comp1 = exComps1.at(entity1);
    entity::ExComp1* comp2 = exComps1.at(entity2);
    entity::ExComp1* comp3 = exComps1.at(entity3);
    spdlog::info("comp1 = {} | entID {}", comp1->data, entity1.id);
    spdlog::info("comp2 = {} | entID {}", comp2->data, entity2.id);
    spdlog::info("comp3 = {} | entID {}", comp3->data, entity3.id);
    smkRegistry.erase(entity1);
    spdlog::info("comp1 = {}", comp1->data);
    spdlog::info("comp2 = {}", comp2->data);
    spdlog::info("comp3 = {}", comp3->data);

    comp1 = exComps1.at(entity1);
    comp2 = exComps1.at(entity2);
    comp3 = exComps1.at(entity3);
    spdlog::info("comp1 = {}", (comp1 == nullptr));
    spdlog::info("comp2 = {}", comp2->data);
    spdlog::info("comp3 = {}", comp3->data);
    
    entity::ExComp1* compPtr = reinterpret_cast<entity::ExComp1*>(exComps1.m_targetStorage.buffer.data());
    for (std::size_t i = 0; i < 3; ++i) {
        spdlog::info("found comp: {}", (compPtr + i)->data);
    }*/

    entityV2::EntityRegistry &registry = entityV2::EntityRegistry::getInstance();
    std::vector<entityV2::EntityReference *> entities;
    std::vector<entityV2::ComponentReference *> components;

    entities.reserve(5);
    for (int i = 0; i < 5; ++i) {
        entities.push_back(registry.createEntity());
    }

    auto refDel = entities[2];

    EXPECT(!refDel->isExpired(), "Reference is valid after creation.");
    registry.eraseEntity(refDel);
    EXPECT(refDel->isExpired(), "Reference is invalid after delete.");

    entities[2] = registry.createEntity();
    for (int i = 0; i < 6; ++i)
        entities.push_back(registry.createEntity());

    EXPECT(refDel->isExpired(), "Reference remains invalid after reuse of the id.");

    delete refDel; // done playing with this.

    {
        for (int i = 0; i < static_cast<int>(entities.size()); ++i) {
            components.push_back(registry.addOrSetComponent<Foo>(entities[i], new Foo(i)));
            entityV2::ComponentReference *fooReference = entities[i]->getComponent<Foo>();
            EXPECT(fooReference != nullptr && fooReference->isPresent() && fooReference->getComponent<Foo>()->i == i,
                   "Add a component.");
        }
    }

    {
        for (int i = 0; i < static_cast<int>(entities.size()); i += 3) {
            components.push_back(registry.addOrSetComponent<Bar>(entities[i], new Bar{static_cast<float>(i)}));
            entityV2::ComponentReference *barReference = entities[i]->getComponent<Bar>();
            EXPECT(barReference != nullptr && barReference->isPresent() && barReference->getComponent<Bar>()->f == static_cast<float>(i),
                   "Add a component.");
        }
    }

    {

        for (auto &entity: entities) {
            entityV2::ComponentReference *fooReference = entity->getComponent<Foo>();
            EXPECT(fooReference != nullptr && fooReference->isPresent(), "Retrieve a component.");
        }

        registry.removeComponent<Foo>(entities[0]);
        registry.removeComponent<Foo>(entities[1]);

        entityV2::ComponentReference *comp0 = entities[0]->getComponent<Foo>();
        entityV2::ComponentReference *comp1 = entities[1]->getComponent<Foo>();
        EXPECT(comp0 == nullptr, "Remove a component.");
        EXPECT(comp1 == nullptr, "Remove a component.");


        registry.eraseEntity<Foo>(entities[2]);

        for (int i = 3; i < static_cast<int>(entities.size()); ++i) {
            entityV2::ComponentReference *component = entities[i]->getComponent<Foo>();
            EXPECT(component != nullptr && component->isPresent(), "Other components are untouched.");
            EXPECT(component->getComponent<Foo>()->i == i, "Other components are untouched.");
        }
    }

    int sum = 0;
    registry.execute([&sum](Foo *foo) { sum += foo->i; });
    // without auto deduction
    // registry.execute<Foo>([&sum](const Foo& foo) { sum += foo.i; });
    EXPECT(sum == 10 * 11 / 2 - 3, "Execute action on a single component type.");

    sum = 0;
    registry.execute([&sum](Bar *bar, Foo *foo) { sum += foo->i - 2 * static_cast<int>(bar->f); });
    //registry.execute<Bar,Foo>([&sum](const Bar& bar, const Foo& foo) { sum += foo.i - 2 * static_cast<int>(bar.f); });
    EXPECT(sum == -3 - 6 - 9, "Execute action on multiple component types.");

    // registry.execute<Entity, Bar>([&](Entity ent, Bar& bar)
    {
        registry.execute([&](entityV2::EntityReference *ent, Bar *bar) {
            auto *pBar = ent->getComponent<Bar>();
            EXPECT(pBar != nullptr && pBar->isPresent(), "Execute provides the correct entity.");
            EXPECT(pBar->getComponent<Bar>()->f == bar->f, "Execute provides the correct entity.");
            bar->f = -1.f;
        });
    }

    {
        for (size_t i = 3; i < entities.size(); i += 3) {
            auto pBar = entities[i]->getComponent<Bar>();
            EXPECT(pBar != nullptr && pBar->isPresent(), "Action can change components.");
            EXPECT(pBar->getComponent<Bar>()->f == -1.f, "Action can change components.");
        }
    }

    for (entityV2::EntityReference *entity: entities) {
        registry.eraseEntity<Foo, Bar>(entity);
        delete entity;
    }
    entities.clear();
}
