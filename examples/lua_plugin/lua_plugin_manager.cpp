#include "lua_script.h"
#include "plugin_manager.h"

#include <filesystem>
#include <thread>

int main()
{
    // TODO: add top-level try-catch block
    ppplugin::GenericPluginManager<ppplugin::LuaScript> manager;
    for (const auto& entry : std::filesystem::recursive_directory_iterator { "." }) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto& path = entry.path();
        if (path.extension() == ".lua") {
            auto [plugin, error] = manager.loadLuaPlugin<ppplugin::LuaScript>(path);
            if (plugin) {
                plugin->call("initialize");
                std::thread t { [plugin = plugin]() { plugin->call("loop", "2"); } };
                t.join();
            }
        }
    }
}
