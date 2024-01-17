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
    auto library_path = executable_dir / "libsimple_plugin.so";
    // setup manager - has to stay alive for as long as you want to use the
    // plugins
    ppplugin::GenericPluginManager<SimplePluginInterface> manager;
    // load plugin library and exit on error
    auto plugin_library = manager.loadCppPlugin(std::filesystem::path { library_path }).valueOrElse([](const auto& error) -> ppplugin::CppPlugin {
        printError("simple_plugin_a", error);
        std::exit(1); // NOLINT(concurrency-mt-unsafe)
    });
    auto plugin_a = plugin_library.call<std::shared_ptr<SimplePluginA>>("create_a");
    auto plugin_b = plugin_library.call<std::shared_ptr<SimplePluginInterface>>("create_b");
    if (!plugin_b) {
        std::cerr << "Unable to call 'create_b'" << std::endl;
        return 1;
    }

    plugin_a->initialize();
    plugin_b->initialize();

    std::thread thread_a([plugin = plugin_a]() { plugin->loop(); });
    std::thread thread_b([plugin = plugin_b]() { plugin->loop(); });
    thread_a.join();
    thread_b.join();
}
