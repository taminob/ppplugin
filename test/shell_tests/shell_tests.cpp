#include "test_helper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ppplugin/shell/plugin.h>

class ShellTest : public testing::Test {
protected:
    void SetUp() override
    {
        auto load_result = ppplugin::ShellPlugin::load("./shell_tests/test.sh");
        ASSERT_TRUE(load_result.hasValue()) << ppplugin::test::errorOutput(load_result);

        plugin = std::make_unique<ppplugin::ShellPlugin>(std::move(load_result.valueRef()->get()));
    }

protected:
    std::unique_ptr<ppplugin::ShellPlugin> plugin;
};

TEST(ShellTestStandalone, failToLoadNonexistentFile)
{
    auto plugin = ppplugin::ShellPlugin::load("./path/that/does/not/exist.sh");
    EXPECT_FALSE(plugin);
}

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
    auto result = plugin->call<void>("print_all_args");

    EXPECT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
}

TEST_F(ShellTest, callFunctionWithStringResult)
{
    auto result = plugin->call<std::string>("print_first", "qwerty");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.value().value(), "qwerty\n");
}

TEST_F(ShellTest, moveToOtherThread)
{
    auto result = plugin->global("some_var", "some_value");
    EXPECT_TRUE(result) << ppplugin::test::errorOutput(result);

    std::thread thread { [moved_plugin = std::move(*plugin)]() mutable {
        auto result = moved_plugin.global<std::string>("some_var");
        EXPECT_TRUE(result) << ppplugin::test::errorOutput(result);
        EXPECT_EQ(result.value().value(), "some_value");
    } };
    thread.join();
}

TEST_F(ShellTest, callFunctionWithStringArguments)
{
    const char* first_arg = "first_arg";
    const std::string second_arg = "second arg";
    const std::string_view third_arg = "third'arg\\'";
    auto result = plugin->call<std::string>("print_all_args",
        first_arg,
        second_arg,
        third_arg,
        "fourth\"arg",
        std::string { "fifth\narg" },
        std::string_view { "sixth\\$!arg" });

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.value().value(), "first_arg second arg third'arg\\' fourth\"arg fifth\narg sixth\\$!arg\n");
}

TEST_F(ShellTest, callFunctionWithNonStringArguments)
{
    const std::vector<std::string> first_arg { "a", "\'" };
    int second_arg { 123 };
    std::uint64_t third_arg { 456 };
    double fourth_arg { 0.2 };
    auto result = plugin->call<std::string>("print_all_args",
        first_arg,
        second_arg,
        third_arg,
        fourth_arg);

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.value().value(), "a ' 123 456 0.2\n");
}

TEST_F(ShellTest, callFunctionModifyEnvironment)
{
    auto before = plugin->global<std::string>("env_var");
    auto call_result = plugin->call<void>("change_environment");
    auto after = plugin->global<std::string>("env_var");

    ASSERT_TRUE(before.hasValue()) << ppplugin::test::errorOutput(before);
    ASSERT_TRUE(call_result.hasValue()) << ppplugin::test::errorOutput(call_result);
    ASSERT_TRUE(after.hasValue()) << ppplugin::test::errorOutput(after);

    EXPECT_EQ(before.valueOr(""), "abc");
    EXPECT_EQ(after.valueOr(""), "xyz");
}

TEST_F(ShellTest, callFunctionExitShell)
{
    auto result = plugin->call<void>("exit_shell");
    // ending the shell in a function is considered a failure
    EXPECT_FALSE(result.hasValue());
    EXPECT_FALSE(*plugin);

    auto later_result = plugin->call<void>("print_first", "123");
    // subsequent calls will all fail
    EXPECT_FALSE(later_result.hasValue());
}
