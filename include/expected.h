#ifndef PPPLUGIN_EXPECTED_H
#define PPPLUGIN_EXPECTED_H

#include <cassert>
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

#ifdef PPPLUGIN_CPP17_COMPATIBILITY
    template <typename X = T, typename = std::enable_if_t<std::is_default_constructible_v<X>>>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    Expected()
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
        requires std::is_default_constructible_v<T>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    ;
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    Expected(T&& value);
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    Expected(E&& error);
    ~Expected();
    Expected(const Expected&) = default;
    Expected(Expected&&) noexcept = default;
    Expected& operator=(const Expected&) = default;
    Expected& operator=(Expected&&) noexcept = default;
    template <typename U>
    Expected& operator=(U&& rhs);

    [[nodiscard]] bool hasValue() const;
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const;

    [[nodiscard]] std::optional<T> value() const&;
    [[nodiscard]] std::optional<T> value() const&&;
    [[nodiscard]] std::optional<std::reference_wrapper<T>> valueRef();
    [[nodiscard]] std::optional<std::reference_wrapper<const T>> valueRef() const;
    template <typename U>
    [[nodiscard]] T valueOr(U&& default_value) const&;
    template <typename U>
    [[nodiscard]] T valueOr(U&& default_value) const&&;
    template <typename F>
    [[nodiscard]] T valueOrElse(F&& func) const&;
    template <typename F>
    [[nodiscard]] T valueOrElse(F&& func) const&&;

    [[nodiscard]] std::optional<E> error() const;
    [[nodiscard]] std::optional<std::reference_wrapper<E>> errorRef();
    [[nodiscard]] std::optional<std::reference_wrapper<const E>> errorRef() const;
    template <typename U>
    [[nodiscard]] E errorOr(U&& default_value) const&;
    template <typename U>
    [[nodiscard]] E errorOr(U&& default_value) const&&;
    template <typename F>
    [[nodiscard]] E errorOrElse(F&& func) const&;
    template <typename F>
    [[nodiscard]] E errorOrElse(F&& func) const&&;

    template <typename F>
    [[nodiscard]] auto andThen(F&& func) const&;
    template <typename F>
    [[nodiscard]] auto andThen(F&& func) const&&;

    struct BadExpectedAccess : public std::exception { };
    [[nodiscard]] T& operator*();
    const T& operator*() const;
    [[nodiscard]] T* operator->();
    const T* operator->() const;

    [[nodiscard]] T& uncheckedValue();
    [[nodiscard]] const T& uncheckedValue() const;
    [[nodiscard]] E& uncheckedError();
    [[nodiscard]] const E& uncheckedError() const;

#if __cplusplus >= 202101L
    [[nodiscard]] explicit operator std::expected<T, E>() const;
#endif // __cplusplus

    template <typename TL, typename EL, typename TR, typename ER>
    friend bool operator==(const Expected<TL, EL>& lhs, const Expected<TR, ER>& rhs);

private:
    union {
        T t;
        E e;
    };
    bool has_value_;
};

template <typename E>
class Expected<void, E> {
public:
    using Value = void;
    using Error = E;

    Expected() = default;
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    Expected(E&& error);
    ~Expected() = default;
    Expected(const Expected&) = default;
    Expected(Expected&&) noexcept = default;
    Expected& operator=(const Expected&) = default;
    Expected& operator=(Expected&&) noexcept = default;

    [[nodiscard]] bool hasValue() const;
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const;

    void value() const&;
    void value() const&&;
    void valueRef();
    void valueRef() const;
    void valueOr() const&;
    void valueOr() const&&;
    template <typename F>
    void valueOrElse(F&& func) const;

    [[nodiscard]] std::optional<E> error() const;
    [[nodiscard]] std::optional<std::reference_wrapper<E>> errorRef();
    [[nodiscard]] std::optional<std::reference_wrapper<const E>> errorRef() const;
    template <typename U>
    [[nodiscard]] E errorOr(U&& default_value) const&;
    template <typename U>
    [[nodiscard]] E errorOr(U&& default_value) const&&;
    template <typename F>
    [[nodiscard]] E errorOrElse(F&& func) const;

    template <typename F>
    [[nodiscard]] auto andThen(F&& func) const&;
    template <typename F>
    [[nodiscard]] auto andThen(F&& func) const&&;

    void operator*() const;

    void uncheckedValue();
    void uncheckedValue() const;
    [[nodiscard]] E& uncheckedError();
    [[nodiscard]] const E& uncheckedError() const;

#if __cplusplus >= 202101L
    explicit operator std::expected<void, E>() const;
#endif // __cplusplus

    template <typename EL, typename ER>
    friend bool operator==(const Expected<void, EL>& lhs, const Expected<void, ER>& rhs);

private:
    std::optional<E> error_;
};

template <typename T, typename E>
#ifdef PPPLUGIN_CPP17_COMPATIBILITY
template <typename, typename>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
Expected<T, E>::Expected()
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    requires std::is_default_constructible_v<T>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    : t {}
    , has_value_ { true }
{
    static_assert(std::is_default_constructible_v<T>,
        "Value type must be default constructible for a default constructor of Expected");
}
// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
template <typename T, typename E>
Expected<T, E>::Expected(T&& value)
    : t { std::forward<T>(value) }
    , has_value_ { true }
{
}
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
template <typename U>
Expected<T, E>& Expected<T, E>::operator=(U&& rhs)
{
    if constexpr (std::is_convertible_v<U, T>) {
        has_value_ = true;
        t = static_cast<T>(rhs);
    } else if constexpr (std::is_convertible_v<U, E>) {
        has_value_ = false;
        e = static_cast<E>(rhs);
    } else {
        static_assert(!sizeof(U),
            "Type assigned to Expected must be convertible to either its Value or its Error");
    }
    return *this;
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
template <typename F>
auto Expected<T, E>::andThen(F&& func) const&
{
    if (!has_value_) {
        return e;
    }
    if constexpr (std::is_invocable_v<F, T>) {
        return func(t);
    } else if constexpr (std::is_invocable_v<F>) {
        return func();
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}
template <typename T, typename E>
template <typename F>
auto Expected<T, E>::andThen(F&& func) const&&
{
    if (!has_value_) {
        return Expected<void, E> { std::move(e) };
    }
    if constexpr (std::is_invocable_v<F, T>) {

        return func(std::move(t));
    } else if constexpr (std::is_invocable_v<F>) {
        return func();
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

#if __cplusplus >= 202101L
template <typename T, typename E>
Expected<T, E>::operator std::expected<T, E>() const
{
    if (has_value_) {
        return t;
    }
    return e;
}
#endif // __cplusplus

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
template <typename E>
Expected<void, E>::Expected(E&& error)
    : error_ { error }
{
}

template <typename E>
[[nodiscard]] bool Expected<void, E>::hasValue() const
{
    return !error_;
}
template <typename E>
// NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
Expected<void, E>::operator bool() const
{
    return hasValue();
}

template <typename E>
void Expected<void, E>::value() const& { }
template <typename E>
void Expected<void, E>::value() const&& { }
template <typename E>
void Expected<void, E>::valueRef() { }
template <typename E>
void Expected<void, E>::valueRef() const { }
template <typename E>
void Expected<void, E>::valueOr() const& { }
template <typename E>
void Expected<void, E>::valueOr() const&& { }
template <typename E>
template <typename F>
void Expected<void, E>::valueOrElse(F&& func) const
{
    if (error_) {
        func();
    }
}

template <typename E>
std::optional<E> Expected<void, E>::error() const
{
    return error_;
}
template <typename E>
std::optional<std::reference_wrapper<E>> Expected<void, E>::errorRef()
{
    if (error_) {
        return *error_;
    }
    return std::nullopt;
}
template <typename E>
std::optional<std::reference_wrapper<const E>> Expected<void, E>::errorRef() const
{
    if (error_) {
        return *error_;
    }
    return std::nullopt;
}
template <typename E>
template <typename U>
E Expected<void, E>::errorOr(U&& default_value) const&
{
    return error_.value_or(std::forward<U>(default_value));
}
template <typename E>
template <typename U>
E Expected<void, E>::errorOr(U&& default_value) const&&
{
    return error_.value_or(std::forward<U>(default_value));
}
template <typename E>
template <typename F>
E Expected<void, E>::errorOrElse(F&& func) const
{
    if (error_) {
        return *error_;
    }
    return static_cast<E>(func());
}

template <typename E>
template <typename F>
auto Expected<void, E>::andThen(F&& func) const&
{
    if (error_) {
        return *error_;
    }
    return func();
}
template <typename E>
template <typename F>
auto Expected<void, E>::andThen(F&& func) const&&
{
    if (error_) {
        return std::move(*error_);
    }
    return func();
}
template <typename E>
void Expected<void, E>::operator*() const
{
    assert(!error_);
}

template <typename E>
void Expected<void, E>::uncheckedValue() const
{
    assert(!error_);
}
template <typename E>
E& Expected<void, E>::uncheckedError()
{
    return *error_;
}
template <typename E>
const E& Expected<void, E>::uncheckedError() const
{
    return *error_;
}

#if __cplusplus >= 202101L
template <typename E>
explicit Expected<void, E>::operator std::expected<void, E>() const
{
    if (error_) {
        return std::expected<void, E> { *error_ };
    }
    return std::expected<void, E> {};
}
#endif // __cplusplus

template <typename EL, typename ER>
bool operator==(const Expected<void, EL>& lhs, const Expected<void, ER>& rhs)
{
    return lhs.error == rhs.error;
}
} // namespace ppplugin
#endif // PPPLUGIN_EXPECTED_H
