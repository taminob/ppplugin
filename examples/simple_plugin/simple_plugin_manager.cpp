// g++ examples/simple_plugin_manager.cpp -I examples -lboost_program_options -lboost_filesystem
#include <filesystem>
#include <thread>

#include "plugin_manager.h"
#include "simple_plugin.h"

void printError(const std::string& plugin_name,
    ppplugin::PluginManager::PluginLoadingError error)
{
    using PluginError = ppplugin::PluginManager::PluginLoadingError;
    std::string_view error_string = error == PluginError::notFound ? "not found"
        : error == PluginError::loadingFailed
        ? "loading failed"
        : error == PluginError::none ? "no error"
                                     : "other error";
    std::cerr << "Unable to load '" << plugin_name << "' ('" << error_string
              << "')!" << std::endl;
}

int main()
{
    // setup manager - has to stay alive for as long as you want to use the
    // plugins
    ppplugin::PluginManager manager;
    // load first plugin and check for errors
    auto [plugin_a, error_a] = manager.loadPlugin<SimplePluginA>(
        std::filesystem::path { "./libsimple_plugin.so" }, "create_a");
    if (!plugin_a) {
        printError("simple_plugin_a", error_a);
    }
    // load second plugin and check for errors
    auto [plugin_b, error_b] = manager.loadPlugin<SimplePluginB>(
        std::filesystem::path { "./libsimple_plugin.so" }, "create_b");
    if (!plugin_b) {
        printError("simple_plugin_b", error_b);
    }

    plugin_a->initialize();
    plugin_b->initialize();

    std::thread thread_a([plugin = plugin_a]() { plugin->loop(); });
    std::thread thread_b([plugin = plugin_b]() { plugin->loop(); });
    thread_a.join();
    thread_b.join();
}
