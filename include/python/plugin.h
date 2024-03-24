#ifndef PPPLUGIN_PYTHON_PLUGIN_H
#define PPPLUGIN_PYTHON_PLUGIN_H

#include "detail/function_details.h"
#include "errors.h"
#include "expected.h"
#include "python/python_interpreter.h"

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

private:
    PythonPlugin() = default;

    class PythonException {
    public:
        PythonException() = default;
        [[nodiscard]] static std::optional<PythonException> latest();

        [[nodiscard]] std::string toString() const;

    private:
        std::optional<std::string> type_;
        std::optional<std::string> value_;
        std::optional<std::string> traceback_;
    };

private:
    PythonInterpreter interpreter_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> PythonPlugin::call(const std::string& function_name, Args&&... args)
{
    return interpreter_.call<ReturnValue>(function_name, std::forward<Args>(args)...);
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_PLUGIN_H
