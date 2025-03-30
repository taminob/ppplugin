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
    [[nodiscard]] static PythonObject from(double value);
    [[nodiscard]] static PythonObject from(unsigned int value);
    [[nodiscard]] static PythonObject from(int value);
    [[nodiscard]] static PythonObject from(unsigned long value);
    [[nodiscard]] static PythonObject from(long value);
    [[nodiscard]] static PythonObject from(unsigned long long value);
    [[nodiscard]] static PythonObject from(long long value);
    [[nodiscard]] static PythonObject from(const char* value);
    [[nodiscard]] static PythonObject from(std::string_view value);
    [[nodiscard]] static PythonObject from(const std::string& value);
    [[nodiscard]] static PythonObject from(bool value);
    [[nodiscard]] static PythonObject from(std::nullptr_t);
    // NOLINTEND(google-runtime-int)
    // NOLINTEND(bugprone-easily-swappable-parameters)
    // TODO: also make adding function (via function pointer) possible?

    /**
     * Wrap given PyObject into PythonObject without claiming
     * ownership (no Py_DECREF at end of lifetime).
     * Use this to access member functions for non-owned PyObjects.
     */
    [[nodiscard]] static PythonObject wrap(PyObject* object);

    [[nodiscard]] PyObject* pyObject() { return object(); }

    explicit operator bool()
    {
        return object() != nullptr;
    }

    /**
     * Get current value as given type.
     */
    template <typename T>
    [[nodiscard]] std::optional<T> as();

    /**
     * Cast current value to given type.
     */
    template <typename T>
    [[nodiscard]] std::optional<T> to();

private:
    PyObject* object() { return object_.get(); }

    // TODO: use explicit template instantiations instead?
    [[nodiscard]] std::optional<int> asInt();
    // NOLINTNEXTLINE(google-runtime-int)
    [[nodiscard]] std::optional<long long> asLongLong();
    [[nodiscard]] std::optional<double> asDouble();
    [[nodiscard]] std::optional<bool> asBool();
    [[nodiscard]] std::optional<std::string> asString();

    [[nodiscard]] std::optional<std::string> toString();

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
