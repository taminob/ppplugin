#include "ppplugin/python/python_tuple.h"
#include "ppplugin/python/python_forward_defs.h"

#include <cassert>

#define PY_SSIZE_T_CLEAN
#include <Python.h> // NOLINT(misc-include-cleaner)

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

void PythonTuple::setTupleItem(int index, PythonObject value)
{
    // PyTuple_SetItem will steal this reference, so claim ownership first
    assert(value);
    auto* released_value = value.release();

    assert(PyTuple_SetItem(object(), index, released_value) == 0);
}

PythonObject PythonTuple::getTupleItem(int index)
{
    auto* item = PyTuple_GetItem(object(), index);
    Py_XINCREF(item); // claim strong reference to item
    return PythonObject { item };
}
} // namespace ppplugin
