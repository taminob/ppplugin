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

    // NOLINTBEGIN(bugprone-easily-swappable-parameters)
    // NOLINTBEGIN(google-runtime-int)
    static PythonObject from(double value);
    static PythonObject from(unsigned int value);
    static PythonObject from(int value);
    static PythonObject from(unsigned long value);
    static PythonObject from(long value);
    static PythonObject from(unsigned long long value);
    static PythonObject from(long long value);
    static PythonObject from(const char* value);
    static PythonObject from(std::string_view value);
    static PythonObject from(const std::string& value);
    static PythonObject from(bool value);
    static PythonObject from(std::nullptr_t);
    // NOLINTEND(google-runtime-int)
    // NOLINTEND(bugprone-easily-swappable-parameters)
    // TODO: also make adding function (via function pointer) possible?

    /**
     * Wrap given PyObject into PythonObject without claiming
     * ownership (no Py_DECREF at end of lifetime).
     * Use this to access member functions for non-owned PyObjects.
     */
    static PythonObject wrap(PyObject* object);

    PyObject* pyObject() { return object(); }

    explicit operator bool()
    {
        return object() != nullptr;
    }

    /**
     * Get current value as given type.
     */
    template <typename T>
    std::optional<T> as();

    /**
     * Cast current value to given type.
     */
    template <typename T>
    std::optional<T> to();

private:
    PyObject* object() { return object_.get(); }

    // TODO: use explicit template instantiations instead?
    std::optional<int> asInt();
    // NOLINTNEXTLINE(google-runtime-int)
    std::optional<long long> asLongLong();
    std::optional<double> asDouble();
    std::optional<bool> asBool();
    std::optional<std::string> asString();

    std::optional<std::string> toString();

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

template <typename T>
std::optional<T> PythonObject::to()
{
    if constexpr (std::is_same_v<T, std::string>) {
        return toString();
    } else {
        static_assert(sizeof(T), "Cannot interpret PythonObject as given type!");
    }
    return std::nullopt;
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_OBJECT_H
