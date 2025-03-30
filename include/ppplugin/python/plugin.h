#ifndef PPPLUGIN_PYTHON_PLUGIN_H
#define PPPLUGIN_PYTHON_PLUGIN_H

#include "ppplugin/errors.h"
#include "ppplugin/expected.h"
#include "ppplugin/python/python_interpreter.h"

#include <filesystem>

namespace ppplugin {
class PythonPlugin {
public:
    /**
     * Load python file from given path.
     *
     * @param main_module If true, execute file as __main__ module
     */
    [[nodiscard]] static Expected<PythonPlugin, LoadError> load(const std::filesystem::path& python_script_path);

    ~PythonPlugin() = default;
    PythonPlugin(const PythonPlugin&) = delete;
    PythonPlugin(PythonPlugin&&) noexcept = default;
    PythonPlugin& operator=(const PythonPlugin&) = delete;
    PythonPlugin& operator=(PythonPlugin&&) noexcept = default;

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] CallResult<ReturnValue> call(const std::string& function_name, Args&&... args);

    template <typename VariableType>
    [[nodiscard]] CallResult<VariableType> global(const std::string& variable_name);
    template <typename VariableType>
    [[nodiscard]] CallResult<void> global(const std::string& variable_name, VariableType&& new_value);

private:
    PythonPlugin() = default;

private:
    PythonInterpreter interpreter_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> PythonPlugin::call(const std::string& function_name, Args&&... args)
{
    return interpreter_.call<ReturnValue>(function_name, std::forward<Args>(args)...);
}

template <typename VariableType>
CallResult<VariableType> PythonPlugin::global(const std::string& variable_name)
{
    return interpreter_.global<VariableType>(variable_name);
}
template <typename VariableType>
CallResult<void> PythonPlugin::global(const std::string& variable_name, VariableType&& new_value)
{
    return interpreter_.global(variable_name, std::forward<VariableType>(new_value));
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_PLUGIN_H
