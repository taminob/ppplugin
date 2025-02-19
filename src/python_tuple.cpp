#include "ppplugin/python/python_tuple.h"
#include "ppplugin/python/python_forward_defs.h"

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
void PythonTuple::setTupleItem(int index, PythonObject value)
{
    auto* py_value = value.pyObject();
    assert(py_value);
    assert(PyTuple_SetItem(object(), index, py_value) == 0);
}
} // namespace ppplugin
