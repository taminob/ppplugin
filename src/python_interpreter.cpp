#include "ppplugin/python/python_interpreter.h"
#include "ppplugin/errors.h"
#include "ppplugin/python/python_exception.h"
#include "ppplugin/python/python_forward_defs.h"
#include "ppplugin/python/python_guard.h"
#include "ppplugin/python/python_object.h"

#include <cassert>
#include <cstdio>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#define PY_SSIZE_T_CLEAN
#include <Python.h> // NOLINT(misc-include-cleaner)

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::once_flag python_initialization_done; // this is fine because Python
                                           // requires a once-per-process
                                           // initialization
[[maybe_unused]] std::vector<std::string> getDictKeys(PyObject* dict)
{
    auto* keys = PyDict_Keys(dict);
    assert(keys);
    auto keys_size = PyList_Size(keys);
    std::vector<std::string> result;
    result.reserve(keys_size);
    for (auto i = 0; i < keys_size; ++i) {
        auto* item = PyList_GetItem(keys, i);
        assert(item);
        PyObject* repr = PyObject_Repr(item);
        assert(repr);
        auto* bytes = PyUnicode_AsEncodedString(repr, "utf-8", "strict");
        assert(bytes);
        result.push_back(PyBytes_AsString(bytes));
        Py_DECREF(bytes);
        Py_DECREF(repr);
        Py_DECREF(item);
    }
    Py_DECREF(keys);
    return result;
}

#if PY_VERSION_HEX >= 0x030c0000 // Python 3.12 or newer
/**
 * Create interpreter configuration struct for new Python sub-interpreter.
 */
PyInterpreterConfig createInterpreterConfig()
{
    PyInterpreterConfig config {};
    config.use_main_obmalloc = 0;
    config.allow_fork = 0;
    config.allow_exec = 0;
    config.allow_threads = 1;
    config.allow_daemon_threads = 0;
    config.check_multi_interp_extensions = 1;
    config.gil = PyInterpreterConfig_OWN_GIL;
    return config;
}
#endif // PY_VERSION_HEX
} // namespace

namespace ppplugin {
PythonInterpreter::PythonInterpreter()
    : state_ { nullptr, [](auto* state) {
                  PythonGuard::lock(state);
                  Py_EndInterpreter(state);
              } }
{
    std::call_once(python_initialization_done, []() {
        PyConfig config {};
        PyConfig_InitIsolatedConfig(&config);
        auto init_status = Py_InitializeFromConfig(&config);
        PyConfig_Clear(&config);
        if (PyStatus_Exception(init_status) != 0) {
            // TODO: handle failure of Python initialization
        }
#if PY_VERSION_HEX < 0x03090000 // below Python 3.9
        // create GIL, is handled by Py_Initialize() since Python 3.7;
        // since Python 3.9, this function is deprecated
        PyEval_InitThreads();
#endif // PY_VERSION_HEX

        // release GIL of main-interpreter
        PyEval_ReleaseThread(PyThreadState_Get());
    });
    PyGILState_Ensure();
#if PY_VERSION_HEX >= 0x030c0000 // Python 3.12 or newer
    PyThreadState* new_state = nullptr;
    const auto config = createInterpreterConfig();
    auto status = Py_NewInterpreterFromConfig(&new_state, &config);
    if (PyStatus_IsError(status) == 0) {
        state_.reset(new_state);
    } else {
        // TODO: handle failure to create interpreter
        assert(false);
    }
#else
    // Python will exit application on error
    state_.reset(Py_NewInterpreter());
#endif // PY_VERSION_HEX
    main_module_ = { PyImport_AddModule("__main__"),
        [state = state_.get()](auto* main_module) {
            if (state) {
                const PythonGuard guard { state };
                Py_DECREF(main_module);
            }
        } };
    // release GIL of sub-interpreter
    PyEval_ReleaseThread(state());
}

PythonInterpreter::~PythonInterpreter()
{
    // main_module_ must be destructed before state_ since it will access the
    // state object in its deleter
    main_module_.reset();
    state_.reset();

    // TODO: call Py_Finalize()?
}

std::optional<LoadError> PythonInterpreter::load(const std::string& file_name)
{
    const std::unique_ptr<FILE, void (*)(FILE*)> file {
        std::fopen(file_name.c_str(), "r+"),
        [](FILE* file) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            std::ignore = std::fclose(file);
        }
    };
    if (!file) {
        return LoadError::fileNotReadable;
    }
    const PythonGuard python_guard { state() };
    auto* globals = PyModule_GetDict(mainModule());
    assert(globals);
    auto* locals = globals;
    const int start { Py_file_input };
    auto* result = PyRun_File(file.get(), file_name.c_str(), start, globals, locals);
    Py_DECREF(globals);
    if (result != nullptr) {
        Py_DECREF(result);
        return std::nullopt;
    }
    // TODO: check python exception on error
    return LoadError::unknown;
}

CallResult<PythonObject> PythonInterpreter::internalCall(const std::string& function_name, PyObject* args)
{
    auto* function = PyObject_GetAttrString(mainModule(), function_name.c_str());
    if ((function == nullptr) || (PyCallable_Check(function) == 0)) {
        Py_XDECREF(function);
        if (PythonException::occurred()) {
            if (auto exception = PythonException::latest()) {
                return CallError {
                    CallError::Code::symbolNotFound,
                    exception->toString()
                };
            }
            return CallError { CallError::Code::symbolNotFound };
        }
        return { CallError::Code::unknown };
    }
    PyObject* kwargs = nullptr;
    // TODO: checkout PyEval_SetTrace (?) or PyThreadState_SetAsyncExc to interrupt thread
    PythonObject result { PyObject_Call(function, args, kwargs) };
    // TODO: use PyObject_CallOneArg, PyObject_CallNoArgs, PyObject_CallObject?;
    //       see: https://docs.python.org/3/c-api/call.html
    //       or consider vectorcall: https://peps.python.org/pep-0590/
    Py_XDECREF(kwargs);
    Py_DECREF(function);
    if (PythonException::occurred()) {
        if (auto exception = PythonException::latest()) {
            return CallError { CallError::Code::unknown,
                exception->toString() };
        }
        return CallError { CallError::Code::unknown };
    }
    return result;
}

CallResult<PythonObject> PythonInterpreter::internalGlobal(const std::string& variable_name)
{
    auto* variable = PyObject_GetAttrString(mainModule(), variable_name.c_str());
    if (variable == nullptr) {
        if (PythonException::occurred()) {
            if (auto exception = PythonException::latest()) {
                return CallError {
                    CallError::Code::symbolNotFound,
                    exception->toString()
                };
            }
            return CallError { CallError::Code::symbolNotFound };
        }
        return { CallError::Code::unknown };
    }
    return PythonObject { variable };
}

CallResult<void> PythonInterpreter::internalGlobal(const std::string& variable_name, PythonObject new_value)
{
    PythonObject attribute_name { PyUnicode_FromString(variable_name.c_str()) };
    auto result = PyObject_SetAttr(mainModule(), attribute_name.pyObject(), new_value.pyObject());
    if (result < 0) {
        auto exception = PythonException::latest();
        return CallError {
            CallError::Code::unknown,
            exception.value_or(PythonException {}).toString()
        };
    }
    return {};
}
} // namespace ppplugin
