// g++ examples/simple_plugin_manager.cpp -I examples -lboost_program_options -lboost_filesystem
#include <filesystem>
#include <thread>

#include "plugin_manager.h"
#include "simple_plugin.h"

template <typename ErrorType>
void printError(const std::string& plugin_name,
    ErrorType error)
{
    std::string_view error_string = error == ErrorType::notFound ? "not found"
        : error == ErrorType::loadingFailed
        ? "loading failed"
        : error == ErrorType::none ? "no error"
                                   : "other error";
    std::cerr << "Unable to load '" << plugin_name << "' ('" << error_string
              << "')!" << std::endl;
}

int main(int argc, char* argv[])
{
    // TODO: add top-level try-catch block
    if (argc < 1) {
        return -1;
    }
    auto executable_dir = std::filesystem::path { argv[0] }.parent_path();
    auto library = executable_dir / "libsimple_plugin.so";
    // setup manager - has to stay alive for as long as you want to use the
    // plugins
    ppplugin::GenericPluginManager<SimplePluginInterface> manager;
    // load first plugin and check for errors
    auto [plugin_a, error_a] = manager.loadCppPlugin<SimplePluginA>(
        std::filesystem::path { library }, "create_a");
    if (!plugin_a) {
        printError("simple_plugin_a", error_a);
    }
    // load second plugin and check for errors
    auto [plugin_b, error_b] = manager.loadCppPlugin<SimplePluginInterface>(
        std::filesystem::path { library }, "create_b");
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
