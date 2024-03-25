#include "python/python_tuple.h"
#include "python/python_guard.h"

#include <Python.h>
#include <object.h>
#include <tupleobject.h>

namespace ppplugin {

PyObject* PythonTuple::initTuple(int size)
{
    if (size < 0) {
        return nullptr;
    }
    auto* new_tuple = PyTuple_New(size);
    assert(new_tuple);
    return new_tuple;
}

void PythonTuple::setTupleItem(int index, double value)
{
    auto* py_value = PyFloat_FromDouble(value);
    assert(py_value);
    assert(PyTuple_SetItem(object(), index, py_value) == 0);
}

void PythonTuple::setTupleItem(int index, unsigned int value)
{
    setTupleItem(index, static_cast<unsigned long>(value));
}

void PythonTuple::setTupleItem(int index, int value)
{
    setTupleItem(index, static_cast<long>(value));
}

void PythonTuple::setTupleItem(int index, unsigned long value)
{
    auto* py_value = PyLong_FromUnsignedLong(value);
    assert(py_value);
    assert(PyTuple_SetItem(object(), index, py_value) == 0);
}

void PythonTuple::setTupleItem(int index, long value)
{
    auto* py_value = PyLong_FromLong(value);
    assert(py_value);
    assert(PyTuple_SetItem(object(), index, py_value) == 0);
}

void PythonTuple::setTupleItem(int index, unsigned long long value)
{
    auto* py_value = PyLong_FromUnsignedLongLong(value);
    assert(py_value);
    assert(PyTuple_SetItem(object(), index, py_value) == 0);
}

void PythonTuple::setTupleItem(int index, long long value)
{
    auto* py_value = PyLong_FromLongLong(value);
    assert(py_value);
    assert(PyTuple_SetItem(object(), index, py_value) == 0);
}

void PythonTuple::setTupleItem(int index, const char* value)
{
    auto* py_value = PyUnicode_FromString(value);
    assert(py_value);
    assert(PyTuple_SetItem(object(), index, py_value) == 0);
}

void PythonTuple::setTupleItem(int index, std::string_view value)
{
    auto* py_value = PyUnicode_FromStringAndSize(
        value.data(), static_cast<Py_ssize_t>(value.size()));
    assert(py_value);
    assert(PyTuple_SetItem(object(), index, py_value) == 0);
}

void PythonTuple::setTupleItem(int index, const std::string& value)
{
    setTupleItem(index, std::string_view { value });
}

void PythonTuple::setTupleItem(int index, bool value)
{
    auto* py_value = PyBool_FromLong(static_cast<int>(value));
    assert(py_value);
    assert(PyTuple_SetItem(object(), index, py_value) == 0);
}

void PythonTuple::setTupleItem(int index, std::nullptr_t)
{
    assert(PyTuple_SetItem(object(), index, Py_None) == 0);
}
} // namespace ppplugin
