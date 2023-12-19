#include "plugin_manager.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <thread>

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


        ppplugin::PluginManager manager;
        std::vector<ppplugin::Plugin> plugins;
        plugins.push_back(manager.loadCppPlugin(cpp_lib_path));
        plugins.push_back(manager.loadCPlugin(c_lib_path));
        plugins.push_back(manager.loadLuaPlugin(lua_lib_path));
        for (auto& plugin : plugins) {
            plugin.call<void>("initialize");
        }
        for (int counter {}; counter < std::numeric_limits<int>::max(); ++counter) {
            for (auto& plugin : plugins) {
                // explicit cast to int to avoid passing by reference
                plugin.call<void>("loop", static_cast<int>(counter));
                //std::ignore = plugin.call<>("loop", counter);
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
