#include "python/python_guard.h"

#include <Python.h>
#include <pystate.h>

namespace ppplugin {
PythonGuard::PythonGuard(PyThreadState* state)
    : previous_gil_(static_cast<int>(PyGILState_Ensure())) // aquire GIL
{
    if (state != nullptr) {
        previous_state_ = PyThreadState_Swap(state); // requires GIL
    } else {
        previous_state_ = PyThreadState_Get(); // requires GIL
    }
}
PythonGuard::~PythonGuard()
{
    PyThreadState_Swap(previous_state_); // requires GIL
    PyGILState_Release(static_cast<PyGILState_STATE>(previous_gil_)); // release GIL
}
} // namespace ppplugin
