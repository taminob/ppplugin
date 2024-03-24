#ifndef PPPLUGIN_PYTHON_OBJECT_H
#define PPPLUGIN_PYTHON_OBJECT_H

#include "python_forward_defs.h"

#include <memory>
#include <optional>
#include <string>

namespace ppplugin {
class PythonObject {
public:
    PythonObject();
    explicit PythonObject(PyObject* object);

    /**
     * Wrap given PyObject into PythonObject without claiming
     * ownership (no Py_DECREF at end of lifetime).
     * Use this to access member functions for non-owning PyObjects.
     */
    static PythonObject wrap(PyObject* object);

    PyObject* pyObject() { return object(); }

    template <typename T>
    std::optional<T> as();

private:
    PyObject* object() { return object_.get(); }

    // TODO: use explicit template instantiations instead?
    std::optional<int> asInt();
    std::optional<long long> asLongLong();
    std::optional<double> asDouble();
    std::optional<bool> asBool();
    std::optional<std::string> asString();

private:
    std::unique_ptr<PyObject, void (*)(PyObject*)> object_;
};

template <typename T>
std::optional<T> PythonObject::as()
{
    if constexpr (std::is_same_v<T, int>) {
        return asInt();
    } else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
        return asDouble();
    } else if constexpr (std::is_same_v<T, bool>) {
        return asBool();
    } else if constexpr (std::is_same_v<T, std::string>) {
        return asString();
    } else if constexpr (std::is_integral_v<T>) {
        return asLongLong();
    } else {
        static_assert(sizeof(T), "Cannot interpret PythonObject as given type!");
    }
    return std::nullopt;
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_OBJECT_H
