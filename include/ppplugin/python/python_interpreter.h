#ifndef PPPLUGIN_PYTHON_INTERPRETER_H
#define PPPLUGIN_PYTHON_INTERPRETER_H

#include "ppplugin/errors.h"
#include "python_forward_defs.h"
#include "python_guard.h"
#include "python_object.h"
#include "python_tuple.h"

#include <functional>
#include <memory>

namespace ppplugin {
class PythonInterpreter {
public:
    PythonInterpreter();
    // TODO: call Py_Finalize()

    std::optional<LoadError> load(const std::string& file_name);
    template <typename ReturnValue, typename... Args>
    CallResult<ReturnValue> call(const std::string& function_name, Args&&... args);

    template <typename VariableType>
    CallResult<VariableType> global(const std::string& variable_name)
    {
        // TODO implement global for Python
        (void)variable_name;
        throw std::logic_error { "unimplemented" };
    }
    template <typename VariableType>
    void global(const std::string& variable_name, VariableType&& new_value)
    {
        // TODO implement global for Python
        (void)variable_name;
        (void)new_value;
        throw std::logic_error { "unimplemented" };
    }

private:
    PyThreadState* state() { return state_.get(); }
    PyObject* mainModule() { return main_module_.get(); }

    CallResult<PythonObject> internalCall(const std::string& function_name, PyObject* args);

private:
    std::unique_ptr<PyObject, std::function<void(PyObject*)>> main_module_;
    std::unique_ptr<PyThreadState, void (*)(PyThreadState*)> state_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> PythonInterpreter::call(const std::string& function_name, Args&&... args)
{
    PythonGuard python_guard { state() };
    PythonTuple args_tuple { std::forward<Args>(args)... };

    return internalCall(function_name, args_tuple.pyObject()).andThen([](const PythonObject& result) {
        if constexpr (std::is_void_v<ReturnValue>) {
            return;
        } else {
            return result.as<ReturnValue>();
        }
    });
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_INTERPRETER_H
