#include "python/python_interpreter.h"
#include "python/python_guard.h"

#include <mutex>
#include <string>
#include <vector>

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pylifecycle.h>

namespace {
std::once_flag python_initialization_done;
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
} // namespace

namespace ppplugin {
PythonInterpreter::PythonInterpreter()
    : state_ { nullptr, [](auto* state) {
                  auto previous_gil = PyGILState_Ensure();
                  auto* previous_state = PyThreadState_Swap(state);
                  Py_EndInterpreter(state);
                  PyThreadState_Swap(previous_state);
                  PyGILState_Release(previous_gil);
              } }
    , main_module_ { nullptr, [](auto* main_module) {
                        auto previous_gil = PyGILState_Ensure();
                        Py_DECREF(main_module);
                        PyGILState_Release(previous_gil);
                    } }
{
    std::call_once(python_initialization_done, []() {
        Py_Initialize();
#if PY_VERSION_HEX < 0x03090000
        // create GIL, is handled by Py_Initialize() since Python 3.9
        PyEval_InitThreads();
#endif // PY_VERSION_HEX
        return PyEval_SaveThread(); // release GIL
    });
    PythonGuard python_guard;
    // TODO: also works if all PythonGuards and the creation
    //       of the sub-interpreter gets removed - find out why
#if PY_VERSION_HEX >= 0x030c0000
    PyThreadState* state = nullptr;
    PyInterpreterConfig config = _PyInterpreterConfig_INIT;
    auto status = Py_NewInterpreterFromConfig(&state, &config);
    // TODO: handle failure to create interpreter
    if (PyStatus_IsError(status) == 0) {
        state_.reset(state);
    } else {
        assert(false);
    }
#else
    state_.reset(Py_NewInterpreter());
#endif // PY_VERSION_HEX
    main_module_.reset(PyImport_AddModule("__main__"));
}

std::optional<LoadError> PythonInterpreter::load(const std::string& file_name)
{
    std::unique_ptr<FILE, void (*)(FILE*)> file { std::fopen(file_name.c_str(), "r+"), [](FILE* file) { std::fclose(file); } };
    if (!file) {
        return LoadError::fileNotReadable;
    }
    PythonGuard python_guard { state() };
    auto* globals = PyModule_GetDict(mainModule());
    assert(globals);
    auto* locals = globals;
    int start { Py_file_input };
    auto* result = PyRun_File(file.get(), file_name.c_str(), start, globals, locals);
    Py_DECREF(globals);
    if (result != nullptr) {
        Py_DECREF(result);
        return std::nullopt;
    }
    // TODO: check python exception on error
    return LoadError::unknown;
}

CallResult<PyObject*> PythonInterpreter::internalCall(const std::string& function_name, PyObject* args)
{
    PythonGuard python_guard { state() };
    auto* function = PyObject_GetAttrString(mainModule(), function_name.c_str());
    if ((function == nullptr) || (PyCallable_Check(function) == 0)) {
        Py_XDECREF(function);
        if (PyErr_Occurred() != nullptr) {
            // TODO: don't print, return instead
            PyErr_Print();
        }
        return { CallError::Code::unknown };
    }
    PyObject* kwargs = nullptr;
    // TODO: checkout PyEval_SetTrace (?) or PyThreadState_SetAsyncExc to interrupt thread
    auto* result = PyObject_Call(function, args, kwargs);
    // TODO: use PyObject_CallOneArg, PyObject_CallNoArgs, PyObject_CallObject?;
    //       see: https://docs.python.org/3/c-api/call.html
    //       or consider vectorcall: https://peps.python.org/pep-0590/
    Py_XDECREF(kwargs);
    Py_DECREF(function);
    return result;
}

} // namespace ppplugin
