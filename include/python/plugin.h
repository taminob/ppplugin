#ifndef PPPLUGIN_PYTHON_PLUGIN_H
#define PPPLUGIN_PYTHON_PLUGIN_H

#include "detail/function_details.h"
#include "errors.h"
#include "expected.h"

#include <boost/python.hpp>

#include <filesystem>

namespace ppplugin {
class PythonPlugin {
public:
    /**
     * Load python file from given path.
     *
     * @param main_module If true, execute file as __main__ module
     */
    static Expected<PythonPlugin, LoadError> load(const std::filesystem::path& python_script_path, bool is_main_module = true);

    ~PythonPlugin() = default;
    PythonPlugin(const PythonPlugin&) = default;
    PythonPlugin(PythonPlugin&&) noexcept = default;
    PythonPlugin& operator=(const PythonPlugin&) = default;
    PythonPlugin& operator=(PythonPlugin&&) noexcept = default;

    template <typename ReturnValue, typename... Args>
    CallResult<ReturnValue> call(const std::string& function_name, Args&&... args);

private:
    explicit PythonPlugin(bool is_main_module);

    static std::string lastPythonError();

private:
    boost::python::object module_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> PythonPlugin::call(const std::string& function_name, Args&&... args)
{
    using FunctionDetails = detail::templates::FunctionDetails<ReturnValue(Args...)>;

    try {
        auto result = module_[function_name.c_str()](std::forward<Args>(args)...);

        if constexpr (detail::templates::returnTypeCount<FunctionDetails>() > 0) {
            return { boost::python::extract<ReturnValue>(result) };
        } else {
            return CallResult<void> {};
        }
    } catch (const boost::python::error_already_set& exception) {
        return { CallError(CallError::Code::unknown, lastPythonError()) };
    }
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_PLUGIN_H
