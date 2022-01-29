#include "testutils.hpp"

#include "engine/entity/entityregistry.h"
#include <optional>
#include <vector>

struct Foo {
    int i;
};

struct Bar {
    float f;
};

int main() {

    entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

    std::vector<entity::EntityReference *> entities;

    entities.reserve(5);
    for (int i = 0; i < 5; ++i) {
        entities.push_back(registry.createEntity());
    }

    auto refDel = entities[2];

    EXPECT(registry.getEntityData(refDel), "Reference is valid after creation.");
    registry.eraseEntity(refDel);
    EXPECT(!registry.getEntityData(refDel), "Reference is invalid after delete.");

    entities[2] = registry.createEntity();
    for (int i = 0; i < 6; ++i)
        entities.push_back(registry.createEntity());

    EXPECT(!registry.getEntityData(refDel), "Reference remains invalid after reuse of the id.");

    delete refDel; // done playing with this.

    {
        for (int i = 0; i < static_cast<int>(entities.size()); ++i) {
            registry.addOrSetComponent<Foo>(entities[i], Foo{i});
            std::optional<const Foo> foo = registry.getComponentData<Foo>(entities[i]);
            EXPECT(foo.has_value() && foo->i == i, "Add a component.");
        }
    }

    {
        for (int i = 0; i < static_cast<int>(entities.size()); i += 3) {
            registry.addOrSetComponent<Bar>(entities[i], Bar{static_cast<float>(i)});
            std::optional<const Bar> bar = registry.getComponentData<Bar>(entities[i]);
            EXPECT(bar.has_value() && bar->f == static_cast<float>(i), "Add a component.");
        }
    }

    {

        for (auto &entity: entities) {
            EXPECT(registry.getComponentData<Foo>(entity), "Retrieve a component.");
        }

        registry.removeComponent<Foo>(entities[0]);
        registry.removeComponent<Foo>(entities[1]);

        EXPECT(!registry.getComponentData<Foo>(entities[0]), "Remove a component.");
        EXPECT(!registry.getComponentData<Foo>(entities[1]), "Remove a component.");


        registry.eraseEntity(entities[2]);

        for (int i = 3; i < static_cast<int>(entities.size()); ++i) {
            EXPECT(registry.getComponentData<Foo>(entities[i]), "Other components are untouched.");
            EXPECT(registry.getComponentData<Foo>(entities[i])->i == i, "Other components are untouched.");
        }
    }

    int sum = 0;
    registry.execute([&sum](const Foo foo) { sum += foo.i; });
    // without auto deduction
    // registry.execute<Foo>([&sum](const Foo& foo) { sum += foo.i; });
    EXPECT(sum == 10 * 11 / 2 - 3, "Execute action on a single component type.");

    sum = 0;
    registry.execute([&sum](const Bar bar, const Foo foo) { sum += foo.i - 2 * static_cast<int>(bar.f); });
    //registry.execute<Bar,Foo>([&sum](const Bar& bar, const Foo& foo) { sum += foo.i - 2 * static_cast<int>(bar.f); });
    EXPECT(sum == -3 - 6 - 9, "Execute action on multiple component types.");

    // registry.execute<Entity, Bar>([&](Entity ent, Bar& bar)
    {
        registry.execute([&](const entity::EntityReference *ent, const Bar bar) {
            auto pBar = registry.getComponentData<Bar>(ent);
            EXPECT(pBar, "Execute provides the correct entity.");
            EXPECT(pBar->f == bar.f, "Execute provides the correct entity.");
            registry.addOrSetComponent(ent, Bar{-1.f}); // TODO I'm have mixed feelings about this.
        });
    }

    {
        for (size_t i = 3; i < entities.size(); i += 3) {
            auto pBar = registry.getComponentData<Bar>(entities[i]);
            EXPECT(pBar, "Action can change components.");
            EXPECT(pBar->f == -1.f, "Action can change components.");
        }
    }

    for (entity::EntityReference *entity: entities) {
        registry.eraseEntity(entity);
        delete entity;
    }
    entities.clear();
}
