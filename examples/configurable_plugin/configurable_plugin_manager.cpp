#include "configurable_plugin.h"
#include "plugin_manager.h"

int main(int argc, char* argv[])
{
    // TODO: add top-level try-catch block
    if (argc < 1) {
        return -1;
    }
    auto executable_dir = std::filesystem::path { argv[0] }.parent_path();
    auto library = executable_dir / "libconfigurable_plugin.so";
    ppplugin::GenericPluginManager<ConfigurablePluginInterface> manager;
    PluginConfiguration config_1 { std::chrono::milliseconds { 600 }, "1" };
    auto [plugin_1, error_1] = manager.loadCppPlugin<ConfigurablePluginA>(library, "create_a", config_1);
    PluginConfiguration config_2 { std::chrono::milliseconds { 200 }, "2" };
    auto [plugin_2, error_2] = manager.loadCppPlugin<ConfigurablePluginA>(library, "create_a", config_2);

    if (!plugin_1 || !plugin_2) {
        return -1;
    }

    std::atomic<bool> stop_signal{false};
    std::thread thread_1([plugin = plugin_1, &stop_signal]() { plugin->loop(stop_signal); });
    std::thread thread_2([plugin = plugin_2, &stop_signal]() { plugin->loop(stop_signal); });
    std::this_thread::sleep_for(std::chrono::seconds{5});
    stop_signal.store(true);
    thread_1.join();
    thread_2.join();
}
