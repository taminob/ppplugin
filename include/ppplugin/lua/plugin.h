#ifndef PPPLUGIN_LUA_PLUGIN_H
#define PPPLUGIN_LUA_PLUGIN_H

#include "lua_script.h"
#include "ppplugin/errors.h"

#include <string>

namespace ppplugin {
class LuaPlugin {
public:
    static Expected<LuaPlugin, LoadError> load(
        const std::filesystem::path& plugin_library_path, bool auto_run = true)
    {
        return LuaScript::load(plugin_library_path, auto_run)
            .andThen([](auto script) {
                LuaPlugin new_plugin { std::move(script) };
                return new_plugin;
            });
    }

    ~LuaPlugin() = default;
    LuaPlugin(const LuaPlugin&) = delete;
    LuaPlugin(LuaPlugin&&) = default;
    LuaPlugin& operator=(const LuaPlugin&) = delete;
    LuaPlugin& operator=(LuaPlugin&&) = default;

    [[nodiscard]] auto& raw() { return script_; }
    [[nodiscard]] const auto& raw() const { return script_; }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const { return true; }

    /**
     * Accepted types are:
     * - void
     * - bool
     * - int
     * - double
     * - std::string
     * - const char*
     * - std::tuple
     */
    template <typename ReturnValue, typename... Args>
    [[nodiscard]] CallResult<ReturnValue> call(const std::string& function_name, Args&&... args)
    {
        return script_.call<ReturnValue>(function_name, std::forward<Args>(args)...);
    }

private:
    explicit LuaPlugin(LuaScript&& script)
        : script_ { std::move(script) }
    {
    }

private:
    LuaScript script_;
};

} // namespace ppplugin

#endif // PPPLUGIN_LUA_PLUGIN_H
