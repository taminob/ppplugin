#include "expected.h"
#include "test/test_types.h"
#include "test_helper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ppplugin;

TEST(Expected, valueConstruction)
{
    [[maybe_unused]] Expected<int, bool> v1 { 0 };
    [[maybe_unused]] Expected<bool, int> v2 { false };
    [[maybe_unused]] Expected<test::NonDefaultConstructible, int> v3 {
        test::NonDefaultConstructible { 1, 2, 3 }
    };
    [[maybe_unused]] Expected<test::NonCopyable, int> v4 { test::NonCopyable {} };
}

TEST(Expected, valueNonTrivialDestruction)
{
    testing::MockFunction<std::function<void()>> f;
    EXPECT_CALL(f, Call()).Times(1);
    [[maybe_unused]] Expected<test::NonTrivialDestructible, bool> v {
        test::NonTrivialDestructible { f.AsStdFunction() }
    };
}

TEST(Expected, valueTrivialDestruction)
{
    [[maybe_unused]] Expected<int, test::NonTrivialDestructible> v { 1 };
}

TEST(Expected, errorNonTrivialDestruction)
{
    testing::MockFunction<std::function<void()>> f;
    EXPECT_CALL(f, Call()).Times(1);
    [[maybe_unused]] Expected<bool, test::NonTrivialDestructible> e {
        test::NonTrivialDestructible { f.AsStdFunction() }
    };
}

TEST(Expected, errorTrivialDestruction)
{
    [[maybe_unused]] Expected<test::NonTrivialDestructible, bool> e { false };
}

TEST(Expected, trivialDestruction)
{
    [[maybe_unused]] Expected<int, bool> e { false };
}

TEST(Expected, simpleCopyValueConstructor)
{
    Expected<test::CopyAndMovable, double> v { test::CopyAndMovable {} };
    auto copy_target { v };
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(copy_target.hasValue());
    EXPECT_EQ(copy_target->copied(), 1);
}

TEST(Expected, simpleCopyErrorConstructor)
{
    Expected<int, test::CopyAndMovable> v { test::CopyAndMovable {} };
    auto copy_target { v };
    ASSERT_FALSE(v.hasValue());
    EXPECT_FALSE(copy_target.hasValue());
    EXPECT_EQ(copy_target.errorRef()->get().copied(), 1);
}

TEST(Expected, similarCopyValueConstructor)
{
    Expected<double, unsigned int> v { 1.0 };
    Expected<float, double> copy_target { v };
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(copy_target.hasValue());
}

TEST(Expected, similarCopyErrorConstructor)
{
    Expected<int, unsigned int> v { 32U };
    Expected<float, double> copy_target { v };
    ASSERT_FALSE(v.hasValue());
    EXPECT_FALSE(copy_target.hasValue());
}

TEST(Expected, simpleCopyValueAssignment)
{
    Expected<test::CopyAndMovable, double> v { test::CopyAndMovable {} };
    decltype(v) copy_target;
    copy_target = v;
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(copy_target.hasValue());
    EXPECT_EQ(copy_target->copied(), 1);
}

TEST(Expected, simpleCopyErrorAssignment)
{
    Expected<int, test::CopyAndMovable> v { test::CopyAndMovable {} };
    decltype(v) copy_target;
    copy_target = v;
    ASSERT_FALSE(v.hasValue());
    EXPECT_FALSE(copy_target.hasValue());
    EXPECT_EQ(copy_target.errorRef()->get().copied(), 1);
}

TEST(Expected, similarCopyValueAssignment)
{
    Expected<bool, unsigned int> v { true };
    Expected<int, double> copy_target;
    copy_target = v;
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(copy_target.hasValue());
}

TEST(Expected, similarCopyErrorAssignment)
{
    Expected<int, const char*> v { "abc" };
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
    Expected<bool, const std::string> move_target { std::move(v) };
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(move_target.hasValue());
}

TEST(Expected, similarMoveErrorConstructor)
{
    Expected<float, std::string> v { "abc" };
    ASSERT_FALSE(v.hasValue());
    Expected<float, std::optional<std::string>> move_target { std::move(v) };
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
    move_target = std::move(v);
    ASSERT_TRUE(v.hasValue());
    EXPECT_TRUE(move_target.hasValue());
}

TEST(Expected, similarMoveErrorAssignment)
{
    Expected<int, std::string> v { "abc" };
    Expected<float, std::string> move_target;
    move_target = std::move(v);
    ASSERT_FALSE(v.hasValue());
    EXPECT_FALSE(move_target.hasValue());
}

TEST(Expected, hasValue)
{
    Expected<int, bool> v { 10 };
    EXPECT_TRUE(v.hasValue());

    Expected<int, bool> e { true };
    EXPECT_FALSE(e.hasValue());
}

TEST(Expected, dereference)
{
    Expected<unsigned, float> v { 5U };
    EXPECT_EQ(*v, 5U);
}

TEST(Expected, value)
{
    Expected<unsigned, double> v { 5U };
    Expected<unsigned, double> e { 4.0 };

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
