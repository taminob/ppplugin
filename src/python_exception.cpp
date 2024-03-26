#include "python/python_exception.h"
#include "detail/compatibility_utils.h"
#include "python/python_object.h"

#include <optional>

#include <Python.h>
#include <import.h>
#include <pytypedefs.h>

namespace ppplugin {
std::optional<PythonException> PythonException::latest()
{
    // TODO: acquire GIL
    PyObject* py_type = nullptr;
    PyObject* py_value = nullptr;
    PyObject* py_traceback = nullptr;
    // TODO: look at PyErr_GetRaisedException for >=3.12
    PyErr_Fetch(&py_type, &py_value, &py_traceback);

    if ((py_type == nullptr) && (py_value == nullptr) && (py_traceback == nullptr)) {
        return std::nullopt;
    }

    PythonException result;
    if (py_type != nullptr) {
        result.type_ = PythonObject { py_type }.as<std::string>();
    }
    if (py_value != nullptr) {
        result.value_ = PythonObject { py_value }.as<std::string>();
    }
    if (py_traceback != nullptr) {
        auto* traceback_module { PyImport_AddModule("traceback") };
        auto* format_traceback { PyObject_GetAttrString(traceback_module, "format_tb") };

        auto output = PythonObject { PyObject_CallOneArg(format_traceback, py_traceback) };
        result.traceback_ = output.as<std::string>();
    }
    return result;
}

[[nodiscard]] std::string PythonException::toString() const
{
    auto result = format("'{}': '{}'", type_.value_or("<unknown>"), value_.value_or("<?>"));
    if (traceback_) {
        result += format("\nTraceback:\n{}", *traceback_);
    }
    return result;
}
} // namespace ppplugin
