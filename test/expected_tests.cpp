#include "ppplugin/expected.h"
#include "test_helper.h"
#include "test_types.h"

#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ppplugin;

TEST(Expected, valueConstruction)
{
    [[maybe_unused]] const Expected<int, bool> v1 { 0 };
    [[maybe_unused]] const Expected<bool, int> v2 { false };
    [[maybe_unused]] const Expected<test::NonDefaultConstructible, int> v3 {
        test::NonDefaultConstructible { 1, 2, 3 }
    };
    [[maybe_unused]] const Expected<test::NonCopyable, int> v4 { test::NonCopyable {} };
}

TEST(Expected, valueNonTrivialDestruction)
{
    testing::MockFunction<std::function<void()>> f;
    EXPECT_CALL(f, Call()).Times(1);
    [[maybe_unused]] const Expected<test::NonTrivialDestructible, bool> v {
        test::NonTrivialDestructible { f.AsStdFunction() }
    };
}

TEST(Expected, valueTrivialDestruction)
{
    [[maybe_unused]] const Expected<int, test::NonTrivialDestructible> v { 1 };
}

TEST(Expected, errorNonTrivialDestruction)
{
    testing::MockFunction<std::function<void()>> f;
    EXPECT_CALL(f, Call()).Times(1);
    [[maybe_unused]] const Expected<bool, test::NonTrivialDestructible> e {
        test::NonTrivialDestructible { f.AsStdFunction() }
    };
}

TEST(Expected, errorTrivialDestruction)
{
    [[maybe_unused]] const Expected<test::NonTrivialDestructible, bool> e { false };
}

TEST(Expected, trivialDestruction)
{
    [[maybe_unused]] const Expected<int, bool> e { false };
}

TEST(Expected, simpleCopyValueConstructor)
{
    const Expected<test::CopyAndMovable, double> v { test::CopyAndMovable {} };
    auto copy_target { v };
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(copy_target.hasValue());
    EXPECT_EQ(copy_target->copied(), 1);
}

TEST(Expected, simpleCopyErrorConstructor)
{
    const Expected<int, test::CopyAndMovable> v { test::CopyAndMovable {} };
    auto copy_target { v };
    ASSERT_FALSE(v.hasValue());
    EXPECT_FALSE(copy_target.hasValue());
    EXPECT_EQ(copy_target.errorRef()->get().copied(), 1);
}

TEST(Expected, similarCopyValueConstructor)
{
    const Expected<double, unsigned int> v { 1.0 };
    const Expected<float, double> copy_target { v };
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(copy_target.hasValue());
}

TEST(Expected, similarCopyErrorConstructor)
{
    const Expected<int, unsigned int> v { 32U };
    const Expected<float, double> copy_target { v };
    ASSERT_FALSE(v.hasValue());
    EXPECT_FALSE(copy_target.hasValue());
}

TEST(Expected, simpleCopyValueAssignment)
{
    const Expected<test::CopyAndMovable, double> v { test::CopyAndMovable {} };
    std::remove_cv_t<decltype(v)> copy_target;
    copy_target = v;
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(copy_target.hasValue());
    EXPECT_EQ(copy_target->copied(), 1);
}

TEST(Expected, simpleCopyErrorAssignment)
{
    const Expected<int, test::CopyAndMovable> v { test::CopyAndMovable {} };
    std::remove_cv_t<decltype(v)> copy_target;
    copy_target = v;
    ASSERT_FALSE(v.hasValue());
    EXPECT_FALSE(copy_target.hasValue());
    EXPECT_EQ(copy_target.errorRef()->get().copied(), 1);
}

TEST(Expected, similarCopyValueAssignment)
{
    const Expected<bool, unsigned int> v { true };
    Expected<int, double> copy_target;
    copy_target = v;
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(copy_target.hasValue());
}

TEST(Expected, similarCopyErrorAssignment)
{
    const Expected<int, const char*> v { "abc" };
    Expected<double, std::string> copy_target;
    copy_target = v;
    ASSERT_FALSE(v.hasValue());
    EXPECT_FALSE(copy_target.hasValue());
}

TEST(Expected, simpleMoveValueConstructor)
{
    Expected<test::CopyAndMovable, double> v { test::CopyAndMovable {} };
    EXPECT_TRUE(v.hasValue());
    EXPECT_EQ(v->moved(), 1);
    auto move_target { std::move(v) };
    EXPECT_TRUE(move_target.hasValue());
    EXPECT_EQ(move_target->moved(), 2);
    EXPECT_EQ(move_target->copied(), 0);
}

TEST(Expected, simpleMoveErrorConstructor)
{
    Expected<int, test::CopyAndMovable> v { test::CopyAndMovable {} };
    EXPECT_FALSE(v.hasValue());
    EXPECT_EQ(v.errorRef()->get().moved(), 1);
    auto move_target { std::move(v) };
    EXPECT_FALSE(move_target.hasValue());
    EXPECT_EQ(move_target.errorRef()->get().moved(), 2);
    EXPECT_EQ(move_target.errorRef()->get().copied(), 0);
}

TEST(Expected, similarMoveValueConstructor)
{
    Expected<void*, std::string> v { nullptr };
    ASSERT_TRUE(v.hasValue());
    const Expected<bool, const std::string> move_target { std::move(v) };
    EXPECT_TRUE(move_target.hasValue());
}

TEST(Expected, similarMoveErrorConstructor)
{
    Expected<float, std::string> v { "abc" };
    ASSERT_FALSE(v.hasValue());
    const Expected<float, std::optional<std::string>> move_target { std::move(v) };
    EXPECT_FALSE(move_target.hasValue());
}

TEST(Expected, simpleMoveValueAssignment)
{
    Expected<test::CopyAndMovable, double> v { test::CopyAndMovable {} };
    decltype(v) move_target;
    EXPECT_TRUE(v.hasValue());
    EXPECT_EQ(v->moved(), 1);
    move_target = std::move(v);
    EXPECT_TRUE(move_target.hasValue());
    EXPECT_EQ(move_target->moved(), 2);
    EXPECT_EQ(move_target->copied(), 0);
}

TEST(Expected, simpleMoveErrorAssignment)
{
    Expected<int, test::CopyAndMovable> v { test::CopyAndMovable {} };
    decltype(v) move_target;
    EXPECT_FALSE(v.hasValue());
    EXPECT_EQ(v.errorRef()->get().moved(), 1);
    move_target = std::move(v);
    EXPECT_FALSE(move_target.hasValue());
    EXPECT_EQ(move_target.errorRef()->get().moved(), 2);
    EXPECT_EQ(move_target.errorRef()->get().copied(), 0);
}

TEST(Expected, similarMoveValueAssignment)
{
    Expected<bool, std::string> v { true };
    Expected<int, std::string> move_target;
    ASSERT_TRUE(v.hasValue());
    move_target = std::move(v);
    EXPECT_TRUE(move_target.hasValue());
}

TEST(Expected, similarMoveErrorAssignment)
{
    Expected<int, std::string> v { "abc" };
    Expected<float, std::string> move_target;
    ASSERT_FALSE(v.hasValue());
    move_target = std::move(v);
    EXPECT_FALSE(move_target.hasValue());
}

TEST(Expected, hasValue)
{
    const Expected<int, bool> v { 10 };
    EXPECT_TRUE(v.hasValue());

    const Expected<int, bool> e { true };
    EXPECT_FALSE(e.hasValue());
}

TEST(Expected, dereference)
{
    Expected<unsigned, float> v { 5U };
    EXPECT_EQ(*v, 5U);
}

TEST(Expected, value)
{
    const Expected<unsigned, double> v { 5U };
    const Expected<unsigned, double> e { 4.0 };

    EXPECT_TRUE(v.value().has_value());
    EXPECT_FALSE(e.value().has_value());
    EXPECT_TRUE(v.value().has_value());
    EXPECT_FALSE(e.value().has_value());
}

TEST(Expected, valueReference)
{
    Expected<int, bool> v { 10 };

    ASSERT_TRUE(v.hasValue());
    v.valueRef().value().get() = 5;

    EXPECT_EQ(v.value(), 5);
}

TEST(Expected, valueDereference)
{
    Expected<int, bool> v { 10 };

    ASSERT_TRUE(v.value().has_value());
    *v = 5;

    EXPECT_EQ(v.value(), 5);
}

TEST(Expected, constValue)
{
    const Expected<int, char*> v { 2 };
    const Expected<int, char*> e { nullptr };

    EXPECT_EQ(v.value(), 2);
    EXPECT_EQ(e.value(), std::nullopt);
    EXPECT_EQ(v.error(), std::nullopt);
    EXPECT_EQ(e.error(), nullptr);
}

TEST(Expected, valueRefNoCopy)
{
    const Expected<test::CopyAndMovable, char*> v {};
    ASSERT_TRUE(v.hasValue());

    auto moves_before = v->moved();
    std::ignore = v.valueRef();
    EXPECT_EQ(v->copied(), 0);
    EXPECT_EQ(v->moved(), moves_before);
}

TEST(Expected, valueDerefNoCopy)
{
    const Expected<test::CopyAndMovable, char*> v {};
    ASSERT_TRUE(v.hasValue());

    auto moves_before = v->moved();
    std::ignore = *v;
    EXPECT_EQ(v->copied(), 0);
    EXPECT_EQ(v->moved(), moves_before);
}

TEST(Expected, errorReference)
{
    Expected<int, bool> v { true };

    ASSERT_FALSE(v.hasValue());
    v.errorRef().value().get() = false;

    EXPECT_EQ(v.error(), false);
}

TEST(Expected, errorRefNoCopy)
{
    const Expected<int, test::CopyAndMovable> v { test::CopyAndMovable {} };
    ASSERT_FALSE(v.hasValue());

    auto moves_before = v.errorRef().value().get().moved();
    std::ignore = v.valueRef();
    EXPECT_EQ(v.errorRef().value().get().copied(), 0);
    EXPECT_EQ(v.errorRef().value().get().moved(), moves_before);
}

TEST(Expected, comparisonNotEqual)
{
    const Expected<int, const char*> v1 { 2 };
    const Expected<int, const char*> v2 { 3 };
    const Expected<int, const char*> e1 { "a" };
    const Expected<int, const char*> e2 { nullptr };

    const Expected<unsigned int, int> v3 { 2U };
    const Expected<unsigned int, int> v4 { 0U };
    const Expected<unsigned int, int> e3 { 0 };
    const Expected<unsigned int, int> e4 { 2 };
    const Expected<unsigned int, int> e5 { 3 };

    test::forEachCombination([](auto&& element_a, auto&& element_b, auto index_a, auto index_b) {
        if (index_a != index_b) {
            EXPECT_NE(element_a, element_b);
        }
    },
        v1, v2, e1, e2);
    test::forEachCombination([](auto&& element_a, auto&& element_b, auto index_a, auto index_b) {
        if (index_a != index_b) {
            EXPECT_NE(element_a, element_b);
        }
    },
        v3, v4, e3, e4, e5);
}

TEST(Expected, comparisonEqual)
{
    const Expected<const char*, int> v1 { "test" };
    const Expected<const char*, int> v2 { "test" };
    const Expected<const char*, int> v3 { v2 };

    const Expected<double, int> e1 { 10 };
    const Expected<double, int> e2 { 10 };

    test::forEachCombination([](auto&& a, auto&& b) {
        EXPECT_EQ(a, b);
    },
        v1, v2, v3);
    test::forEachCombination([](auto&& a, auto&& b) {
        EXPECT_EQ(a, b);
    },
        e1, e2);
}

TEST(Expected, andThenReturnRawValue)
{
    const Expected<const char*, int> v { "test" };

    auto result = v.andThen([]() { return 2.0; });

    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(result.value().value_or(0.0), 2.0);
}

TEST(Expected, andThenReturnValueInExpected)
{
    const Expected<const char*, int> v { "test" };

    auto result = v.andThen([]() -> Expected<double, int> { return 3.0; });

    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(result.value().value_or(0.0), 3.0);
}

TEST(Expected, andThenReturnError)
{
    const Expected<const char*, int> v { "test" };

    auto result = v.andThen([]() -> decltype(v) { return 1; });

    // static_assert(std::is_same_v<const decltype(result), decltype(v)>);

    EXPECT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().value_or(-1), 1);
}

TEST(Expected, andThenError)
{
    const Expected<const char*, int> e { 5 };

    auto result = e.andThen([]() { return "abc"; });

    EXPECT_FALSE(result.hasValue());
    EXPECT_EQ(result.error().value_or(-1), 5);
}
