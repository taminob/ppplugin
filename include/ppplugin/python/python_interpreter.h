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
    ~PythonInterpreter();
    PythonInterpreter(const PythonInterpreter&) = delete;
    PythonInterpreter(PythonInterpreter&&) = default;
    PythonInterpreter& operator=(const PythonInterpreter&) = delete;
    PythonInterpreter& operator=(PythonInterpreter&&) = default;

    [[nodiscard]] std::optional<LoadError> load(const std::string& file_name);
    template <typename ReturnValue, typename... Args>
    [[nodiscard]] CallResult<ReturnValue> call(const std::string& function_name, Args&&... args);

    template <typename VariableType>
    [[nodiscard]] CallResult<VariableType> global(const std::string& variable_name);
    template <typename VariableType>
    [[nodiscard]] CallResult<void> global(const std::string& variable_name, VariableType&& new_value);

private:
    [[nodiscard]] PyThreadState* state() { return state_.get(); }
    [[nodiscard]] PyObject* mainModule() { return main_module_.get(); }

    /**
     * Call function with given arguments.
     * Arguments must be wrapped in a python tuple.
     *
     * @note the GIL must be held
     */
    [[nodiscard]] CallResult<PythonObject> internalCall(const std::string& function_name, PyObject* args);

    /**
     * Return value of global variable for given name.
     *
     * @note the GIL must be held
     */
    [[nodiscard]] CallResult<PythonObject> internalGlobal(const std::string& variable_name);
    /**
     * Set value of global variable for given name.
     * Will create the global variable if it does not exist yet.
     *
     * @note the GIL must be held
     */
    [[nodiscard]] CallResult<void> internalGlobal(const std::string& variable_name, PythonObject new_value);

private:
    std::unique_ptr<PyThreadState, void (*)(PyThreadState*)> state_;
    std::unique_ptr<PyObject, std::function<void(PyObject*)>> main_module_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> PythonInterpreter::call(const std::string& function_name, Args&&... args)
{
    PythonGuard python_guard { state() };
    PythonTuple args_tuple { std::forward<Args>(args)... };

    return internalCall(function_name, args_tuple.pyObject()).andThen([](PythonObject&& result) -> CallResult<ReturnValue> {
        if constexpr (std::is_void_v<ReturnValue>) {
            return {};
        } else {
            if (auto return_value = std::move(result).as<ReturnValue>()) {
                return *return_value;
            }
            return CallError { CallError::Code::incorrectType, "Unable to convert result to return type!" };
        }
    });
}

template <typename VariableType>
CallResult<VariableType> PythonInterpreter::global(const std::string& variable_name)
{
    PythonGuard python_guard { state() };
    return internalGlobal(variable_name).andThen([](PythonObject&& object) -> CallResult<VariableType> {
        if (auto result = std::move(object).template as<VariableType>()) {
            return *result;
        }
        return { CallError::Code::incorrectType };
    });
}

template <typename VariableType>
CallResult<void> PythonInterpreter::global(const std::string& variable_name, VariableType&& new_value)
{
    PythonGuard python_guard { state() };
    return internalGlobal(variable_name, PythonObject::from(std::forward<VariableType>(new_value)));
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_INTERPRETER_H
