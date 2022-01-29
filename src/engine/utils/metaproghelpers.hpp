#pragma once

#include <typeindex>
#include <vector>

namespace utils {

    template<typename T>
    static std::type_index getTypeIndex() {
        return std::type_index(typeid(T));
    }

    template<typename ...Ts>
    static void gatherTypeIDs(std::vector<std::type_index> &indexVector) {
        indexVector = {getTypeIndex<Ts>()...};
    }

    /**
     * @tparam Functor auto-deduced Type that overrides the call operator 'operator()'
     * @tparam Args auto-deduced call parameter Types
     * @param typeIndices a vector that will be filled with the parameters type_index in order
     */
    template<typename Functor, typename ...Args>
    static void unpackFunctorArguments(void(Functor::*)(Args...) const, std::vector<std::type_index> &typeIndices) {
        gatherTypeIDs<Args...>(typeIndices);
    }

    /**
     * @tparam Functor auto-deduced Type that overrides the call operator 'operator()'
     * @tparam Args auto-deduced call parameter Types
     * @param typeIndices a vector that will be filled with the parameters type_index in order
     */
    template<typename Functor, typename ...Args>
    static void unpackFunctorArguments(void(Functor::*)(Args...), std::vector<std::type_index> &typeIndices) {
        gatherTypeIDs<Args...>(typeIndices);
    }

    // Helper for type deduction that does not impose any conditions on T.
    template<typename T>
    struct TypeHolder {
        using type = T;
    };

    // Determine whether a parameter pack contains a specific type.
    template<typename What, typename ... Args>
    struct contains_type {
        static constexpr bool value{(std::is_same_v<What, Args> || ...)};
    };

    // Version that handles a tuple.
    template<typename What, typename... Args>
    struct contains_type<What, std::tuple<Args...>> {
        static constexpr bool value{(std::is_same_v<What, Args> || ...)};
    };

    template<typename What, typename ... Args>
    constexpr bool contains_type_v = contains_type<What, Args...>::value;
}