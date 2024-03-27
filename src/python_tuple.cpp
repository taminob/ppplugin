#include "python/python_tuple.h"
#include "python/python_forward_defs.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>

#define PY_SSIZE_T_CLEAN
#include <Python.h> // NOLINT(misc-include-cleaner)
#include <boolobject.h>
#include <floatobject.h>
#include <longobject.h>
#include <object.h>
#include <pyport.h>
#include <tupleobject.h>
#include <unicodeobject.h>

namespace ppplugin {

PyObject* PythonTuple::initTuple(int size)
{
    // TODO: ensure GIL is held for each function where it is necessary
    if (size < 0) {
        return nullptr;
    }
    auto* new_tuple = PyTuple_New(size);
    assert(new_tuple);
    return new_tuple;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
// NOLINTBEGIN(google-runtime-int)
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
// NOLINTEND(google-runtime-int)
// NOLINTEND(bugprone-easily-swappable-parameters)

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
