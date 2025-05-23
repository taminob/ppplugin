#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <limits>
#include <thread>
#include <utility>
#include <vector>

#include "ppplugin/errors.h"
#include "ppplugin/noop_plugin.h"
#include "ppplugin/plugin.h"
#include "ppplugin/plugin_manager.h"

int main(int argc, char* argv[])
{
    if (argc < 1) {
        return -1;
    }
    try {
        auto executable_dir = std::filesystem::path { argv[0] }.parent_path();
        auto cpp_lib_path = executable_dir / "libcpp_plugin.so";
        auto c_lib_path = executable_dir / "libc_plugin.so";
        auto lua_lib_path = executable_dir / "lua_plugin.lua";
        auto python_lib_path = executable_dir / "python_plugin.py";
        auto non_existant_lib_path = executable_dir / "does_not_exist";

        ppplugin::PluginManager manager;
        std::vector<ppplugin::Plugin> plugins;
        if (auto plugin = manager.loadCppPlugin(cpp_lib_path)) {
            plugins.push_back(std::move(*plugin));
        }
        if (auto plugin = manager.loadCPlugin(c_lib_path)) {
            plugins.push_back(std::move(*plugin));
        }
        if (auto plugin = manager.loadLuaPlugin(lua_lib_path)) {
            plugins.push_back(std::move(*plugin));
        }
        if (auto plugin = manager.loadPythonPlugin(python_lib_path)) {
            plugins.push_back(std::move(*plugin));
        }
        plugins.push_back(manager.loadCPlugin(non_existant_lib_path)
                .andThen([](const auto& plugin) {
                    // convert to generic plugin
                    return ppplugin::Plugin { plugin };
                })
                // silently fail and use no-op plugin instead
                .valueOr(ppplugin::NoopPlugin {}));
        for (auto& plugin : plugins) {
            plugin.call<void>("initialize").valueOrElse([](const ppplugin::CallError& error) {
                std::cerr << "Initialize Error: " << error.what() << '\n';
            });
        }
        for (int counter {}; counter < std::numeric_limits<int>::max(); ++counter) {
            for (auto& plugin : plugins) {
                // explicit cast to int to avoid passing by reference
                plugin.call<void>("loop", static_cast<int>(counter)) // NOLINT(readability-redundant-casting)
                    .valueOrElse([](const ppplugin::CallError& error) {
                        std::cerr << "Loop Error: " << error.what() << '\n';
                    });
            }
            std::this_thread::sleep_for(std::chrono::milliseconds { 500 });
        }
    } catch (const std::exception& exception) {
        std::cerr << "A fatal error occurred: '" << exception.what() << "'\n";
        return 1;
    } catch (...) {
        std::cerr << "An unknown fatal error occurred!";
        return 1;
    }
    return 0;
}
