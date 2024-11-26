#include "ppplugin/python/python_guard.h"
#include "ppplugin/python/python_forward_defs.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h> // NOLINT(misc-include-cleaner)
#include <ceval.h>

namespace ppplugin {
PythonGuard::PythonGuard(PyThreadState* state)
    : state_ { state }
{
    PyEval_AcquireThread(state_); // aquire GIL
}
PythonGuard::~PythonGuard()
{
    PyEval_ReleaseThread(state_); // release GIL
}
} // namespace ppplugin
