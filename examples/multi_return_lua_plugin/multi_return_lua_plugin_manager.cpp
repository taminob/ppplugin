#include "plugin_manager.h"

#include <filesystem>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    try {
        if (argc < 1) {
            return -1;
        }
        // path to plugins can be passed via command line;
        // if no path was specified, the location of the executable is used instead
        std::filesystem::path plugin_path;
        if (argc < 2) {
            plugin_path = std::filesystem::path { argv[0] }.parent_path();
        } else {
            plugin_path = std::filesystem::path { argv[1] };
        }
        plugin_path /= "plugin.lua";

        ppplugin::PluginManager manager;
        auto plugin = manager.loadLuaPlugin(plugin_path);
        if (plugin) {
            auto [a, b, c] = plugin.call<std::tuple<int, int, int>>("values", 10);
            std::cout << "a, b, c: " << a << ", " << b << ", " << c << std::endl;
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