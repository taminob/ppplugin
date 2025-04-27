#include "ppplugin/python/python_guard.h"
#include "ppplugin/python/python_forward_defs.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h> // NOLINT(misc-include-cleaner)
#include <ceval.h>

namespace ppplugin {
PythonGuard::PythonGuard(PyThreadState* state)
    : state_ { state }
{
    lock(state_);
}

PythonGuard::~PythonGuard()
{
    unlock(state_);
}

void PythonGuard::lock(PyThreadState* state)
{
    assert(state != nullptr);
    PyEval_AcquireThread(state); // aquire GIL
}

void PythonGuard::unlock(PyThreadState* state)
{
    assert(state != nullptr);
    PyEval_ReleaseThread(state); // release GIL
}
} // namespace ppplugin
