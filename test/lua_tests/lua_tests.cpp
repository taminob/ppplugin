#include "test_helper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ppplugin/lua/plugin.h>

class LuaTest : public testing::Test {
protected:
    void SetUp() override
    {
        auto load_result = ppplugin::LuaPlugin::load("./lua_tests/test.lua");
        ASSERT_TRUE(load_result.hasValue());

        plugin = std::make_unique<ppplugin::LuaPlugin>(std::move(load_result.valueRef()->get()));
    }

protected:
    std::unique_ptr<ppplugin::LuaPlugin> plugin;
};

TEST_F(LuaTest, callFunctionWithDifferentArguments)
{
    auto result_1 = plugin->call<bool>("accept_number_string_bool", 12, "abc", true);
    auto result_2 = plugin->call<bool>("accept_number_string_bool", 2.0, "def", false);

    EXPECT_TRUE(result_1.valueOr(false));
    EXPECT_TRUE(result_2.valueOr(false));
}

TEST_F(LuaTest, callFunctionWithWrongArguments)
{
    auto result_1 = plugin->call<bool>("accept_number_string_bool", "def", false);
    auto result_2 = plugin->call<bool>("accept_number_string_bool", 1, 2, 3, 4);

    ASSERT_TRUE(result_1.hasValue()) << ppplugin::test::errorOutput(result_1);
    ASSERT_TRUE(result_2.hasValue()) << ppplugin::test::errorOutput(result_2);
    EXPECT_FALSE(result_1.valueOr(true));
    EXPECT_FALSE(result_2.valueOr(true));
}

TEST_F(LuaTest, callFunctionWithArrayArgument)
{
    auto result = plugin->call<std::string>("serialize_array", std::vector<std::string> { "a", "b", "c" });

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(""), "1:a,2:b,3:c,");
}

TEST_F(LuaTest, callFunctionWithMapArgument)
{
    const auto example_map = std::map<std::string, int> { { "a", 1 }, { "b", 2 }, { "c", 3 }, { "d", 0 } };

    auto result_a = plugin->call<int>("access_table", example_map, "a");
    auto result_c = plugin->call<int>("access_table", example_map, "c");
    auto result_d = plugin->call<int>("access_table", example_map, "d");

    EXPECT_EQ(result_a.valueOr(-1), 1);
    EXPECT_EQ(result_c.valueOr(-1), 3);
    EXPECT_EQ(result_d.valueOr(-1), 0);
}

TEST_F(LuaTest, callFunctionWithArrayResult)
{
    auto result = plugin->call<std::vector<std::string>>("return_array");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_THAT(result.valueOr(std::vector<std::string> {}), testing::ElementsAre("a", "b", "c"));
}

TEST_F(LuaTest, callFunctionWithMapResult)
{
    using ResultType = std::map<std::string, int>;
    const ResultType expected { { "a", 1 }, { "b", 2 }, { "c", 3 }, { "d", 4 } };

    auto result = plugin->call<ResultType>("return_map");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(ResultType {}), expected);
}

TEST_F(LuaTest, nestedTable)
{
    using ResultType = std::map<std::vector<std::string>, std::map<int, int>>;
    const ResultType value = {
        { { "a", "b" }, { { 1, 2 }, { 0, 0 } } },
        { { "c" }, {} },
        { { "x" }, { { -1, 5 } } },
    };

    auto result = plugin->call<ResultType>("identity", value);

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(ResultType {}), value);
}
