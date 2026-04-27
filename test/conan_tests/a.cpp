#include <ppplugin/plugin_manager.h>

#include <iostream>

int main(int argc, char* /*argv*/[])
{
    auto unwrap_loading = [](auto&& load_result) {
        return std::forward<decltype(load_result)>(load_result)
            .andThen([](auto&& plugin) { return ppplugin::Plugin { std::forward<decltype(plugin)>(plugin) }; })
            .valueOr(ppplugin::NoopPlugin {});
    };

    std::vector<ppplugin::Plugin> plugins;
    plugins.push_back(unwrap_loading(ppplugin::ShellPlugin::load("a.sh")));
    plugins.push_back(unwrap_loading(ppplugin::ShellPlugin::load("a.lua")));
    plugins.push_back(unwrap_loading(ppplugin::PythonPlugin::load("a.py")));
    plugins.push_back(unwrap_loading(ppplugin::CPlugin::load("c.so")));
    plugins.push_back(unwrap_loading(ppplugin::CppPlugin::load("cpp.so")));

    for (auto&& plugin : plugins) {
        std::ignore = plugin.call<int>("main", static_cast<int>(argc)).valueOr(-1);
    }
}
