#ifndef PPPLUGIN_SHELL_PLUGIN_H
#define PPPLUGIN_SHELL_PLUGIN_H

#include "ppplugin/errors.h"
#include "shell_session.h"
#include "shell_type_conversion.h"

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

    explicit operator bool() { return shell_.isRunning(); }

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
        : shell_ { "/bin/sh" }
    {
    }

    template <typename... Args>
    std::string buildCommandLine(const std::string& function_name, Args&&... args);

private:
    ShellSession shell_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> ShellPlugin::call(const std::string& function_name, Args&&... args)
{
    auto command_line = buildCommandLine(function_name, std::forward<Args>(args)...);
    if constexpr (std::is_void_v<ReturnValue>) {
        return shell_.callWithoutResult(command_line);
    } else {
        return shell_.callWithResult(command_line).andThen([](auto&& result) -> CallResult<ReturnValue> {
            if (auto converted_result = convertFromShellString<ReturnValue>(result)) {
                return *converted_result;
            }
            return CallError { CallErrorCode::incorrectType };
        });
    }
}

template <typename VariableType>
CallResult<VariableType> ShellPlugin::global(const std::string& variable_name)
{
    return shell_.environmentVariable(variable_name).andThen([](auto&& result) -> CallResult<VariableType> {
        if (auto converted_result = convertFromShellString<VariableType>(result)) {
            return *converted_result;
        }
        return CallError { CallErrorCode::incorrectType };
    });
}

template <typename VariableType>
CallResult<void> ShellPlugin::global(const std::string& variable_name, VariableType&& new_value)
{
    return shell_.environmentVariable(variable_name, std::forward<VariableType>(new_value));
}

template <typename... Args>
std::string ShellPlugin::buildCommandLine(const std::string& function_name, Args&&... args)
{
    auto command_line = convertToShellString(function_name);
    if constexpr (sizeof...(Args)) {
        // use fold expression to convert each argument and separate using space character
        command_line += ((' ' + convertToShellString(std::forward<Args>(args))) + ...);
    }
    return command_line;
}
} // namespace ppplugin

#endif // PPPLUGIN_SHELL_PLUGIN_H
