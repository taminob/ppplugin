#ifndef PPPLUGIN_SHELL_PLUGIN_H
#define PPPLUGIN_SHELL_PLUGIN_H

#include "ppplugin/errors.h"
#include "shell_session.h"

#include <filesystem>
#include <string>

namespace ppplugin {
class ShellPlugin {
public:
    [[nodiscard]] static Expected<ShellPlugin, LoadError> load(
        const std::filesystem::path& plugin_library_path);

    ~ShellPlugin() = default;
    ShellPlugin(const ShellPlugin&) = delete;
    ShellPlugin(ShellPlugin&&) = default;
    ShellPlugin& operator=(const ShellPlugin&) = delete;
    ShellPlugin& operator=(ShellPlugin&&) = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() { return shell_.isRunning(); }

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
    explicit ShellPlugin()
        // : shell_ { "/tmp/a.out" }
        : shell_ { "/bin/sh" }
    {
    }

private:
    ShellSession shell_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> ShellPlugin::call(const std::string& function_name, Args&&... args)
{
    if constexpr (std::is_void_v<ReturnValue>) {
        return shell_.callWithoutResult(function_name, { args... });
    } else {
        return shell_.callWithResult(function_name, { args... });
    }
}

template <typename VariableType>
CallResult<VariableType> ShellPlugin::global(const std::string& variable_name)
{
    return shell_.environmentVariable(variable_name);
}

template <typename VariableType>
CallResult<void> ShellPlugin::global(const std::string& variable_name, VariableType&& new_value)
{
    return shell_.environmentVariable(variable_name, std::forward<VariableType>(new_value));
}
} // namespace ppplugin

#endif // PPPLUGIN_SHELL_PLUGIN_H
