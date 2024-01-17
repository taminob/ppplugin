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
    using Value = T;
    using Error = E;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    Expected(T&& value);
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    Expected(E&& error);
    ~Expected();
    Expected(const Expected&) = default;
    Expected(Expected&&) noexcept = default;
    Expected& operator=(const Expected&) = default;
    Expected& operator=(Expected&&) noexcept = default;

    [[nodiscard]] bool hasValue() const;
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const;

    std::optional<T> value() const&;
    std::optional<T> value() const&&;
    std::optional<std::reference_wrapper<T>> valueRef();
    std::optional<std::reference_wrapper<const T>> valueRef() const;
    template <typename U>
    T valueOr(U&& default_value) const&;
    template <typename U>
    T valueOr(U&& default_value) const&&;
    template <typename F>
    T valueOrElse(F&& func) const&;
    template <typename F>
    T valueOrElse(F&& func) const&&;

    std::optional<E> error() const;
    std::optional<std::reference_wrapper<E>> errorRef();
    std::optional<std::reference_wrapper<const E>> errorRef() const;
    template <typename U>
    E errorOr(U&& default_value) const&;
    template <typename U>
    E errorOr(U&& default_value) const&&;
    template <typename F>
    E errorOrElse(F&& func) const&;
    template <typename F>
    E errorOrElse(F&& func) const&&;

    struct BadExpectedAccess : public std::exception { };
    T& operator*();
    const T& operator*() const;
    T* operator->();
    const T* operator->() const;

    T& uncheckedValue();
    const T& uncheckedValue() const;
    E& uncheckedError();
    const E& uncheckedError() const;

    template <typename TL, typename EL, typename TR, typename ER>
    friend bool operator==(const Expected<TL, EL>& lhs, const Expected<TR, ER>& rhs);

private:
    union {
        T t;
        E e;
    };
    bool has_value_;
};

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)

template <typename T, typename E>
Expected<T, E>::Expected(T&& value)
    : t { std::forward<T>(value) }
    , has_value_ { true }
{
}
// NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
template <typename T, typename E>
Expected<T, E>::Expected(E&& error)
    : e { std::forward<E>(error) }
    , has_value_ { false }
{
}
template <typename T, typename E>
Expected<T, E>::~Expected()
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
template <typename T, typename E>
[[nodiscard]] bool Expected<T, E>::hasValue() const
{
    return has_value_;
}

template <typename T, typename E>
Expected<T, E>::operator bool() const { return hasValue(); }

template <typename T, typename E>
std::optional<T> Expected<T, E>::value() const&
{
    if (has_value_) {
        return t;
    }
    return std::nullopt;
}
template <typename T, typename E>
std::optional<T> Expected<T, E>::value() const&&
{
    if (has_value_) {
        return std::move(t);
    }
    return std::nullopt;
}
template <typename T, typename E>
std::optional<std::reference_wrapper<T>> Expected<T, E>::valueRef()
{
    if (has_value_) {
        return t;
    }
    return std::nullopt;
}
template <typename T, typename E>
std::optional<std::reference_wrapper<const T>> Expected<T, E>::valueRef() const
{
    if (has_value_) {
        return t;
    }
    return std::nullopt;
}
template <typename T, typename E>
template <typename U>
T Expected<T, E>::valueOr(U&& default_value) const&
{
    if (has_value_) {
        return t;
    }
    return static_cast<T>(std::forward<U>(default_value));
}
template <typename T, typename E>
template <typename U>
T Expected<T, E>::valueOr(U&& default_value) const&&
{
    if (has_value_) {
        return std::move(t);
    }
    return static_cast<T>(std::forward<U>(default_value));
}
template <typename T, typename E>
template <typename F>
T Expected<T, E>::valueOrElse(F&& func) const&
{
    if (has_value_) {
        return t;
    }
    if constexpr (std::is_invocable_v<F, E>) {
        return static_cast<T>(func(e));
    } else if constexpr (std::is_invocable_v<F>) {
        return static_cast<T>(func());
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}
template <typename T, typename E>
template <typename F>
T Expected<T, E>::valueOrElse(F&& func) const&&
{
    if (has_value_) {
        return std::move(t);
    }
    if constexpr (std::is_invocable_v<F, E>) {
        return static_cast<T>(func(std::move(e)));
    } else if constexpr (std::is_invocable_v<F>) {
        return static_cast<T>(func());
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}

template <typename T, typename E>
std::optional<E> Expected<T, E>::error() const
{
    if (has_value_) {
        return e;
    }
    return std::nullopt;
}
template <typename T, typename E>
std::optional<std::reference_wrapper<E>> Expected<T, E>::errorRef()
{
    if (!has_value_) {
        return e;
    }
    return std::nullopt;
}
template <typename T, typename E>
std::optional<std::reference_wrapper<const E>> Expected<T, E>::errorRef() const
{
    if (!has_value_) {
        return e;
    }
    return std::nullopt;
}
template <typename T, typename E>
template <typename U>
E Expected<T, E>::errorOr(U&& default_value) const&
{
    if (!has_value_) {
        return e;
    }
    return static_cast<E>(std::forward<U>(default_value));
}
template <typename T, typename E>
template <typename U>
E Expected<T, E>::errorOr(U&& default_value) const&&
{
    if (!has_value_) {
        return std::move(e);
    }
    return static_cast<T>(std::forward<U>(default_value));
}
template <typename T, typename E>
template <typename F>
E Expected<T, E>::errorOrElse(F&& func) const&
{
    if (!has_value_) {
        return e;
    }
    if constexpr (std::is_invocable_v<F, T>) {
        return static_cast<E>(func(t));
    } else if constexpr (std::is_invocable_v<F>) {
        return static_cast<E>(func());
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}
template <typename T, typename E>
template <typename F>
E Expected<T, E>::errorOrElse(F&& func) const&&
{
    if (!has_value_) {
        return std::move(e);
    }
    if constexpr (std::is_invocable_v<F, T>) {
        return static_cast<E>(func(std::move(t)));
    } else if constexpr (std::is_invocable_v<F>) {
        return static_cast<E>(func());
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}

template <typename T, typename E>
T& Expected<T, E>::operator*()
{
    if (!has_value_) {
        throw BadExpectedAccess {};
    }
    return t;
}
template <typename T, typename E>
const T& Expected<T, E>::operator*() const
{
    if (!has_value_) {
        throw BadExpectedAccess {};
    }
    return t;
}
template <typename T, typename E>
T* Expected<T, E>::operator->()
{
    if (!has_value_) {
        throw BadExpectedAccess {};
    }
    return &t;
}
template <typename T, typename E>
const T* Expected<T, E>::operator->() const
{
    if (!has_value_) {
        throw BadExpectedAccess {};
    }
    return &t;
}

template <typename T, typename E>
T& Expected<T, E>::uncheckedValue()
{
    return t;
}
template <typename T, typename E>
const T& Expected<T, E>::uncheckedValue() const
{
    return t;
}
template <typename T, typename E>
E& Expected<T, E>::uncheckedError()
{
    return e;
}
template <typename T, typename E>
const E& Expected<T, E>::uncheckedError() const
{
    return e;
}

template <typename TL, typename EL, typename TR, typename ER>
bool operator==(const Expected<TL, EL>& lhs, const Expected<TR, ER>& rhs)
{
    if (lhs.has_value_ != lhs.has_value_) {
        return false;
    }
    if (lhs.has_value_) {
        return lhs->t == rhs->t;
    }
    return lhs->e == rhs->e;
}

// NOLINTEND(cppcoreguidelines-pro-type-union-access)
} // namespace ppplugin
#endif // PPPLUGIN_EXPECTED_H
