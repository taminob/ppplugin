#include <exception>
#include <filesystem>
#include <iostream>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "ppplugin/lua/plugin.h"
#include "ppplugin/plugin_manager.h"

int main(int argc, char* argv[])
{
    try {
        if (argc < 1) {
            return -1;
        }
        // path to plugins can be passed via command line;
        // if no path was specified, the location of the executable is used instead
        std::filesystem::path plugin_dir;
        if (argc < 2) {
            plugin_dir = std::filesystem::path { argv[0] }.parent_path();
        } else {
            plugin_dir = std::filesystem::path { argv[1] };
        }
        ppplugin::GenericPluginManager<ppplugin::LuaPlugin> manager;
        std::vector<std::thread> threads;

        // recursively traverse filesystem to find scripts
        const std::filesystem::recursive_directory_iterator dir_iterator { plugin_dir };
        for (const auto& entry : dir_iterator) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const auto& path = entry.path();
            // only load files ending with ".lua" and execute in separate thread
            if (path.extension() == ".lua") {
                if (auto plugin = manager.loadLuaPlugin(path)) {
                    threads.emplace_back([plugin = std::move(*plugin), plugin_number = threads.size()]() mutable {
                        // data race due to shared resource "stdout",
                        // but Lua plugins are otherwise thread-safe
                        std::ignore = plugin.call<void>("initialize");
                        std::ignore = plugin.call<void>("loop", plugin_number);
                    });
                }
            }
        }

        for (auto& thread : threads) {
            thread.join();
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
