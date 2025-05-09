#ifndef PPPLUGIN_TEST_HELPER_H
#define PPPLUGIN_TEST_HELPER_H

#include <array>
#include <string>
#include <tuple>

#include <ppplugin/detail/compatibility_utils.h>
#include <ppplugin/expected.h>

namespace ppplugin::test {
namespace detail {
    template <std::size_t outerIndex, std::size_t innerIndex,
        typename Func, typename TupleOne, typename TupleTwo>
    constexpr void forEachCombinationImpl(Func&& func,
        TupleOne&& outer_args, TupleTwo&& inner_args)
    {
        constexpr auto OUTER_SIZE = std::tuple_size<TupleOne>();
        constexpr auto INNER_SIZE = std::tuple_size<TupleTwo>();
        if constexpr (innerIndex >= INNER_SIZE) {
            if constexpr (outerIndex < OUTER_SIZE) {
                forEachCombinationImpl<outerIndex + 1, 0>(std::forward<Func>(func),
                    std::forward<TupleOne>(outer_args),
                    std::forward<TupleTwo>(inner_args));
            }
        } else if constexpr (outerIndex < OUTER_SIZE) {
            auto&& a = std::get<outerIndex>(outer_args);
            auto&& b = std::get<innerIndex>(inner_args);
            using A = decltype(a);
            using B = decltype(b);
            if constexpr (std::is_invocable_v<Func, A, B>) {
                func(a, b);
            } else if constexpr (std::is_invocable_v<Func, A, B, std::size_t>) {
                func(a, b, outerIndex * OUTER_SIZE + innerIndex);
            } else if constexpr (std::is_invocable_v<Func,
                                     A, B, std::size_t, std::size_t>) {
                func(a, b, outerIndex, innerIndex);
            } else {
                static_assert(!sizeof(Func), "No known way to call given function!");
            }

            if constexpr (innerIndex < INNER_SIZE) {
                forEachCombinationImpl<outerIndex, innerIndex + 1>(std::forward<Func>(func),
                    std::forward<TupleOne>(outer_args),
                    std::forward<TupleTwo>(inner_args));
            }
        }
    }
} // namespace detail

constexpr void noop() { }

template <typename Func, typename... Args>
constexpr void forEachCombination(Func&& func, Args&&... args)
{
    if constexpr (sizeof...(Args) > 0) {
        detail::forEachCombinationImpl<0, 0>(std::forward<Func>(func),
            std::make_tuple(std::forward<Args>(args)...),
            std::make_tuple(std::forward<Args>(args)...));
    }
}

template <typename T, typename E>
[[nodiscard]] static std::string errorOutput(const ppplugin::Expected<T, E>& expected)
{
    auto error = *expected.error();

    std::string output;
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    output += error.where();
    output += " - ";
#endif // PPPLUGIN_CPP17_COMPATIBILITY

    output += error.what();
    output += " (";
    output += codeToString(error.code());
    output += ")";

    return output;
}
} // namespace ppplugin::test

#endif // PPPLUGIN_TEST_HELPER_H
