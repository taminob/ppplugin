#ifndef PPPLUGIN_PYTHON_PLUGIN_H
#define PPPLUGIN_PYTHON_PLUGIN_H

#include "errors.h"

#include <string_view>

namespace ppplugin {
class PythonPlugin {
public:
    PythonPlugin() = default;

    virtual ~PythonPlugin() = default;
    PythonPlugin(const PythonPlugin&) = default;
    PythonPlugin(PythonPlugin&&) noexcept = default;
    PythonPlugin& operator=(const PythonPlugin&) = default;
    PythonPlugin& operator=(PythonPlugin&&) noexcept = default;

    template <typename ReturnValue, typename... Args>
    CallResult<ReturnValue> call(std::string_view /*function*/, Args... /*args*/)
    {
        return;
    }
};
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_PLUGIN_H
