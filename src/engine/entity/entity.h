#ifndef ACAENGINE_ENTITY_H
#define ACAENGINE_ENTITY_H

#include <unordered_map>
#include <typeindex>

namespace entity {
    struct Entity {
        std::unordered_map<std::type_index, int> componentMap = {};
    };
}

#endif //ACAENGINE_ENTITY_H
