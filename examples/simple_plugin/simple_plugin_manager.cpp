#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "ppplugin/cpp/plugin.h"
#include "ppplugin/errors.h"
#include "ppplugin/plugin_manager.h"

#include "simple_plugin.h"

namespace {
template <typename ClassType>
std::thread initializeAndLoop(ppplugin::CppPlugin& plugin, const std::string& function_name)
{
    auto result = plugin.call<std::shared_ptr<ClassType>>(function_name);
    if (result) {
        auto& plugin = *result;
        plugin->initialize();
        return std::thread { [plugin]() {
            plugin->loop();
        } };
    }
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access); checked in previous if
    return std::thread { [function_name, error = *result.error()]() {
        std::cerr << "Unable to call '" << function_name
                  << "' ('" << ppplugin::codeToString(error.code()) << "')!\n";
    } };
}
} // namespace

int main(int argc, char* argv[])
{
    try {
        if (argc < 1) {
            return -1;
        }
        auto executable_dir = std::filesystem::path { argv[0] }.parent_path();
        auto library_path = executable_dir / "libsimple_plugin.so";
        // setup manager - has to stay alive for as long as you want to use the plugins
        ppplugin::GenericPluginManager<SimplePluginInterface> manager;
        // load plugin library and exit on error
        auto plugin_library = manager.loadCppPlugin(std::filesystem::path { library_path })
                                  .valueOrElse([](const auto& error) -> ppplugin::CppPlugin {
                                      std::cerr << "Unable to load plugin: " << ppplugin::codeToString(error);
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
