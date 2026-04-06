#include "ppplugin/python/python_list.h"

#include <cassert>

#define PY_SSIZE_T_CLEAN
#include <Python.h> // NOLINT(misc-include-cleaner)

namespace ppplugin {
PyObject* PythonList::initList(int size)
{
    if (size < 0) {
        return nullptr;
    }
    auto* new_list = PyList_New(size);
    assert(new_list);
    return new_list;
}

void PythonList::setListItem(int index, PyObject* value)
{
    // PyList_SetItem will steal this reference, so claim ownership first
    assert(value);
    assert(PyList_SetItem(object(), index, value) == 0);
}
} // namespace ppplugin
