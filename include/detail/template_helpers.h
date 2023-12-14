#ifndef PPPLUGIN_DETAIL_TEMPLATE_HELPERS_H
#define PPPLUGIN_DETAIL_TEMPLATE_HELPERS_H

namespace ppplugin::detail::templates {
/**
 * Wrap types into given type "Wrapper" (which has to accept exactly one template argument,
 * but can have more defaulted template parameters) and puts them into the first given type
 * "OuterType" which has to accept as many template parameter as types were given.
 *
 * E.g. "WrapParameterPack<O, W, a, b, c>" will result in "O<W<a>, W<b>, W<c>>"
 */
template <
    template <typename...> typename OuterType,
    template <typename, typename...> typename Wrapper,
    typename... Types>
struct WrapParameterPack {
    /**
     * Custom tuple type to avoid accidentally unpacking a std::tuple
     */
    template <typename... Ts>
    struct Tuple { };

    // split parameter pack into pairs
    template <typename T, typename... Ts>
    struct PackHelper {
        using Type = Tuple<Wrapper<T>, typename PackHelper<Ts...>::Type>;
    };
    // return simple wrapped type if only one type is left
    template <typename T>
    struct PackHelper<T> {
        using Type = Wrapper<T>;
    };
    /**
     * Pack given "Ts..." into "Tuple<T0, Tuple<T1, ...>>".
     */
    template <typename... Ts>
    using Pack = typename PackHelper<Ts...>::Type;

    // unpacking is done through inheritance;
    // this is the base class which defines the final type
    template <typename T, typename...>
    struct UnpackHelper {
        using Type = T;
    };
    // move template arguments of second Tuple out of tuple
    template <typename... Ts1, typename... Ts2>
    struct UnpackHelper<Tuple<Ts1...>, Tuple<Ts2...>>
        : UnpackHelper<Tuple<Ts1...>, Ts2...> { };
    // move independent arguments (those which are not in a Tuple) one after
    // the other into first Tuple; they are already wrapped into Wrapper which
    // has to be specified to avoid ambiguity with the other specialization
    template <typename... Ts1, typename T, typename... Ts2>
    struct UnpackHelper<Tuple<Ts1...>, Wrapper<T>, Ts2...>
        : UnpackHelper<Tuple<Ts1..., Wrapper<T>>, Ts2...> { };
    /**
     * Unpack given "T" with nested "Tuple"s into flat "Tuple".
     */
    template <typename T>
    using Unpack = typename UnpackHelper<Tuple<>, T>::Type;

    // this type is not used, it only exists to allow the specialization below
    template <typename... Args>
    struct TupleToOuterType { };
    /**
     * Convert "Tuple<...>" to "OuterType<...>"
     */
    template <typename... Args>
    struct TupleToOuterType<Tuple<Args...>> {
        using Type = OuterType<Args...>;
    };

    /**
     * Pack given types into nested tuples, then flatten the tuples and convert
     * to correct OuterType.
     */
    using Type = typename TupleToOuterType<Unpack<Pack<Types...>>>::Type;
};

} // namespace ppplugin::detail::templates

#endif // PPPLUGIN_DETAIL_TEMPLATE_HELPERS_H
