#include "python/python_exception.h"
#include "detail/compatibility_utils.h"
#include "python/python_object.h"

#include <cassert>
#include <optional>
#include <string>

#define PY_SSIZE_T_CLEAN
#include <Python.h> // NOLINT(misc-include-cleaner)
#include <abstract.h>
#include <import.h>
#include <listobject.h>
#include <object.h>
#include <pyerrors.h>
#include <pytypedefs.h>

namespace ppplugin {
std::optional<PythonException> PythonException::latest()
{
    // TODO: make sure GIL is acquired
#if PY_VERSION_HEX >= 0x030c0000 // Python 3.12 or newer
    PythonObject exception { PyErr_GetRaisedException() };
    PythonObject type { PyObject_Type(exception.pyObject()) };
    PythonObject args_tuple { PyException_GetArgs(exception.pyObject()) };
    PythonObject value { (PyTuple_Size(args_tuple.pyObject()) > 0) ? PyTuple_GetItem(args_tuple.pyObject(), 0)
                                                                   : nullptr };
    PythonObject traceback { PyException_GetTraceback(exception.pyObject()) };
#else
    PyObject* py_type {};
    PyObject* py_value = {};
    PyObject* py_traceback {};
    PyErr_Fetch(&py_type, &py_value, &py_traceback);
    PyErr_NormalizeException(&py_type, &py_value, &py_traceback);
    PythonObject type { py_type };
    PythonObject value { py_value };
    PythonObject traceback { py_traceback };
#endif // PY_VERSION_HEX

    if (!type && !value && !traceback) {
        return std::nullopt;
    }

    PythonException result;
    if (type) {
        result.type_ = type.to<std::string>();
    }
    if (value) {
        result.value_ = value.to<std::string>();
    }
    if (traceback) {
        PythonObject traceback_module { PyImport_ImportModule("traceback") };
        assert(traceback_module);
        PythonObject format_traceback { PyObject_GetAttrString(traceback_module.pyObject(), "format_tb") };
        assert(format_traceback);

        auto output = PythonObject { PyObject_CallOneArg(format_traceback.pyObject(), traceback.pyObject()) };
        if (PyList_Check(output.pyObject())) {
            auto output_size = PyList_Size(output.pyObject());
            std::string formatted_traceback;
            for (int i = 0; i < output_size; ++i) {
                formatted_traceback += PythonObject::wrap(PyList_GetItem(output.pyObject(), i)).to<std::string>().value_or("");
            }
            if (!formatted_traceback.empty()) {
                result.traceback_ = formatted_traceback;
            }
        } else {
            result.traceback_ = output.to<std::string>();
        }
    }
    return result;
}

bool PythonException::occurred()
{
    return PyErr_Occurred() != nullptr;
}

std::string PythonException::toString() const
{
    auto result = format("'{}': '{}'", type_.value_or("<unknown>"), value_.value_or("<?>"));
    if (traceback_) {
        result += format("\nTraceback:\n{}", *traceback_);
    }
    return result;
}
} // namespace ppplugin
