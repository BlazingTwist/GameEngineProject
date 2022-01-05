#ifndef ACAENGINE_COMPONENT_H
#define ACAENGINE_COMPONENT_H

#include <vector>

namespace entity {
    struct Component {
        int entityReferenceID;
        std::vector<std::byte> componentBytes = {};

        template<typename T_component>
        explicit Component(int entityReferenceID, T_component componentData)
                : entityReferenceID(entityReferenceID) {
            setData(componentData);
        }

        template<typename T_component>
        T_component getComponentData() const {
            // http://www.cplusplus.com/forum/beginner/155821/
            // http://en.cppreference.com/w/cpp/types/is_trivially_copyable
            static_assert(std::is_trivially_copyable<T_component>::value, "not a TriviallyCopyable type");

            T_component result;
            auto *result_begin = reinterpret_cast<std::byte * > (std::addressof(result));
            std::copy(componentBytes.begin(), componentBytes.end(), result_begin);
            return result;
        }

        template<typename T_component>
        void setData(T_component data){
            componentBytes.clear();
            componentBytes.reserve(sizeof(T_component));
            const auto *begin = reinterpret_cast< const std::byte * >(std::addressof(data));
            const std::byte *end = begin + sizeof(T_component);
            std::copy(begin, end, std::back_inserter(componentBytes));
        }
    };
}

#endif //ACAENGINE_COMPONENT_H
