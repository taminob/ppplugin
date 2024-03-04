#ifndef PPPLUGIN_LUA_PLUGIN_H
#define PPPLUGIN_LUA_PLUGIN_H

#include "errors.h"
#include "lua/lua_script.h"

#include <string>
#include <string_view>

namespace ppplugin {
class LuaPlugin {
public:
    explicit LuaPlugin(const std::filesystem::path& lua_script_path)
        : script_ { lua_script_path }
    {
    }
    virtual ~LuaPlugin() = default;
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
    LuaScript script_;
};

} // namespace ppplugin

#endif // PPPLUGIN_LUA_PLUGIN_H
