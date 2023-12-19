#include "detail/scope_guard.h"
#include "expected.h"
#include "test/test_types.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ppplugin;

TEST(Expected, valueConstruction)
{
    Expected<int, bool> v1 { 0 };
    Expected<bool, int> v2 { false };
    Expected<test::NonDefaultConstructible, int> v3 { test::NonDefaultConstructible { 1, 2, 3 } };
    Expected<test::NonCopyable, int> v4 { test::NonCopyable {} };
}

TEST(Expected, valueNonTrivialDestruction)
{
    testing::MockFunction<std::function<void()>> f;
    EXPECT_CALL(f, Call()).Times(1);
    Expected<test::NonTrivialDestructible, bool> v {
        test::NonTrivialDestructible { f.AsStdFunction() }
    };
}

TEST(Expected, valueTrivialDestruction)
{
    Expected<int, test::NonTrivialDestructible> v { 1 };
}

TEST(Expected, errorNonTrivialDestruction)
{
    testing::MockFunction<std::function<void()>> f;
    EXPECT_CALL(f, Call()).Times(1);
    Expected<bool, test::NonTrivialDestructible> e {
        test::NonTrivialDestructible { f.AsStdFunction() }
    };
}

TEST(Expected, errorTrivialDestruction)
{
    Expected<test::NonTrivialDestructible, bool> e { false };
}

TEST(Expected, trivialDestruction)
{
    Expected<int, bool> e { false };
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

    ASSERT_TRUE(v.value().has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    v.value().value().get() = 5;

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
