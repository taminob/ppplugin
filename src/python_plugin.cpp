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

std::string PythonPlugin::lastPythonError()
{
    // TODO: look at PyErr_GetRaisedException for >=3.12
    PyObject* py_type = nullptr;
    PyObject* py_value = nullptr;
    PyObject* py_traceback = nullptr;
    PyErr_Fetch(&py_type, &py_value, &py_traceback);

    std::optional<std::string> type;
    std::optional<std::string> value;
    std::optional<std::string> traceback;
    auto object_to_string = [](PyObject* object) {
        boost::python::handle<> handle { object };
        boost::python::str str { handle };
        return boost::python::extract<std::string> { str };
    };
    if (py_type != nullptr) {
        type = object_to_string(py_type);
    }
    if (py_value != nullptr) {
        value = object_to_string(py_value);
    }
    if (py_traceback != nullptr) {
        auto traceback_module { boost::python::import("traceback") };
        auto format_traceback(traceback_module.attr("format_tb"));

        boost::python::handle<> tb_handle { py_traceback };
        auto tb_list { format_traceback(tb_handle) };
        auto tb_str { boost::python::str("\n").join(tb_list) };
        traceback = boost::python::extract<std::string> { tb_str };
    }
    auto result = format("'{}': '{}'", type.value_or("<unknown>"), value.value_or("<?>"));
    if (traceback) {
        result += format("\nTraceback:\n{}", *traceback);
    }
    return result;
}
} // namespace ppplugin
