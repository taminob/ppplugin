#include "ppplugin/lua/plugin.h"

namespace ppplugin {
Expected<LuaPlugin, LoadError> LuaPlugin::load(const std::filesystem::path& plugin_library_path, bool auto_run)
{
    return LuaScript::load(plugin_library_path, auto_run)
        .andThen([](auto script) {
            LuaPlugin new_plugin { std::move(script) };
            return new_plugin;
        });
}
} // namespace ppplugin
