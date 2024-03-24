#include "python/python_object.h"
#include "python/python_guard.h"

#include <Python.h>

namespace ppplugin {
PythonObject::PythonObject()
    : PythonObject { nullptr }
{
}

PythonObject::PythonObject(PyObject* object)
    : object_ { object, [](PyObject* object) {
                   PythonGuard python_guard; // TODO: pass correct thread state?
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
        auto* utf8_object = PyUnicode_AsUTF8String(object());
        if (utf8_object == nullptr) {
            return std::nullopt;
        }
        std::string result { PyBytes_AsString(utf8_object) };
        Py_DECREF(utf8_object);
        return result;
    }
    return std::nullopt;
}
} // namespace ppplugin
