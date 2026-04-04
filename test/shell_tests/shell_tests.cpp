#include "test_helper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ppplugin/shell/plugin.h>

class ShellTest : public testing::Test {
protected:
    void SetUp() override
    {
        auto load_result = ppplugin::ShellPlugin::load("./shell_tests/test.sh");
        ASSERT_TRUE(load_result.hasValue());

        plugin = std::make_unique<ppplugin::ShellPlugin>(std::move(load_result.valueRef()->get()));
    }

protected:
    std::unique_ptr<ppplugin::ShellPlugin> plugin;
};

TEST_F(ShellTest, getEnvironmentVariable)
{
    auto result = plugin->global<std::string>("env_var");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(""), "abc");
}

TEST_F(ShellTest, setEnvironmentVariable)
{
    auto original_result = plugin->global<std::string>("env_var");
    auto set_result = plugin->global("env_var", "xyz");
    auto changed_result = plugin->global<std::string>("env_var");

    ASSERT_TRUE(original_result.hasValue()) << ppplugin::test::errorOutput(original_result);
    ASSERT_TRUE(set_result.hasValue()) << ppplugin::test::errorOutput(set_result);
    ASSERT_TRUE(changed_result.hasValue()) << ppplugin::test::errorOutput(changed_result);

    EXPECT_NE(original_result.valueOr(""), changed_result.valueOr(""));
    EXPECT_EQ(changed_result.valueOr(""), "xyz");
}

TEST_F(ShellTest, callFunction)
{
    auto result = plugin->call<void>("print_first", "qwertz");

    EXPECT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
}

TEST_F(ShellTest, callFunctionWithStringResult)
{
    auto result = plugin->call<std::string>("print_first", "qwerty");

    EXPECT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.value().value(), "qwerty\n");
}
