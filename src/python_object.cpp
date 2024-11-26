#include "ppplugin/python/python_object.h"

#include <optional>
#include <string>

#define PY_SSIZE_T_CLEAN
#include <Python.h> // NOLINT(misc-include-cleaner)
#include <boolobject.h>
#include <bytesobject.h>
#include <floatobject.h>
#include <longobject.h>
#include <object.h>
#include <pytypedefs.h>
#include <unicodeobject.h>

namespace ppplugin {
PythonObject::PythonObject()
    : PythonObject { nullptr }
{
}

PythonObject::PythonObject(PyObject* object)
    : object_ { object, [](PyObject* object) {
                   // TODO: must ensure GIL is held
                   Py_DECREF(object);
               } }
{
}

PythonObject PythonObject::wrap(PyObject* object)
{
    PythonObject new_object;
    new_object.object_ = { object, [](auto*) {} };
    return new_object;
}

std::optional<int> PythonObject::asInt()
{
    if (PyLong_Check(object()) != 0) {
        // TODO: PyErr_Occurred check necessary
        return PyLong_AsLong(object());
    }
    return std::nullopt;
}

// NOLINTNEXTLINE(google-runtime-int)
std::optional<long long> PythonObject::asLongLong()
{
    int overflow {};
    auto result = PyLong_AsLongLongAndOverflow(object(), &overflow);
    if (overflow == 0) {
        // TODO: PyErr_Occurred check necessary
        return result;
    }
    return std::nullopt;
}

std::optional<double> PythonObject::asDouble()
{
    if (PyFloat_Check(object()) != 0) {
        // TODO: PyErr_Occurred check necessary
        return PyFloat_AsDouble(object());
    }
    return std::nullopt;
}

std::optional<bool> PythonObject::asBool()
{
    if (PyBool_Check(object()) != 0) {
        return object() == Py_True;
    }
    return std::nullopt;
}

std::optional<std::string> PythonObject::asString()
{
    if (PyUnicode_Check(object()) != 0) {
        PythonObject utf8_object { PyUnicode_AsUTF8String(object()) };
        if (!utf8_object) {
            return std::nullopt;
        }
        std::string result { PyBytes_AsString(utf8_object.pyObject()) };
        return result;
    }
    if (PyBytes_Check(object()) != 0) {
        auto* result = PyBytes_AsString(object());
        if (result == nullptr) {
            return std::nullopt;
        }
        return std::string { result };
    }
    return std::nullopt;
}

std::optional<std::string> PythonObject::toString()
{
    return PythonObject { PyObject_Str(object()) }.asString();
}
} // namespace ppplugin
