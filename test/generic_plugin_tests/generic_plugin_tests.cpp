#include "test_helper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ppplugin/plugin.h>

template <typename T, const char* path>
struct TestParameter {
    using PluginType = T;
    static constexpr auto PLUGIN_PATH = path;
};

template <typename T>
class GenericPluginTests : public testing::TestWithParam<T> {
public:
    void SetUp() override
    {
    }

    /**
     * Load plugin and return it on success.
     * On failure, an EXPECT statement will fail and a NoopPlugin is returned.
     */
    template <typename P>
    static ppplugin::Plugin load(const std::filesystem::path& plugin_path)
    {
        return P::load(plugin_path)
            .andThen([](auto&& plugin) { return ppplugin::Plugin { std::forward<decltype(plugin)>(plugin) }; })
            .valueOrElse([](auto&& error) -> ppplugin::Plugin {
                EXPECT_TRUE(false) << error;
                return ppplugin::NoopPlugin {};
            });
    }
};
TYPED_TEST_SUITE_P(GenericPluginTests);

const char python_plugin_path[] = "./generic_plugin_tests/test.py";
const char lua_plugin_path[] = "./generic_plugin_tests/test.lua";
const char shell_plugin_path[] = "./generic_plugin_tests/test.sh";

using Types = testing::Types<
    TestParameter<ppplugin::PythonPlugin, python_plugin_path>,
    TestParameter<ppplugin::LuaPlugin, lua_plugin_path>,
    TestParameter<ppplugin::ShellPlugin, shell_plugin_path>>;

INSTANTIATE_TYPED_TEST_SUITE_P(GenericPluginTest, GenericPluginTests, Types);

TYPED_TEST_P(GenericPluginTests, loadingSuccess)
{
    auto plugin = decltype(*this)::template load<TypeParam>("./generic_plugin_tests/test.py");
    EXPECT_TRUE(static_cast<bool>(plugin));
}
