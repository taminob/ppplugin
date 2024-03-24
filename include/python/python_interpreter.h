#ifndef PPPLUGIN_PYTHON_INTERPRETER_H
#define PPPLUGIN_PYTHON_INTERPRETER_H

#include "errors.h"
#include "python_forward_defs.h"
#include "python_object.h"
#include "python_tuple.h"

#include <memory>

namespace ppplugin {
class PythonInterpreter {
public:
    PythonInterpreter();

    std::optional<LoadError> load(const std::string& file_name);
    template <typename ReturnValue, typename... Args>
    CallResult<ReturnValue> call(const std::string& function_name, Args&&... args);

private:
    PyThreadState* state() { return state_.get(); }
    PyObject* mainModule() { return main_module_.get(); }

    CallResult<PyObject*> internalCall(const std::string& function_name, PyObject* args);

private:
    std::unique_ptr<PyThreadState, void (*)(PyThreadState*)> state_;
    std::unique_ptr<PyObject, void (*)(PyObject*)> main_module_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> PythonInterpreter::call(const std::string& function_name, Args&&... args)
{
    PythonTuple args_tuple { std::forward<Args>(args)... };

    return internalCall(function_name, args_tuple.pyObject()).andThen([](PyObject* result) {
        if constexpr (std::is_void_v<ReturnValue>) {
            return;
        } else {
            return PythonObject { result }.as<ReturnValue>();
        }
    });
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_INTERPRETER_H
