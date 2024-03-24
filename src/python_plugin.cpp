#include "python/plugin.h"

#include "detail/compatibility_utils.h"
#include "errors.h"
#include "expected.h"

#include <filesystem>
#include <optional>
#include <string>

#include <boost/python.hpp>

//#include <Python.h>
#include <pyerrors.h>
#include <pylifecycle.h>
#include <pytypedefs.h>

namespace ppplugin {
Expected<PythonPlugin, LoadError> PythonPlugin::load(const std::filesystem::path& python_script_path)
{
    if (!std::filesystem::exists(python_script_path)) {
        return LoadError::fileNotFound;
    }
    PythonPlugin new_plugin {};
    if (auto load_error = new_plugin.interpreter_.load(python_script_path)) {
        return *load_error;
    }
    return new_plugin;
}

std::optional<PythonPlugin::PythonException> PythonPlugin::PythonException::latest()
{
    try {
        PyObject* py_type = nullptr;
        PyObject* py_value = nullptr;
        PyObject* py_traceback = nullptr;
        // TODO: look at PyErr_GetRaisedException for >=3.12
        PyErr_Fetch(&py_type, &py_value, &py_traceback);

        if ((py_type == nullptr) && (py_value == nullptr) && (py_traceback == nullptr)) {
            return std::nullopt;
        }

        PythonException result;
        auto object_to_string = [](PyObject* object) {
            const boost::python::handle<> handle { object };
            const boost::python::str string { handle };
            return boost::python::extract<std::string> { string };
        };
        if (py_type != nullptr) {
            result.type_ = object_to_string(py_type);
        }
        if (py_value != nullptr) {
            result.value_ = object_to_string(py_value);
        }
        if (py_traceback != nullptr) {
            auto traceback_module { boost::python::import("traceback") };
            auto format_traceback(traceback_module.attr("format_tb"));

            const boost::python::handle<> tb_handle { py_traceback };
            auto tb_list { format_traceback(tb_handle) };
            auto tb_str { boost::python::str("\n").join(tb_list) };
            result.traceback_ = boost::python::extract<std::string> { tb_str };
        }
        return result;
    } catch (const boost::python::error_already_set& exception) {
        return std::nullopt;
    }
}

[[nodiscard]] std::string PythonPlugin::PythonException::toString() const
{
    auto result = format("'{}': '{}'", type_.value_or("<unknown>"), value_.value_or("<?>"));
    if (traceback_) {
        result += format("\nTraceback:\n{}", *traceback_);
    }
    return result;
}
} // namespace ppplugin
