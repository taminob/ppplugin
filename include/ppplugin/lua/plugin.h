#ifndef PPPLUGIN_LUA_PLUGIN_H
#define PPPLUGIN_LUA_PLUGIN_H

#include "lua_script.h"
#include "ppplugin/errors.h"

#include <string>

namespace ppplugin {
class LuaPlugin {
public:
    [[nodiscard]] static Expected<LuaPlugin, LoadError> load(
        const std::filesystem::path& plugin_library_path, bool auto_run = true);

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
    [[nodiscard]] CallResult<ReturnValue> call(const std::string& function_name, Args&&... args);

    template <typename VariableType>
    [[nodiscard]] CallResult<VariableType> global(const std::string& variable_name);
    template <typename VariableType>
    [[nodiscard]] CallResult<void> global(const std::string& variable_name, VariableType&& new_value);

private:
    explicit LuaPlugin(LuaScript&& script)
        : script_ { std::move(script) }
    {
    }

private:
    LuaScript script_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> LuaPlugin::call(const std::string& function_name, Args&&... args)
{
    return script_.call<ReturnValue>(function_name, std::forward<Args>(args)...);
}

template <typename VariableType>
CallResult<VariableType> LuaPlugin::global(const std::string& variable_name)
{
    return script_.global<VariableType>(variable_name);
}

template <typename VariableType>
CallResult<void> LuaPlugin::global(const std::string& variable_name, VariableType&& new_value)
{
    script_.global(variable_name, std::forward<VariableType>(new_value));
    // cannot fail in Lua; new value will be assigned regardless of previous type
    // and variable will be created if it does not exist yet
    return {};
}
} // namespace ppplugin

#endif // PPPLUGIN_LUA_PLUGIN_H
