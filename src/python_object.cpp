#include "ppplugin/python/python_object.h"
#include "ppplugin/python/python_exception.h"

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

PythonObject PythonObject::from(char value)
{
    // Python does not have a character class, only strings of length 1
    return PythonObject::from(std::string_view { &value, 1 });
}

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
    if (isInt()) {
        auto result = PyLong_AsLong(object());
        if (result == -1 && PythonException::occurred()) {
            return std::nullopt;
        }
        return result;
    }
    return std::nullopt;
}

// NOLINTNEXTLINE(google-runtime-int)
std::optional<long long> PythonObject::asLongLong()
{
    if (!isInt()) {
        return std::nullopt;
    }

    int overflow {};
    auto result = PyLong_AsLongLongAndOverflow(object(), &overflow);
    if ((result == -1 && PythonException::occurred()) || overflow != 0) {
        // overflow or other error occurred
        return std::nullopt;
    }
    return result;
}

std::optional<double> PythonObject::asDouble()
{
    if (isDouble()) {
        auto result = PyFloat_AsDouble(object());
        if (result == -1.0 && PythonException::occurred()) {
            return std::nullopt;
        }
        return result;
    }
    return std::nullopt;
}

std::optional<bool> PythonObject::asBool()
{
    if (isBool()) {
        return object() == Py_True;
    }
    return std::nullopt;
}

std::optional<std::string> PythonObject::asString()
{
    if (isString()) {
        PythonObject utf8_object { PyUnicode_AsUTF8String(object()) };
        if (!utf8_object) {
            return std::nullopt;
        }
        std::string result { PyBytes_AsString(utf8_object.pyObject()) };
        return result;
    }
    if (isBytes()) {
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

bool PythonObject::isInt()
{
    return object() != nullptr && PyLong_Check(object()) != 0;
}

bool PythonObject::isDouble()
{
    return object() != nullptr && PyFloat_Check(object()) != 0;
}

bool PythonObject::isBool()
{
    return object() != nullptr && PyBool_Check(object()) != 0;
}

bool PythonObject::isString()
{
    return object() != nullptr && PyUnicode_Check(object()) != 0;
}

bool PythonObject::isBytes()
{
    return object() != nullptr && PyBytes_Check(object()) != 0;
}

bool PythonObject::isDict()
{
    return object() != nullptr && PyDict_Check(object()) != 0;
}

bool PythonObject::isList()
{
    return object() != nullptr && PyList_Check(object()) != 0;
}

PyObject* PythonObject::initList(int size)
{
    if (size < 0) {
        return nullptr;
    }
    auto* new_list = PyList_New(size);
    assert(new_list);
    return new_list;
}

PyObject* PythonObject::initDict()
{
    auto* new_dict = PyDict_New();
    assert(new_dict);
    return new_dict;
}

void PythonObject::setListItem(int index, PyObject* value)
{
    // PyList_SetItem will steal this reference, so claim ownership first
    assert(value);
    Py_INCREF(value);

    assert(isList());
    assert(PyList_SetItem(object(), index, value) == 0);
}

void PythonObject::setDictItem(PyObject* key, PyObject* value)
{
    assert(key);
    assert(value);

    assert(isDict());
    assert(PyDict_SetItem(object(), key, value) == 0);
}

PythonObject PythonObject::getItem(int index)
{
    return PythonObject { PySequence_GetItem(object(), index) };
}

PythonObject PythonObject::getNextItem(PythonObject& iterator)
{
    if (!iterator) {
        PythonObject py_object;
        if (isDict()) {
            // if dictionary, iterate over items (key/value tuples)
            py_object = PythonObject { PyDict_Items(object()) };
        } else {
            // otherwise, iterate over all elements directly
            py_object = PythonObject::wrap(object());
        }
        auto new_iterator = PythonObject { PyObject_GetIter(py_object.object()) };
        if (!new_iterator) {
            return {};
        }
        iterator = std::move(new_iterator);
    }

    return PythonObject { PyIter_Next(iterator.object()) };
}
} // namespace ppplugin
