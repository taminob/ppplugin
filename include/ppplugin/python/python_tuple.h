#ifndef PPPLUGIN_PYTHON_TUPLE_H
#define PPPLUGIN_PYTHON_TUPLE_H

#include "python_forward_defs.h"
#include "python_object.h"

namespace ppplugin {
class PythonTuple {
public:
    template <typename... Args>
    explicit PythonTuple(Args&&... args);

    [[nodiscard]] PyObject* pyObject() { return object(); }

private:
    [[nodiscard]] PyObject* object() { return object_.pyObject(); }

    [[nodiscard]] static PyObject* initTuple(int size);

    template <typename T, typename... Args>
    void fillTuple(int start_index, T&& arg, Args&&... args);

    void setTupleItem(int index, PythonObject value);

private:
    PythonObject object_;
};

template <typename... Args>
PythonTuple::PythonTuple(Args&&... args)
    : object_ { initTuple(sizeof...(args)) }
{
    if constexpr (sizeof...(args) > 0) {
        fillTuple(0, std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
void PythonTuple::fillTuple(int start_index, T&& arg, Args&&... args)
{
    setTupleItem(start_index, PythonObject::from(std::forward<T>(arg)));
    if constexpr (sizeof...(Args) > 0) {
        fillTuple(start_index + 1, std::forward<Args>(args)...);
    }
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_TUPLE_H
