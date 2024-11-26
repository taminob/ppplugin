#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>

#include "ppplugin/cpp/plugin.h"
#include "ppplugin/errors.h"
#include "ppplugin/plugin_manager.h"

#include "simple_plugin.h"

void printError(std::string_view function_name, const ppplugin::CallError& error)
{
    const std::string_view error_string = error == ppplugin::CallError::Code::symbolNotFound ? "not found"
        : error == ppplugin::CallError::Code::notLoaded                                      ? "not loaded"
        : error == ppplugin::CallError::Code::unknown                                        ? "unknown error"
                                                                                             : "other error";
    std::cerr << "Unable to call '" << function_name << "' ('" << error_string
              << "')!\n";
}

void printError(std::string_view plugin_name, ppplugin::LoadError error)
{
    using ppplugin::LoadError;
    const std::string_view error_string = error == LoadError::fileNotFound ? "not found"
        : error == LoadError::fileInvalid                                  ? "invalid"
        : error == LoadError::unknown                                      ? "unknown error"
                                                                           : "other error";
    std::cerr << "Unable to load '" << plugin_name << "' ('" << error_string
              << "')!\n";
}

template <typename ClassType>
std::thread initializeAndLoop(ppplugin::CppPlugin& plugin, const std::string& function_name)
{
    auto plugin_a = plugin.call<std::shared_ptr<ClassType>>(function_name);
    if (plugin_a) {
        auto& plugin = *plugin_a;
        plugin->initialize();
        return std::thread { [plugin]() {
            plugin->loop();
        } };
    }
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access); checked in previous if
    return std::thread { [function_name, error = *plugin_a.error()]() {
        printError(function_name, error);
    } };
}

int main(int argc, char* argv[])
{
    try {
        if (argc < 1) {
            return -1;
        }
        auto executable_dir = std::filesystem::path { argv[0] }.parent_path();
        auto library_path = executable_dir / "libsimple_plugin.so";
        // setup manager - has to stay alive for as long as you want to use the
        // plugins
        ppplugin::GenericPluginManager<SimplePluginInterface> manager;
        // load plugin library and exit on error
        auto plugin_library = manager.loadCppPlugin(std::filesystem::path { library_path }).valueOrElse([](const auto& error) -> ppplugin::CppPlugin {
            printError("simple_plugin_a", error);
            std::exit(1); // NOLINT(concurrency-mt-unsafe)
        });

        auto thread_a = initializeAndLoop<SimplePluginA>(plugin_library, "create_a");
        auto thread_b = initializeAndLoop<SimplePluginInterface>(plugin_library, "create_b");
        thread_a.join();
        thread_b.join();
    } catch (const std::exception& exception) {
        std::cerr << "A fatal error occurred: '" << exception.what() << "'\n";
        return 1;
    } catch (...) {
        std::cerr << "An unknown fatal error occurred!\n";
        return 1;
    }
    return 0;
}
