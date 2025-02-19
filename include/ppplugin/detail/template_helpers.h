#ifndef PPPLUGIN_DETAIL_TEMPLATE_HELPERS_H
#define PPPLUGIN_DETAIL_TEMPLATE_HELPERS_H

#include <type_traits>
#include <variant>

namespace ppplugin::detail::templates {
namespace helper {
    /**
     * Custom tuple type to avoid accidentally mistreating std::tuple.
     *
     * Cannot access arguments.
     * Use "TupleToType" to insert arguments into target type.
     */
    template <typename...>
    struct Tuple { };

    template <template <typename...> typename, typename...>
    struct TupleToType { };
    /**
     * Convert "Tuple<...>" to "OuterType<...>"
     */
    template <
        template <typename...> typename TargetType,
        typename... Args>
    struct TupleToType<TargetType, Tuple<Args...>> {
        using Type = TargetType<Args...>;
    };
    template <template <typename...> typename TargetType, typename... Args>
    using TupleToTypeT = typename TupleToType<TargetType, Args...>::Type;
} // namespace helper

/**
 * Remove duplicates from parameter pack.
 */
template <template <typename...> typename OuterType, typename... Types>
class Unique {
    // this type is not used, it only exists to allow the specialization below
    template <typename... Args>
    using Tuple = helper::Tuple<Args...>;

    // deduplication is achieved via inheritance.
    // this is the base class which will define the final type
    template <typename T, typename...>
    struct Helper {
        using Type = T;
    };
    // if the type "T" is not yet in "Ts1...", add it, otherwise
    // skip it and resume until there is nothing left
    template <typename... Ts1, typename T, typename... Ts2>
    struct Helper<Tuple<Ts1...>, T, Ts2...>
        : std::conditional_t<(std::is_same_v<T, Ts1> || ...),
              Helper<Tuple<Ts1...>, Ts2...>,
              Helper<Tuple<Ts1..., T>, Ts2...>> { };

public:
    using Type = typename helper::TupleToType<
        OuterType,
        typename Helper<Tuple<>, Types...>::Type>::Type;
};
template <template <typename...> typename OuterType, typename... Types>
using UniqueT = typename Unique<OuterType, Types...>::Type;

template <typename... Types>
using UniqueVariant = UniqueT<std::variant, Types...>;

/**
 * Get first template type of a parameter pack.
 *
 * Compilation will fail if the parameter pack is empty.
 */
template <typename... Types>
class First {
    template <typename T, typename...>
    struct Helper {
        using Type = T;
    };

public:
    static_assert(sizeof...(Types) > 0,
        "To get the first type, the parameter pack cannot be empty!");
    using Type = typename Helper<Types...>::Type;
};
template <typename... Types>
using FirstT = typename First<Types...>::Type;

/**
 * Get first template type of a parameter pack or, if the parameter pack is empty, the
 * first specified type.
 */
template <typename, typename... Types>
struct FirstOr {
    using Type = FirstT<Types...>;
};
template <typename DefaultType>
struct FirstOr<DefaultType> {
    using Type = DefaultType;
};
template <typename DefaultType, typename... Types>
using FirstOrT = typename FirstOr<DefaultType, Types...>::Type;

/**
 * Get second template parameter of given class.
 * If the class does not have any or sufficient template parameters,
 * return given DefaultType.
 */
template <typename DefaultType, typename>
struct SecondTemplateParameterOr {
    using Type = DefaultType;
};
template <typename DefaultType, template <typename...> typename T,
    typename U, typename V, typename... Ts>
struct SecondTemplateParameterOr<DefaultType, T<U, V, Ts...>> {
    using Type = V;
};
template <typename... Ts>
using SecondTemplateParameterOrT = typename SecondTemplateParameterOr<Ts...>::Type;

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
class WrapParameterPack {
    template <typename... Args>
    using Tuple = helper::Tuple<Args...>;

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
     * Pack given "Ts..." into "Tuple<Wrapper<T0>, Tuple<Wrapper<T1>, ...>>".
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
     *
     * E.g. "Tuple<T1, Tuple<T2, Tuple<...>>>" to "Tuple<T1, T2, T3>"
     */
    template <typename T>
    using Unpack = typename UnpackHelper<Tuple<>, T>::Type;

public:
    /**
     * Pack given types into nested tuples, then flatten the tuples and convert
     * to correct OuterType.
     */
    using Type = typename helper::TupleToType<OuterType,
        Unpack<Pack<Types...>>>::Type;
};

template <typename ActualType, template <typename...> typename ExpectedType>
struct IsSpecialization : std::false_type { };
template <template <typename...> typename ExpectedType, typename... Ts>
struct IsSpecialization<ExpectedType<Ts...>, ExpectedType> : std::true_type { };

template <typename T>
using IsStdTuple = IsSpecialization<T, std::tuple>;

/**
 * C++17 compatible version of std::remove_cvref_t.
 */
template <typename T>
using RemoveCvrefT = std::remove_cv_t<std::remove_reference_t<T>>;
} // namespace ppplugin::detail::templates

#endif // PPPLUGIN_DETAIL_TEMPLATE_HELPERS_H
