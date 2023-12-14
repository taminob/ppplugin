#include "configurable_plugin.h"
#include "plugin_manager.h"

int main()
{
    ppplugin::GenericPluginManager<ConfigurablePluginInterface> manager;
    PluginConfiguration config_1 { std::chrono::milliseconds { 600 }, "1" };
    auto [plugin_1, error_1] = manager.loadPlugin<ConfigurablePluginA>("./libconfigurable_plugin.so", "create_a", config_1);
    PluginConfiguration config_2 { std::chrono::milliseconds { 200 }, "2" };
    auto [plugin_2, error_2] = manager.loadPlugin<ConfigurablePluginA>("./libconfigurable_plugin.so", "create_a", config_2);

    if (!plugin_1 || !plugin_2) {
        return -1;
    }
    std::thread plugin_thread([plugin = plugin_2]() { plugin->loop(); });
    plugin_1->loop();
    plugin_thread.join();
}
