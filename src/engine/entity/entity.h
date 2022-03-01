#ifndef ACAENGINE_ENTITY_H
#define ACAENGINE_ENTITY_H

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <algorithm>

namespace entity {
    struct Entity {
        /**
         * A mapping of type_index to componentIDs
         */
        std::unordered_map<std::type_index, int> componentMap = {};

        [[nodiscard]] bool containsAllComponents(const std::vector<std::type_index> &types) const {
            return std::all_of(types.begin(), types.end(), [this](const std::type_index &type) { return componentMap.find(type) != componentMap.end(); });
        }
    };
}

#endif //ACAENGINE_ENTITY_H
