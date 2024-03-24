#ifndef PPPLUGIN_PYTHON_TUPLE_H
#define PPPLUGIN_PYTHON_TUPLE_H

#include "python_object.h"
#include "python_forward_defs.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace ppplugin {
class PythonTuple {
public:
    template <typename... Args>
    explicit PythonTuple(Args&&... args);

    PyObject* pyObject() { return object(); }

private:
    PyObject* object() { return object_.pyObject(); }

    static PyObject* initTuple(int size);
    static void deleteTuple(PyObject* object);

    template <typename T, typename... Args>
    void fillTuple(int start_index, T&& arg, Args&&... args);

    void setTupleItem(int index, double value);
    void setTupleItem(int index, unsigned int value);
    void setTupleItem(int index, int value);
    void setTupleItem(int index, unsigned long value);
    void setTupleItem(int index, long value);
    void setTupleItem(int index, unsigned long long value);
    void setTupleItem(int index, long long value);
    void setTupleItem(int index, const char* value);
    void setTupleItem(int index, std::string_view value);
    void setTupleItem(int index, const std::string& value);
    void setTupleItem(int index, bool value);
    void setTupleItem(int index, std::nullptr_t);
    // TODO: also make adding function (via function pointer) possible?

private:
    PythonObject object_;
};

template <typename... Args>
PythonTuple::PythonTuple(Args&&... args)
    : object_ { initTuple(sizeof...(args)) }
{
    if (sizeof...(args) > 0) {
        fillTuple(0, std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
void PythonTuple::fillTuple(int start_index, T&& arg, Args&&... args)
{
    setTupleItem(start_index, std::forward<T>(arg));
    if constexpr (sizeof...(Args) > 0) {
        fillTuple(start_index + 1, std::forward<Args>(args)...);
    }
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_TUPLE_H
