#include "ppplugin/python/python_object.h"

#include <optional>
#include <string>
#include <string_view>

#define PY_SSIZE_T_CLEAN
#include <Python.h> // NOLINT(misc-include-cleaner)

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
    new_object.object_ = { object, [](auto*) { } };
    return new_object;
}
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
// NOLINTBEGIN(google-runtime-int)
PythonObject PythonObject::from(double value)
{
    return PythonObject { PyFloat_FromDouble(value) };
}

PythonObject PythonObject::from(unsigned int value)
{
    return PythonObject::from(static_cast<unsigned long>(value));
}

PythonObject PythonObject::from(int value)
{
    return PythonObject::from(static_cast<long>(value));
}

PythonObject PythonObject::from(unsigned long value)
{
    return PythonObject { PyLong_FromUnsignedLong(value) };
}

PythonObject PythonObject::from(long value)
{
    return PythonObject { PyLong_FromLong(value) };
}

PythonObject PythonObject::from(unsigned long long value)
{
    return PythonObject { PyLong_FromUnsignedLongLong(value) };
}

PythonObject PythonObject::from(long long value)
{
    return PythonObject { PyLong_FromLongLong(value) };
}
// NOLINTEND(google-runtime-int)
// NOLINTEND(bugprone-easily-swappable-parameters)

PythonObject PythonObject::from(const char* value)
{
    return PythonObject { PyUnicode_FromString(value) };
}

PythonObject PythonObject::from(std::string_view value)
{
    return PythonObject { PyUnicode_FromStringAndSize(
        value.data(), static_cast<Py_ssize_t>(value.size())) };
}

PythonObject PythonObject::from(const std::string& value)
{
    return PythonObject::from(std::string_view { value });
}

PythonObject PythonObject::from(bool value)
{
    return PythonObject { PyBool_FromLong(static_cast<int>(value)) };
}

PythonObject PythonObject::from(std::nullptr_t)
{
    return PythonObject::wrap(Py_None);
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
        const auto* result = PyBytes_AsString(object());
        if (result == nullptr) {
            return std::nullopt;
        }
        auto length = PyBytes_Size(object());
        return std::string { result, static_cast<std::string::size_type>(length) };
    }
    return std::nullopt;
}

std::optional<std::string> PythonObject::toString()
{
    return PythonObject { PyObject_Str(object()) }.asString();
}
} // namespace ppplugin
