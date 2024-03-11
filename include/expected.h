#ifndef PPPLUGIN_EXPECTED_H
#define PPPLUGIN_EXPECTED_H

#include <cassert>
#include <optional>
#include <variant>
#if __cplusplus >= 202101L
#include <expected>
#endif // __cplusplus

namespace ppplugin {
template <typename T, typename E>
class Expected {
    static_assert(!std::is_same_v<T, E>,
        "Value and Error cannot have the same type for Expected!");
    static_assert(!std::is_reference_v<T>,
        "Value of Expected cannot be a reference!");
    static_assert(!std::is_reference_v<E>,
        "Error of Expected cannot be a reference!");

public:
    using Value = T;
    using Error = E;

#ifdef PPPLUGIN_CPP17_COMPATIBILITY
    template <typename X = T, typename = std::enable_if_t<std::is_default_constructible_v<X>>>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    constexpr Expected()
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
        requires std::is_default_constructible_v<T>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    ;
    // NOLINTBEGIN(google-explicit-constructor,hicpp-explicit-conversions)
    constexpr Expected(const T& value);
    constexpr Expected(T&& value);
    constexpr Expected(const E& error);
    constexpr Expected(E&& error);
    ~Expected() = default;
    constexpr Expected(const Expected&) = default;
    constexpr Expected(Expected&&) noexcept = default;
    template <typename U, typename V,
        typename = std::enable_if_t<std::is_convertible_v<U, T> && std::is_convertible_v<V, E>>>
    constexpr Expected(const Expected<U, V>& rhs);
    template <typename U, typename V,
        typename = std::enable_if_t<std::is_nothrow_convertible_v<U, T> && std::is_nothrow_convertible_v<V, E>>>
    constexpr Expected(Expected<U, V>&& rhs) noexcept;
    constexpr Expected& operator=(const Expected&) = default;
    constexpr Expected& operator=(Expected&&) noexcept = default;
    template <typename U, typename V>
    constexpr Expected& operator=(const Expected<U, V>& rhs);
    template <typename U, typename V,
        typename = std::enable_if_t<std::is_convertible_v<U, T> && std::is_convertible_v<V, E>>>
    constexpr Expected& operator=(Expected<U, V>&& rhs) noexcept;
    template <typename U,
        typename = std::enable_if_t<std::is_convertible_v<U, T> || std::is_convertible_v<U, T>>>
    constexpr Expected& operator=(U&& rhs);
    // NOLINTEND(google-explicit-constructor,hicpp-explicit-conversions)

    [[nodiscard]] constexpr bool hasValue() const;
    constexpr explicit operator bool() const;

    [[nodiscard]] constexpr std::optional<T> value() const&;
    [[nodiscard]] constexpr std::optional<T> value() &&;
    [[nodiscard]] constexpr std::optional<std::reference_wrapper<T>> valueRef();
    [[nodiscard]] constexpr std::optional<std::reference_wrapper<const T>> valueRef() const;
    template <typename U>
    [[nodiscard]] constexpr T valueOr(U&& default_value) const&;
    template <typename U>
    [[nodiscard]] constexpr T valueOr(U&& default_value) &&;
    template <typename F>
    [[nodiscard]] constexpr T valueOrElse(F&& func) const&;
    template <typename F>
    [[nodiscard]] constexpr T valueOrElse(F&& func) &&;

    [[nodiscard]] constexpr std::optional<E> error() const;
    [[nodiscard]] constexpr std::optional<std::reference_wrapper<E>> errorRef();
    [[nodiscard]] constexpr std::optional<std::reference_wrapper<const E>> errorRef() const;
    template <typename U>
    [[nodiscard]] constexpr E errorOr(U&& default_value) const&;
    template <typename U>
    [[nodiscard]] constexpr E errorOr(U&& default_value) &&;
    template <typename F>
    [[nodiscard]] constexpr E errorOrElse(F&& func) const&;
    template <typename F>
    [[nodiscard]] constexpr E errorOrElse(F&& func) &&;

    template <typename F>
    [[nodiscard]] constexpr auto andThen(F&& func) const&;
    template <typename F>
    [[nodiscard]] constexpr auto andThen(F&& func) &&;

    [[nodiscard]] constexpr T& operator*();
    [[nodiscard]] constexpr const T& operator*() const;
    [[nodiscard]] constexpr T* operator->();
    [[nodiscard]] constexpr const T* operator->() const;

#if __cplusplus >= 202101L
    [[nodiscard]] explicit operator std::expected<T, E>() const;
#endif // __cplusplus

    template <typename U, typename V>
    friend constexpr bool operator==(const Expected<U, V>& lhs, const Expected<U, V>& rhs);

private:
    template <typename, typename>
    friend class Expected;

    // actually, these functions also have a check, but will throw on error
    [[nodiscard]] constexpr T& uncheckedValue() & { return std::get<T>(value_); }
    [[nodiscard]] constexpr const T& uncheckedValue() const& { return std::get<T>(value_); }
    [[nodiscard]] constexpr T&& uncheckedValue() && { return std::get<T>(std::move(value_)); }
    [[nodiscard]] constexpr E& uncheckedError() & { return std::get<E>(value_); }
    [[nodiscard]] constexpr const E& uncheckedError() const& { return std::get<E>(value_); }
    [[nodiscard]] constexpr E&& uncheckedError() && { return std::get<E>(std::move(value_)); }

    std::variant<T, E> value_;
};

template <typename E>
class Expected<void, E> {
public:
    using Value = void;
    using Error = E;

    constexpr Expected() = default;
    // NOLINTBEGIN(google-explicit-constructor,hicpp-explicit-conversions)
    constexpr Expected(const E& error);
    constexpr Expected(E&& error);
    // NOLINTEND(google-explicit-constructor,hicpp-explicit-conversions)
    ~Expected() = default;
    constexpr Expected(const Expected&) = default;
    constexpr Expected(Expected&&) noexcept = default;
    constexpr Expected& operator=(const Expected&) = default;
    constexpr Expected& operator=(Expected&&) noexcept = default;

    [[nodiscard]] constexpr bool hasValue() const;
    constexpr explicit operator bool() const;

    constexpr void value() const&;
    constexpr void value() &&;
    constexpr void valueRef();
    constexpr void valueRef() const;
    constexpr void valueOr() const&;
    constexpr void valueOr() &&;
    template <typename F>
    constexpr void valueOrElse(F&& func) const;

    [[nodiscard]] constexpr std::optional<E> error() const;
    [[nodiscard]] constexpr std::optional<std::reference_wrapper<E>> errorRef();
    [[nodiscard]] constexpr std::optional<std::reference_wrapper<const E>> errorRef() const;
    template <typename U>
    [[nodiscard]] constexpr E errorOr(U&& default_value) const&;
    template <typename U>
    [[nodiscard]] constexpr E errorOr(U&& default_value) &&;
    template <typename F>
    [[nodiscard]] constexpr E errorOrElse(F&& func) const;

    template <typename F>
    [[nodiscard]] constexpr auto andThen(F&& func) const&;
    template <typename F>
    [[nodiscard]] constexpr auto andThen(F&& func) &&;

    constexpr void operator*() const;

#if __cplusplus >= 202101L
    explicit operator std::expected<void, E>() const;
#endif // __cplusplus

    template <typename EL, typename ER>
    friend constexpr bool operator==(const Expected<void, EL>& lhs, const Expected<void, ER>& rhs);

private:
    std::optional<E> error_;
};

template <typename T, typename E>
#ifdef PPPLUGIN_CPP17_COMPATIBILITY
template <typename, typename>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
constexpr Expected<T, E>::Expected()
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    requires std::is_default_constructible_v<T>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    : value_ { T {} }
{
    static_assert(std::is_default_constructible_v<T>,
        "Value type must be default constructible for a default constructor of Expected");
}
template <typename T, typename E>
constexpr Expected<T, E>::Expected(const T& value)
    : value_ { value }
{
}
template <typename T, typename E>
constexpr Expected<T, E>::Expected(T&& value)
    : value_ { std::move(value) }
{
}
template <typename T, typename E>
constexpr Expected<T, E>::Expected(const E& error)
    : value_ { error }
{
}
template <typename T, typename E>
constexpr Expected<T, E>::Expected(E&& error)
    : value_ { std::move(error) }
{
}
template <typename T, typename E>
template <typename U, typename V, typename>
constexpr Expected<T, E>::Expected(const Expected<U, V>& rhs)
    : value_ { rhs.hasValue()
            ? decltype(value_) { static_cast<T>(rhs.uncheckedValue()) }
            : decltype(value_) { static_cast<E>(rhs.uncheckedError()) } }
{
    static_assert(std::is_convertible_v<U, T> && std::is_convertible_v<V, E>,
        "Incompatible types for Expected construction");
}
template <typename T, typename E>
template <typename U, typename V, typename>
constexpr Expected<T, E>::Expected(Expected<U, V>&& rhs) noexcept
    : value_ { rhs.hasValue()
            ? decltype(value_) { static_cast<T&&>(std::move(rhs).uncheckedValue()) }
            : decltype(value_) { static_cast<E&&>(std::move(rhs).uncheckedError()) } }
{
    static_assert(std::is_convertible_v<U, T> && std::is_convertible_v<V, E>,
        "Incompatible types for Expected construction");
}
template <typename T, typename E>
template <typename U, typename V>
constexpr Expected<T, E>& Expected<T, E>::operator=(const Expected<U, V>& rhs)
{
    static_assert(std::is_convertible_v<U, T> && std::is_convertible_v<V, E>,
        "Incompatible types for Expected assignment");
    if (rhs.hasValue()) {
        value_ = static_cast<T>(rhs.uncheckedValue());
    } else {
        value_ = static_cast<E>(rhs.uncheckedError());
    }
    return *this;
}
template <typename T, typename E>
template <typename U, typename V, typename>
constexpr Expected<T, E>& Expected<T, E>::operator=(Expected<U, V>&& rhs) noexcept
{
    static_assert(std::is_convertible_v<U, T> && std::is_convertible_v<V, E>,
        "Incompatible types for Expected assignment");
    if (rhs.hasValue()) {
        value_ = static_cast<T&&>(std::move(rhs).uncheckedValue());
    } else {
        value_ = static_cast<E&&>(std::move(rhs).uncheckedError());
    }
    return *this;
}
template <typename T, typename E>
template <typename U, typename>
constexpr Expected<T, E>& Expected<T, E>::operator=(U&& rhs)
{
    if constexpr (std::is_convertible_v<U, T>) {
        value_ = static_cast<T>(std::forward<U>(rhs));
    } else if constexpr (std::is_convertible_v<U, E>) {
        value_ = static_cast<E>(std::forward<U>(rhs));
    } else {
        static_assert(!sizeof(U),
            "Type assigned to Expected must be convertible to either its Value or its Error");
    }
    return *this;
}

template <typename T, typename E>
constexpr bool Expected<T, E>::hasValue() const
{
    return std::holds_alternative<T>(value_);
}

template <typename T, typename E>
constexpr Expected<T, E>::operator bool() const { return hasValue(); }

template <typename T, typename E>
constexpr std::optional<T> Expected<T, E>::value() const&
{
    if (hasValue()) {
        return uncheckedValue();
    }
    return std::nullopt;
}
template <typename T, typename E>
constexpr std::optional<T> Expected<T, E>::value() &&
{
    if (hasValue()) {
        return std::move(uncheckedValue());
    }
    return std::nullopt;
}
template <typename T, typename E>
constexpr std::optional<std::reference_wrapper<T>> Expected<T, E>::valueRef()
{
    if (hasValue()) {
        return uncheckedValue();
    }
    return std::nullopt;
}
template <typename T, typename E>
constexpr std::optional<std::reference_wrapper<const T>> Expected<T, E>::valueRef() const
{
    if (hasValue()) {
        return uncheckedValue();
    }
    return std::nullopt;
}
template <typename T, typename E>
template <typename U>
constexpr T Expected<T, E>::valueOr(U&& default_value) const&
{
    if (hasValue()) {
        return uncheckedValue();
    }
    return static_cast<T>(std::forward<U>(default_value));
}
template <typename T, typename E>
template <typename U>
constexpr T Expected<T, E>::valueOr(U&& default_value) &&
{
    if (hasValue()) {
        return std::move(*this).uncheckedValue();
    }
    return static_cast<T>(std::forward<U>(default_value));
}
template <typename T, typename E>
template <typename F>
constexpr T Expected<T, E>::valueOrElse(F&& func) const&
{
    if (hasValue()) {
        return uncheckedValue();
    }
    if constexpr (std::is_invocable_v<F, E>) {
        return static_cast<T>(std::forward<F>(func)(uncheckedError()));
    } else if constexpr (std::is_invocable_v<F>) {
        return static_cast<T>(std::forward<F>(func)());
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}
template <typename T, typename E>
template <typename F>
constexpr T Expected<T, E>::valueOrElse(F&& func) &&
{
    if (hasValue()) {
        return std::move(uncheckedValue());
    }
    if constexpr (std::is_invocable_v<F, E>) {
        return static_cast<T>(std::forward<F>(func)(std::move(*this).uncheckedError()));
    } else if constexpr (std::is_invocable_v<F>) {
        return static_cast<T>(std::forward<F>(func)());
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}

template <typename T, typename E>
constexpr std::optional<E> Expected<T, E>::error() const
{
    if (!hasValue()) {
        return uncheckedError();
    }
    return std::nullopt;
}
template <typename T, typename E>
constexpr std::optional<std::reference_wrapper<E>> Expected<T, E>::errorRef()
{
    if (!hasValue()) {
        return uncheckedError();
    }
    return std::nullopt;
}
template <typename T, typename E>
constexpr std::optional<std::reference_wrapper<const E>> Expected<T, E>::errorRef() const
{
    if (!hasValue()) {
        return uncheckedError();
    }
    return std::nullopt;
}
template <typename T, typename E>
template <typename U>
constexpr E Expected<T, E>::errorOr(U&& default_value) const&
{
    if (!hasValue()) {
        return uncheckedError();
    }
    return static_cast<E>(std::forward<U>(default_value));
}
template <typename T, typename E>
template <typename U>
constexpr E Expected<T, E>::errorOr(U&& default_value) &&
{
    if (!hasValue()) {
        return std::move(uncheckedError());
    }
    return static_cast<T>(std::forward<U>(default_value));
}
template <typename T, typename E>
template <typename F>
constexpr E Expected<T, E>::errorOrElse(F&& func) const&
{
    if (!hasValue()) {
        return uncheckedError();
    }
    if constexpr (std::is_invocable_v<F, T>) {
        return static_cast<E>(std::forward<F>(func)(uncheckedValue()));
    } else if constexpr (std::is_invocable_v<F>) {
        return static_cast<E>(std::forward<F>(func)());
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}
template <typename T, typename E>
template <typename F>
constexpr E Expected<T, E>::errorOrElse(F&& func) &&
{
    if (!hasValue()) {
        return std::move(uncheckedError());
    }
    if constexpr (std::is_invocable_v<F, T>) {
        return static_cast<E>(std::forward<F>(func)(std::move(uncheckedValue())));
    } else if constexpr (std::is_invocable_v<F>) {
        return static_cast<E>(std::forward<F>(func)());
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}

template <typename T, typename E>
template <typename F>
constexpr auto Expected<T, E>::andThen(F&& func) const&
{
    if constexpr (std::is_invocable_v<F, T>) {
        using ReturnType = Expected<std::invoke_result_t<F, T>, E>;
        if (!hasValue()) {
            return static_cast<ReturnType>(uncheckedError());
        }
        return static_cast<ReturnType>(std::forward<F>(func)(uncheckedValue()));
    } else if constexpr (std::is_invocable_v<F>) {
        using ReturnType = Expected<std::invoke_result_t<F>, E>;
        if (!hasValue()) {
            return static_cast<ReturnType>(uncheckedError());
        }
        return static_cast<ReturnType>(std::forward<F>(func)());
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}
template <typename T, typename E>
template <typename F>
constexpr auto Expected<T, E>::andThen(F&& func) &&
{
    if constexpr (std::is_invocable_v<F, T>) {
        using ReturnType = Expected<std::invoke_result_t<F, T>, E>;
        if (!hasValue()) {
            return static_cast<ReturnType>(std::move(*this).uncheckedError());
        }
        if constexpr (std::is_void_v<std::invoke_result_t<F, T>>) {
            std::forward<F>(func)(std::move(*this).uncheckedValue());
            return ReturnType {};
        } else {
            return static_cast<ReturnType>(std::forward<F>(func)(
                std::move(*this).uncheckedValue()));
        }
    } else if constexpr (std::is_invocable_v<F>) {
        using ReturnType = Expected<std::invoke_result_t<F>, E>;
        if (!hasValue()) {
            return static_cast<ReturnType>(std::move(*this).uncheckedError());
        }
        if constexpr (std::is_void_v<std::invoke_result_t<F, T>>) {
            std::forward<F>(func)();
            return ReturnType {};
        } else {
            return static_cast<ReturnType>(std::forward<F>(func)());
        }
    } else {
        static_assert(std::is_invocable_v<F>,
            "Given function has to be invocable!");
    }
}

template <typename T, typename E>
constexpr T& Expected<T, E>::operator*()
{
    return uncheckedValue();
}
template <typename T, typename E>
constexpr const T& Expected<T, E>::operator*() const
{
    return uncheckedValue();
}
template <typename T, typename E>
constexpr T* Expected<T, E>::operator->()
{
    return &uncheckedValue();
}
template <typename T, typename E>
constexpr const T* Expected<T, E>::operator->() const
{
    return &uncheckedValue();
}

#if __cplusplus >= 202101L
template <typename T, typename E>
constexpr Expected<T, E>::operator std::expected<T, E>() const
{
    if (hasValue()) {
        return t;
    }
    return e;
}
#endif // __cplusplus

template <typename U, typename V>
constexpr bool operator==(const Expected<U, V>& lhs, const Expected<U, V>& rhs)
{
    return lhs.value_ == rhs.value_;
}
template <typename TL, typename EL, typename TR, typename ER>
constexpr bool operator!=(const Expected<TL, EL>& lhs, const Expected<TR, ER>& rhs)
{
    return !(lhs == rhs);
}

template <typename E>
constexpr Expected<void, E>::Expected(const E& error)
    : error_ { error }
{
}
template <typename E>
constexpr Expected<void, E>::Expected(E&& error)
    : error_ { std::move(error) }
{
}

template <typename E>
constexpr bool Expected<void, E>::hasValue() const
{
    return !error_;
}
template <typename E>
// NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
constexpr Expected<void, E>::operator bool() const
{
    return hasValue();
}

template <typename E>
constexpr void Expected<void, E>::value() const& { }
template <typename E>
constexpr void Expected<void, E>::value() && { }
template <typename E>
constexpr void Expected<void, E>::valueRef() { }
template <typename E>
constexpr void Expected<void, E>::valueRef() const { }
template <typename E>
constexpr void Expected<void, E>::valueOr() const& { }
template <typename E>
constexpr void Expected<void, E>::valueOr() && { }
template <typename E>
template <typename F>
constexpr void Expected<void, E>::valueOrElse(F&& func) const
{
    if (error_) {
        if constexpr (std::is_invocable_v<F, E>) {
            std::forward<F>(func)(*error_);
        } else if constexpr (std::is_invocable_v<F>) {
            std::forward<F>(func)();
        } else {
            static_assert(std::is_invocable_v<F>,
                "Given function has to be invocable!");
        }
    }
}

template <typename E>
constexpr std::optional<E> Expected<void, E>::error() const
{
    return error_;
}
template <typename E>
constexpr std::optional<std::reference_wrapper<E>> Expected<void, E>::errorRef()
{
    if (error_) {
        return *error_;
    }
    return std::nullopt;
}
template <typename E>
constexpr std::optional<std::reference_wrapper<const E>> Expected<void, E>::errorRef() const
{
    if (error_) {
        return *error_;
    }
    return std::nullopt;
}
template <typename E>
template <typename U>
constexpr E Expected<void, E>::errorOr(U&& default_value) const&
{
    return error_.value_or(std::forward<U>(default_value));
}
template <typename E>
template <typename U>
constexpr E Expected<void, E>::errorOr(U&& default_value) &&
{
    return error_.value_or(std::forward<U>(default_value));
}
template <typename E>
template <typename F>
constexpr E Expected<void, E>::errorOrElse(F&& func) const
{
    if (error_) {
        return *error_;
    }
    return static_cast<E>(std::forward<F>(func)());
}

template <typename E>
template <typename F>
constexpr auto Expected<void, E>::andThen(F&& func) const&
{
    if (error_) {
        return *error_;
    }
    return std::forward<F>(func)();
}
template <typename E>
template <typename F>
constexpr auto Expected<void, E>::andThen(F&& func) &&
{
    if (error_) {
        return std::move(*error_);
    }
    return std::forward<F>(func)();
}
template <typename E>
constexpr void Expected<void, E>::operator*() const
{
    assert(!error_);
}

#if __cplusplus >= 202101L
template <typename E>
constexpr Expected<void, E>::operator std::expected<void, E>() const
{
    if (error_) {
        return std::expected<void, E> { *error_ };
    }
    return std::expected<void, E> {};
}
#endif // __cplusplus

template <typename EL, typename ER>
constexpr bool operator==(const Expected<void, EL>& lhs, const Expected<void, ER>& rhs)
{
    return lhs.error == rhs.error;
}
} // namespace ppplugin
#endif // PPPLUGIN_EXPECTED_H
