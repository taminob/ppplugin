#ifndef PPPLUGIN_EXPECTED_H
#define PPPLUGIN_EXPECTED_H

#include <optional>
#if __cplusplus >= 202101L
#include <expected>
#endif // __cplusplus

namespace ppplugin {
template <typename T, typename E>
class Expected {
    static_assert(!std::is_same_v<T, E>,
        "Value and Error cannot have the same type for Expected!");

public:
    explicit Expected(T&& value)
        : t { std::forward<T>(value) }
        , has_value_ { true }
    {
    }
    explicit Expected(E&& error)
        : e { std::forward<E>(error) }
        , has_value_ { false }
    {
    }
    ~Expected()
    {
        if (has_value_) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                t.~T();
            }
        } else {
            if constexpr (!std::is_trivially_destructible_v<E>) {
                e.~E();
            }
        }
    }

    [[nodiscard]] bool hasValue() const { return has_value_; }
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const
    {
        return has_value_;
    }

    std::optional<std::reference_wrapper<T>> value()
    {
        if (has_value_) {
            return t;
        }
        return std::nullopt;
    }
    std::optional<std::reference_wrapper<const T>> value() const
    {
        if (has_value_) {
            return t;
        }
        return std::nullopt;
    }

    std::optional<std::reference_wrapper<E>> error()
    {
        if (!has_value_) {
            return e;
        }
        return std::nullopt;
    }
    std::optional<std::reference_wrapper<const E>> error() const
    {
        if (!has_value_) {
            return e;
        }
        return std::nullopt;
    }

    T& operator*() { return t; }
    const T& operator*() const { return t; }
    T* operator->() { return &t; }
    const T* operator->() const { return &t; }

    friend bool operator==(const Expected& lhs, const Expected& rhs)
    {
        if (lhs.has_value_ != lhs.has_value_) {
            return false;
        }
        if (lhs.has_value_) {
            return lhs->t == rhs->t;
        }
        return lhs->e == rhs->e;
    }

    using Value = T;
    using Error = E;

private:
    union {
        T t;
        E e;
    };
    bool has_value_;
};
} // namespace ppplugin
#endif // PPPLUGIN_EXPECTED_H
