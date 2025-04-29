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

TEST_F(PythonTest, callFunctionWithDict)
{
    auto result = plugin->call<int>("accept_dict", std::map<std::string, int> {
                                                       { "", 1 },
                                                       { "a", 2 },
                                                       { "b", 3 },
                                                       { "c", 4 },
                                                   },
        "c");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(0), 4);
}

TEST_F(PythonTest, callFunctionWithList)
{
    auto result = plugin->call<std::string>("accept_list", std::vector<char> { 'a', 'b', 'c' });

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(""), "a,b,c,");
}

TEST_F(PythonTest, nestedDict)
{
    using ResultType = std::map<std::string, std::vector<std::map<int, int>>>;
    const ResultType expected = {
        { "", {} },
        { "a", { { { 1, 1 }, { 2, 2 } }, { { 0, 0 } } } },
        { "b", { { { 1, 1 } } } }
    };

    auto result = plugin->call<ResultType>("identity", expected);

    EXPECT_EQ(result.valueOr(ResultType {}), expected);
}

TEST_F(PythonTest, getGlobalString)
{
    auto result = plugin->global<std::string>("string_global");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(""), "abc");
}

TEST_F(PythonTest, getGlobalChar)
{
    auto result = plugin->global<char>("char_global");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(' '), 'a');
}

TEST_F(PythonTest, getGlobalBool)
{
    auto result = plugin->global<bool>("bool_global");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(true), false);
}

TEST_F(PythonTest, getGlobalInt)
{
    auto result = plugin->global<int>("int_global");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(0), 12);
}

TEST_F(PythonTest, getGlobalFloat)
{
    auto result = plugin->global<float>("float_global");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_DOUBLE_EQ(result.valueOr(0.0), 42.0);
}

TEST_F(PythonTest, getGlobalList)
{
    using ResultType = std::vector<std::vector<int>>;
    const ResultType expected { {}, { 0 }, { 0, 0 } };

    auto result = plugin->global<ResultType>("list_global");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(ResultType {}), expected);
}

TEST_F(PythonTest, getGlobalDict)
{
    using ResultType = std::map<std::string, int>;
    const ResultType expected { { "a", 1 }, { "b", 2 } };

    auto result = plugin->global<ResultType>("dict_global");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(result.valueOr(ResultType {}), expected);
}

TEST_F(PythonTest, setGlobalDict)
{
    using ResultType = std::map<std::string, int>;
    const ResultType expected { { "x", 10 }};

    auto set_result = plugin->global("dict_global", expected);

    EXPECT_TRUE(set_result.hasValue()) << ppplugin::test::errorOutput(set_result);

    auto result = plugin->call<ResultType>("get_global", "dict_global");

    ASSERT_TRUE(result.hasValue()) << ppplugin::test::errorOutput(result);
    EXPECT_EQ(*result, expected);
}
