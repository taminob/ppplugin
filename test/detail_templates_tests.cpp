#include "detail/template_helpers.h"

#include <memory>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

// NOLINTNEXTLINE(google-build-using-namespace); this is alright for tests
using namespace ppplugin::detail::templates;

// Tests for First / FirstOr

static_assert(
    std::is_same_v<ppplugin::detail::templates::FirstT<int>,
        int>);
static_assert(
    std::is_same_v<ppplugin::detail::templates::FirstT<float, double, int>,
        float>);
static_assert(
    std::is_same_v<ppplugin::detail::templates::FirstOrT<float, bool, int>,
        bool>);
static_assert(
    std::is_same_v<ppplugin::detail::templates::FirstOrT<bool, std::unique_ptr<int>>,
        std::unique_ptr<int>>);
static_assert(
    std::is_same_v<ppplugin::detail::templates::FirstOrT<std::vector<double>>,
        std::vector<double>>);

// Tests for WrapParameterPack

template <typename... Ts>
using O1 = std::tuple<Ts...>;
template <typename... Ts>
using O2 = std::variant<Ts...>;

template <typename T>
using W1 = std::unique_ptr<T>;
template <typename T>
using W2 = std::shared_ptr<T>;
template <typename T>
using W3 = std::vector<T>;

static_assert(
    std::is_same_v<WrapParameterPack<O1, W1, int, O1<W1<int>>, float, double>::Type,
        O1<W1<int>, W1<O1<W1<int>>>, W1<float>, W1<double>>>);

static_assert(
    std::is_same_v<WrapParameterPack<O1, W1, int, float, double>::Type,
        O1<W1<int>, W1<float>, W1<double>>>);

static_assert(
    std::is_same_v<WrapParameterPack<O2, W3, double, float>::Type,
        O2<W3<double>, W3<float>>>);

static_assert(
    std::is_same_v<WrapParameterPack<O2, W1, int>::Type,
        O2<W1<int>>>);

static_assert(
    std::is_same_v<WrapParameterPack<O2, W3, O2<bool>>::Type,
        O2<W3<O2<bool>>>>);

static_assert(
    std::is_same_v<WrapParameterPack<W2, W2, W2<bool>>::Type,
        W2<W2<W2<bool>>>>);
