#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "ppplugin/plugin_manager.h"
#include "ppplugin/python/plugin.h"

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
        ppplugin::GenericPluginManager<ppplugin::PythonPlugin> manager;
        std::vector<std::thread> threads;
        int plugin_number {};

        // recursively traverse filesystem to find scripts
        const std::filesystem::recursive_directory_iterator dir_iterator { plugin_dir };
        for (const auto& entry : dir_iterator) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const auto& path = entry.path();
            // only load files ending with ".py" and execute in separate thread
            if (path.extension() == ".py") {
                auto plugin = manager.loadPythonPlugin(path);
                if (plugin) {
                    threads.emplace_back([plugin = std::move(*plugin), plugin_number = plugin_number++]() mutable {
                        // ignore calling errors
                        std::ignore = plugin.call<void>("initialize", plugin_number);
                        std::ignore = plugin.call<void>("loop", std::to_string(plugin_number));
                    });
                } else {
                    std::cerr << "Failed to load " << path << '\n';
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
