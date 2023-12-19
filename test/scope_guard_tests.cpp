#include "detail/scope_guard.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ppplugin::detail;

TEST(ScopeGuard, regularUsage)
{
    testing::MockFunction<std::function<void()>> f;
    EXPECT_CALL(f, Call()).Times(1);

    ScopeGuard guard { f.AsStdFunction() };
}

TEST(ScopeGuard, cancel)
{
    testing::MockFunction<std::function<void()>> f;
    EXPECT_CALL(f, Call()).Times(0);

    ScopeGuard guard { f.AsStdFunction() };
    guard.cancel();
}

TEST(ScopeGuard, moveSourceCalled)
{
    bool guard_called { false };
    {
        ScopeGuard first_guard { []() {} };
        {
            ScopeGuard second_guard {
                [&guard_called]() {
                    guard_called = true;
                }
            };
            first_guard = std::move(second_guard);
        }
        EXPECT_FALSE(guard_called);
    }
    EXPECT_TRUE(guard_called);
}

TEST(ScopeGuard, moveTargetCalled)
{
    bool guard_called { false };
    {
        ScopeGuard first_guard { [&guard_called]() {
            guard_called = true;
        } };
        {
            ScopeGuard second_guard { []() {} };
            first_guard = std::move(second_guard);
            EXPECT_TRUE(guard_called);
        }
    }
}

TEST(ScopeGuard, call)
{
    testing::MockFunction<std::function<void()>> f;
    EXPECT_CALL(f, Call()).Times(1);

    ScopeGuard guard { f.AsStdFunction() };
    guard.call();
}
