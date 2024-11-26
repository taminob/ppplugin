#ifndef PPPLUGIN_DETAIL_FUNCTION_DETAILS_H
#define PPPLUGIN_DETAIL_FUNCTION_DETAILS_H

#include "ppplugin/detail/template_helpers.h"

#include <tuple>

namespace ppplugin::detail::templates {
template <typename T>
struct FunctionDetails;

template <typename R, typename... Ts>
struct FunctionDetails<R(Ts...)> {
    using ReturnType = R;
    static constexpr auto ARGUMENT_COUNT = sizeof...(Ts);
    template <std::size_t index>
    using Argument = std::tuple_element<index, std::tuple<Ts...>>;
};

/**
 * Return number of return types for given FunctionDetails:
 * - for std::tuple - number of tuple elements
 * - for void - zero
 * - everything else - one
 */
template <typename FunctionDetails>
constexpr std::size_t returnTypeCount()
{
    if constexpr (detail::templates::IsStdTuple<typename FunctionDetails::ReturnType>::value) {
        return std::tuple_size_v<typename FunctionDetails::ReturnType>;
    } else if constexpr (std::is_void_v<typename FunctionDetails::ReturnType>) {
        return 0;
    } else {
        return 1;
    }
}
} // namespace ppplugin::detail::templates

#endif // PPPLUGIN_DETAIL_FUNCTION_DETAILS_H
