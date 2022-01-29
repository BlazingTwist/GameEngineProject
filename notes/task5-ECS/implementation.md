## Initial Specification vs Actual Implementation

### ComponentAccess

#### Adding a new Component
```c++
// Specification
//  Add a new component to an existing entity.
//  No changes are done, if entity already has a component of this type.
//  @return A reference to the new component or the already existing component.
Component& insert(Entity _ent, const Component& _comp);

// Implementation
//  Add a new component to an existing entity.
//  Update the component if the entity already has a component of this type.
template<typename T_component>
void addOrSetComponent(const EntityReference *reference, T_component component);
```

#### Getting an existing Component
```c++
// Specification
//  Retrieve a component of _ent.
//  @return A reference to the new component or the already existing component.
Component* at(Entity _ent);
const Component* at(Entity _ent) const;

// Implementation
//  Retrieve a component of an Entity
template<typename T_component>
[[nodiscard]] std::optional<const T_component>
        getComponentData(const EntityReference *reference) const;
```

#### Removing an existing Component
```c++
// Specification
//  Remove a component from an existing entity.
//  Does not check whether it exists.
void erase(Entity _ent);

// Implementation
//  Remove a component from an existing entity.
//  Does nothing, if entity/component does not exist
template<typename T_component>
void removeComponent(const EntityReference *reference);
```

### Registry

#### Creating a new Entity
```c++
// Specification
Entity create();

// Implementation
//  Creates a new Entity with the specified Components
//  @return an EntityReference, caller needs to delete the pointer when done.
template<typename... T_Components>
EntityReference *createEntity(T_Components...args);
```

#### Deleting an existing Entity
```c++
// Specification
void erase(Entity _ent);

// Implementation
void eraseEntity(EntityReference *reference)
```

#### Get a Reference to an existing Entity
```c++
// Specification
EntityRef getRef(Entity _ent) const;

// Implementation: not implemented.
```

#### Get the Entity via a Reference
```c++
// Specification
std::optional<Entity> getEntity(EntityRef _ent) const;

// Implementation
[[nodiscard]]
std::optional<const Entity> getEntityData(const EntityReference *reference) const;
```

#### ComponentAccess
`ComponentAccess` is now `ComponentRegistry`, but it is recommended to leave
access/management of these Registries to the `EntityRegistry`.

#### Executing Actions on Entities
```c++
// Specification
//  Execute an Action on all entities having the components expected by Action::operator(component_type&...).
//  In addition, the entity itself is provided, if the first parameter is of type Entity
template<typename Action>
void execute(const Action& _action);

// Implementation
//  Execute an Action on all entities having the components expected by Action::operator(const TComponent ...).
//  In addition, the EntityReference is provided, if the first parameter is of type EntityReference. (const EntityReference *)
//
//  main difference: Components are not passed as reference, Entity passed as const pointer.
template<typename Action>
void execute(const Action &action);
```