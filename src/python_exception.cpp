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
#if PY_VERSION_HEX >= 0x030c0000 // Python 3.12 or newer
    PythonObject exception { PyErr_GetRaisedException() };
    PythonObject type { PyException_GetTraceback(exception.pyObject()) };
    PythonObject args_tuple { PyException_GetArgs(exception.pyObject()) };
    PythonObject value { (PyTuple_Size(args_tuple.pyObject()) > 0) ? PyTuple_GetItem(args_tuple.pyObject(), 0)
                                                                   : nullptr };
    PythonObject traceback { PyException_GetTraceback(exception.pyObject()) };
#else
    PyObject* py_type = nullptr;
    PyObject* py_value = nullptr;
    PyObject* py_traceback = nullptr;
    PyErr_Fetch(&py_type, &py_value, &py_traceback);
    PythonObject type { py_type };
    PythonObject value { py_value };
    PythonObject traceback { py_traceback };
#endif // PY_VERSION_HEX

    if (!type && !value && !traceback) {
        return std::nullopt;
    }

    PythonException result;
    if (type) {
        result.type_ = type.as<std::string>();
    }
    if (value) {
        result.value_ = value.as<std::string>();
    }
    if (traceback) {
        PythonObject traceback_module { PyImport_AddModule("traceback") };
        PythonObject format_traceback { PyObject_GetAttrString(traceback_module.pyObject(), "format_tb") };

        auto output = PythonObject { PyObject_CallOneArg(format_traceback.pyObject(), traceback.pyObject()) };
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
