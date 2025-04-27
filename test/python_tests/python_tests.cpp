#include "test_helper.h"

#include <gtest/gtest.h>

#include <ppplugin/python/plugin.h>

class PythonTest : public testing::Test {
protected:
    void SetUp() override
    {
        auto load_result = ppplugin::PythonPlugin::load("./python_tests/test.py");
        ASSERT_TRUE(load_result.hasValue());

        plugin = std::make_unique<ppplugin::PythonPlugin>(std::move(load_result.valueRef()->get()));
    }

protected:
    std::unique_ptr<ppplugin::PythonPlugin> plugin;
};

TEST_F(PythonTest, callFunctionWithDifferentArguments)
{
    auto result_1 = plugin->call<bool>("accept_int_string_bool_float", 12, "abc", true, 2.0F);
    auto result_2 = plugin->call<bool>("accept_int_string_bool_float", 2UL, "def", false, 12.0);

    EXPECT_TRUE(result_1.valueOr(false));
    EXPECT_TRUE(result_2.valueOr(false));
}

TEST_F(PythonTest, callFunctionWithWrongArguments)
{
    auto result_1 = plugin->call<bool>("accept_int_string_bool_float", "def", false, false, false);
    auto result_2 = plugin->call<bool>("accept_int_string_bool_float", 1, 2, 3, 4);

    // successful since Python is dynamically typed and can accept any argument type
    ASSERT_TRUE(result_1.hasValue()) << ppplugin::test::errorOutput(result_1);
    ASSERT_TRUE(result_2.hasValue()) << ppplugin::test::errorOutput(result_2);
    EXPECT_FALSE(result_1.valueOr(true));
    EXPECT_FALSE(result_2.valueOr(true));
}
