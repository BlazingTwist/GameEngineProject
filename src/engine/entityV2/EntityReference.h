#ifndef ACAENGINE_ENTITYREFERENCE_v2_H
#define ACAENGINE_ENTITYREFERENCE_v2_H

#include <typeindex>
#include <unordered_map>
#include <algorithm>
#include "ComponentReference.h"

namespace entityV2 {
    struct EntityReference {
    public:
        /**
         * @return true, if the Entity referenced by this has been deleted | false otherwise.
         */
        [[nodiscard]] bool isExpired() const {
            return _isExpired;
        }

        [[nodiscard]] const std::unordered_map<std::type_index, ComponentReference *> &getComponentMap() const {
            return componentMap;
        }

        template<typename T>
        [[nodiscard]] ComponentReference *getComponent() const {
            auto findResult = componentMap.find(std::type_index(typeid(T)));
            if(findResult == componentMap.end()){
                return nullptr;
            }
            return findResult->second;
        }

        [[nodiscard]] bool containsAllComponents(const std::vector<std::type_index> &types) const {
            return std::all_of(types.begin(), types.end(), [this](const std::type_index &type) { return componentMap.find(type) != componentMap.end(); });
        }

    private:
        explicit EntityReference(unsigned int referenceID) : _referenceID(referenceID) {};

        unsigned int _referenceID;
        bool _isExpired = false;
        std::unordered_map<std::type_index, ComponentReference *> componentMap = {};

        friend class EntityRegistry;
    };
}

#endif //ACAENGINE_ENTITYREFERENCE_v2_H
