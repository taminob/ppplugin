#ifndef PPPLUGIN_PYTHON_GUARD_H
#define PPPLUGIN_PYTHON_GUARD_H

#include "python_forward_defs.h"

namespace ppplugin {
struct PythonGuard final {
    explicit PythonGuard(PyThreadState* state = nullptr);
    ~PythonGuard();
    PythonGuard(const PythonGuard&) = delete;
    PythonGuard(PythonGuard&&) = delete;
    PythonGuard& operator=(const PythonGuard&) = delete;
    PythonGuard& operator=(PythonGuard&&) = delete;

private:
    int previous_gil_;
    PyThreadState* previous_state_;
};
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_GUARD_H
