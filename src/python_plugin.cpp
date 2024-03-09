#include "python/plugin.h"

namespace ppplugin {
PythonPlugin::PythonPlugin(bool is_main_module)
{
    Py_Initialize();
    if (is_main_module) {
        auto main_module = boost::python::import("__main__");
        module_ = main_module.attr("__dict__");
    } else {
        module_ = boost::python::dict {};
    }
}

Expected<PythonPlugin, LoadError> PythonPlugin::load(const std::filesystem::path& python_script_path, bool main_module)
{
    if (!std::filesystem::exists(python_script_path)) {
        return LoadError::fileNotFound;
    }
    PythonPlugin new_plugin { main_module };
    std::ignore = boost::python::exec_file(python_script_path.c_str(),
        new_plugin.module_, new_plugin.module_);
    return new_plugin;
}

std::optional<PythonPlugin::PythonException> PythonPlugin::PythonException::latest()
{
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
        boost::python::handle<> handle { object };
        boost::python::str string { handle };
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

        boost::python::handle<> tb_handle { py_traceback };
        auto tb_list { format_traceback(tb_handle) };
        auto tb_str { boost::python::str("\n").join(tb_list) };
        result.traceback_ = boost::python::extract<std::string> { tb_str };
    }
    return result;
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
