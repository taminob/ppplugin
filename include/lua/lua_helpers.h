#ifndef PPPLUGIN_LUA_LUA_HELPERS_H
#define PPPLUGIN_LUA_LUA_HELPERS_H

#include <optional>
#include <tuple>

// TODO: move to detail?

namespace ppplugin {
class LuaState;

template<typename>
struct PopTuple;

template <typename... Ts>
struct PopTuple<std::tuple<Ts...>> {
    template <typename State>
    static std::optional<std::tuple<Ts...>> pop(State& state);
};
template <typename T, typename... Ts>
struct PopTuple<std::tuple<T, Ts...>> {
    template <typename State>
    static std::optional<std::tuple<T, Ts...>> pop(State& state);
};

template <typename... Ts>
template <typename State>
std::optional<std::tuple<Ts...>> PopTuple<std::tuple<Ts...>>::pop(State& state)
{
    if constexpr (sizeof...(Ts) > 0) {
        return PopTuple<std::tuple<Ts...>>::pop(state);
    } else {
        return std::tuple<> {};
    }
}

template <typename T, typename... Ts>
template <typename State>
std::optional<std::tuple<T, Ts...>> PopTuple<std::tuple<T, Ts...>>::pop(State& state)
{
    if (auto result = state.template pop<T>()) {
        if (auto others = PopTuple<std::tuple<Ts...>>::pop(state)) {
            return std::tuple_cat(*others, std::make_tuple(*result));
        }
    }
    return std::nullopt; // TODO: add concrete error
}
} // namespace ppplugin

#endif // PPPLUGIN_LUA_LUA_HELPERS_H
