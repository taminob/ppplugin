#ifndef PPPLUGIN_PYTHON_GUARD_H
#define PPPLUGIN_PYTHON_GUARD_H

#include "python_forward_defs.h"

namespace ppplugin {
struct PythonGuard final {
    explicit PythonGuard(PyThreadState* state);
    ~PythonGuard();
    PythonGuard(const PythonGuard&) = delete;
    PythonGuard(PythonGuard&&) = delete;
    PythonGuard& operator=(const PythonGuard&) = delete;
    PythonGuard& operator=(PythonGuard&&) = delete;

private:
    PyThreadState* state_;
};
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_GUARD_H
