#include "python/python_guard.h"

#include <Python.h>
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
