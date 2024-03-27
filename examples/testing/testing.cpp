#include "plugin_manager.h"
#include "python/plugin.h"
#include "python/python_interpreter.h"

#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

int main(int argc, char* argv[])
{
    std::vector<std::thread> threads;
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
        ppplugin::GenericPluginManager<ppplugin::PythonPlugin> const manager;
        int plugin_number {};
        std::vector<ppplugin::PythonPlugin> const plugins;

        // checkout: https://stackoverflow.com/questions/26061298/python-multi-thread-multi-interpreter-c-api#26570708
        // recursively traverse filesystem to find scripts
        const std::filesystem::recursive_directory_iterator dir_iterator { plugin_dir };
        for (const auto& entry : dir_iterator) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const auto& path = entry.path();
            // only load files ending with ".lua" and execute in separate thread
            if (path.extension() == ".py") {
                ppplugin::PythonInterpreter interpreter;
                if (auto error = interpreter.load(path.string())) {
                    std::cerr << "failed to load " << path << '\n';
                    // return -1;
                }
                threads.emplace_back([manager, path, plugin_number = plugin_number++, interpreter = std::move(interpreter)]() mutable {
                    interpreter.call<void>("initialize", std::hash<std::thread::id> {}(std::this_thread::get_id()));
                    interpreter.call<void>("loop", std::hash<std::thread::id> {}(std::this_thread::get_id()));
                });
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
