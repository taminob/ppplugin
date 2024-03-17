#include "plugin_manager.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <tuple>

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
        if (auto plugin = manager.loadLuaPlugin(plugin_path)) {
            auto [a, b, c] = plugin->call<std::tuple<int, int, int>>("values", 10).valueOr(std::make_tuple(-1, -1, -1));
            std::cout << "a, b, c: " << a << ", " << b << ", " << c << '\n';
        } else {
            std::cerr << "Unable to load plugin '" << plugin_path << "'\n";
            return 1;
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
