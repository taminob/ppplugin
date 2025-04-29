#ifndef PPPLUGIN_PYTHON_OBJECT_H
#define PPPLUGIN_PYTHON_OBJECT_H

#include "ppplugin/detail/template_helpers.h"
#include "python_forward_defs.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ppplugin {
class PythonObject {
public:
    PythonObject();
    explicit PythonObject(PyObject* object);

    [[nodiscard]] static PythonObject from(double value);
    // NOLINTBEGIN(google-runtime-int)
    [[nodiscard]] static PythonObject from(unsigned int value);
    [[nodiscard]] static PythonObject from(int value);
    [[nodiscard]] static PythonObject from(unsigned long value);
    [[nodiscard]] static PythonObject from(long value);
    [[nodiscard]] static PythonObject from(unsigned long long value);
    [[nodiscard]] static PythonObject from(long long value);
    // NOLINTEND(google-runtime-int)
    [[nodiscard]] static PythonObject from(char value);
    [[nodiscard]] static PythonObject from(const char* value);
    [[nodiscard]] static PythonObject from(std::string_view value);
    [[nodiscard]] static PythonObject from(const std::string& value);
    [[nodiscard]] static PythonObject from(bool value);
    [[nodiscard]] static PythonObject from(std::nullptr_t);
    template <typename K, typename V>
    [[nodiscard]] static PythonObject from(const std::map<K, V>& value);
    template <typename T>
    [[nodiscard]] static PythonObject from(const std::vector<T>& value);
    // TODO: also make adding function (via function pointer) possible?

    /**
     * Wrap given PyObject into PythonObject without claiming
     * ownership (no Py_DECREF at end of lifetime).
     * Use this to access member functions for non-owned PyObjects.
     */
    [[nodiscard]] static PythonObject wrap(PyObject* object);

    [[nodiscard]] PyObject* pyObject() { return object(); }
    /**
     * Release owned reference and returned internal PyObject.
     * This will make this instance an empty PythonObject and no
     * Py_DECREF will be called for any previously managed object.
     */
    [[nodiscard]] PyObject* release() { return object_.release(); }

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
    template <typename T>
    [[nodiscard]] std::optional<T> asMap();
    template <typename T>
    [[nodiscard]] std::optional<T> asArray();

    [[nodiscard]] std::optional<std::string> toString();

    [[nodiscard]] bool isInt();
    [[nodiscard]] bool isDouble();
    [[nodiscard]] bool isBool();
    [[nodiscard]] bool isString();
    [[nodiscard]] bool isBytes();
    [[nodiscard]] bool isDict();
    [[nodiscard]] bool isList();

    [[nodiscard]] static PyObject* initList(int size);
    [[nodiscard]] static PyObject* initDict();

    void setListItem(int index, PyObject* value);
    void setDictItem(PyObject* key, PyObject* value);

    [[nodiscard]] PythonObject getItem(int index);
    [[nodiscard]] PythonObject getNextItem(PythonObject& iterator);

private:
    std::unique_ptr<PyObject, void (*)(PyObject*)> object_;
};

template <typename K, typename V>
PythonObject PythonObject::from(const std::map<K, V>& dict)
{
    PythonObject object { initDict() };
    for (auto&& [key, value] : dict) {
        auto py_key = PythonObject::from(key);
        auto py_value = PythonObject::from(value);
        object.setDictItem(py_key.pyObject(), py_value.pyObject());
    }
    return object;
}

template <typename T>
PythonObject PythonObject::from(const std::vector<T>& value)
{
    PythonObject object { initList(value.size()) };
    for (std::size_t i = 0; i < value.size(); ++i) {
        auto element = PythonObject::from(value[i]);
        object.setListItem(i, element.pyObject());
    }
    return object;
}

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
    } else if constexpr (detail::templates::IsSpecializationV<T, std::vector>) {
        return asArray<T>();
    } else if constexpr (detail::templates::IsSpecializationV<T, std::map>) {
        return asMap<T>();
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

template <typename T>
std::optional<T> PythonObject::asMap()
{
    if (!isDict()) {
        return std::nullopt;
    }

    using KeyType = typename T::key_type;
    using ValueType = typename T::mapped_type;

    T result;
    PythonObject iterator;
    PythonObject item;
    while ((item = getNextItem(iterator))) {
        auto key = item.getItem(0).as<KeyType>();
        auto value = item.getItem(1).as<ValueType>();
        if (key && value) {
            result.emplace(std::move(*key), std::move(*value));
        } else {
            return std::nullopt;
        }
    }
    return result;
}

template <typename T>
std::optional<T> PythonObject::asArray()
{
    if (!isList()) {
        return std::nullopt;
    }

    using ValueType = typename T::value_type;

    T result;
    PythonObject iterator;
    while (auto item = getNextItem(iterator)) {
        if (auto value = item.as<ValueType>()) {
            result.push_back(std::move(*value));
        } else {
            return std::nullopt;
        }
    }
    return result;
}
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_OBJECT_H
