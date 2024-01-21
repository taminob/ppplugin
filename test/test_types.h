#ifndef PPPLUGIN_TEST_TYPES_H
#define PPPLUGIN_TEST_TYPES_H

#include <functional>
#include <type_traits>
#include <utility>

namespace ppplugin::test {
class Copyable {
    std::function<void()> on_copy_construction_callback_;
    std::function<void()> on_copy_assignment_callback_;
    int copy_constructed_ {};
    int copy_assigned_ {};

public:
    Copyable() = default;
    explicit Copyable(const std::function<void()>& func)
        : on_copy_construction_callback_ { func }
        , on_copy_assignment_callback_ { func }
    {
    }
    explicit Copyable(std::function<void()> copy_construct_callback,
        std::function<void()> copy_assign_callback)
        : on_copy_construction_callback_ { std::move(copy_construct_callback) }
        , on_copy_assignment_callback_ { std::move(copy_assign_callback) }
    {
    }
    virtual ~Copyable() = default;
    Copyable(const Copyable& rhs)
        : copy_constructed_ { rhs.copy_constructed_ }
        , copy_assigned_ { rhs.copy_assigned_ }
    {
        ++copy_constructed_;
        if (on_copy_construction_callback_) {
            on_copy_construction_callback_();
        }
    }
    Copyable(Copyable&&) = delete;
    Copyable& operator=(const Copyable& rhs)
    {
        copy_constructed_ = rhs.copy_constructed_;
        copy_assigned_ = rhs.copy_assigned_;
        ++copy_assigned_;
        if (on_copy_assignment_callback_) {
            on_copy_assignment_callback_();
        }
        return *this;
    }
    Copyable& operator=(Copyable&&) = delete;

    [[nodiscard]] constexpr int copied() const { return copy_constructed_ + copy_assigned_; }
    [[nodiscard]] constexpr int copyConstructed() const { return copy_constructed_; }
    [[nodiscard]] constexpr int copyAssigned() const { return copy_assigned_; }
};

class Movable {
    std::function<void()> on_move_construction_callback_;
    std::function<void()> on_move_assignment_callback_;
    int move_constructed_ {};
    int move_assigned_ {};

public:
    Movable() = default;
    explicit Movable(const std::function<void()>& move_callback)
        : on_move_construction_callback_ { move_callback }
        , on_move_assignment_callback_ { move_callback }
    {
    }
    explicit Movable(std::function<void()> move_construct_callback,
        std::function<void()> move_assign_callback)
        : on_move_construction_callback_ { std::move(move_construct_callback) }
        , on_move_assignment_callback_ { std::move(move_assign_callback) }
    {
    }
    virtual ~Movable() = default;
    Movable(const Movable&) = delete;
    Movable(Movable&& rhs)
        : move_constructed_ { rhs.move_constructed_ }
        , move_assigned_ { rhs.move_assigned_ }
    {
        ++move_constructed_;
        if (on_move_construction_callback_) {
            on_move_construction_callback_();
        }
    }
    Movable& operator=(const Movable&) = delete;
    Movable& operator=(Movable&& rhs)
    {
        move_constructed_ = rhs.move_constructed_;
        move_assigned_ = rhs.move_assigned_;
        ++move_assigned_;
        if (on_move_assignment_callback_) {
            on_move_assignment_callback_();
        }
        return *this;
    }

    [[nodiscard]] constexpr int moved() const { return move_constructed_ + move_assigned_; }
    [[nodiscard]] constexpr int moveConstructed() const { return move_constructed_; }
    [[nodiscard]] constexpr int moveAssigned() const { return move_assigned_; }
};

struct CopyAndMovable : public Copyable, public Movable {
    CopyAndMovable() = default;
    explicit CopyAndMovable(const std::function<void()>& copy_or_move_callback)
        : Copyable { copy_or_move_callback }
        , Movable { copy_or_move_callback }
    {
    }
    explicit CopyAndMovable(const std::function<void()>& copy_callback,
        const std::function<void()>& move_callback)
        : Copyable { copy_callback }
        , Movable { move_callback }
    {
    }
    explicit CopyAndMovable(
        std::function<void()> copy_ctor_callback,
        std::function<void()> copy_assign_callback,
        std::function<void()> move_ctor_callback,
        std::function<void()> move_assign_callback)
        : Copyable { std::move(copy_ctor_callback), std::move(copy_assign_callback) }
        , Movable { std::move(move_ctor_callback), std::move(move_assign_callback) }
    {
    }
    ~CopyAndMovable() override = default;
    CopyAndMovable(const CopyAndMovable& rhs)
        : Copyable { rhs }
        , Movable {} // NOLINT(readability-redundant-member-init); GCC will emit a warning otherwise
    {
    }
    CopyAndMovable(CopyAndMovable&& rhs)
        : Movable { std::move(rhs) }
    {
    }
    CopyAndMovable& operator=(const CopyAndMovable& rhs)
    {
        Copyable::operator=(rhs);
        return *this;
    }
    CopyAndMovable& operator=(CopyAndMovable&& rhs)
    {
        Movable::operator=(std::move(rhs));
        return *this;
    }
};

struct NonCopyable {
    NonCopyable() = default;
    virtual ~NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable& operator=(NonCopyable&&) = default;
};
static_assert(!std::is_copy_constructible_v<NonCopyable>
    && !std::is_copy_assignable_v<NonCopyable>);

struct NonMovable {
    NonMovable() = default;
    virtual ~NonMovable() = default;
    NonMovable(const NonMovable&) = default;
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(const NonMovable&) = default;
    NonMovable& operator=(NonMovable&&) = delete;
};
static_assert(!std::is_move_constructible_v<NonMovable>
    && !std::is_move_assignable_v<NonMovable>);

struct NonCopyOrMovable : public NonCopyable, public NonMovable { };
static_assert(!std::is_copy_constructible_v<NonCopyOrMovable>
    && !std::is_copy_assignable_v<NonCopyOrMovable>
    && !std::is_move_constructible_v<NonCopyOrMovable>
    && !std::is_move_assignable_v<NonCopyOrMovable>);

struct NonDefaultConstructible {
    NonDefaultConstructible() = delete;
    template <typename Arg, typename... Args>
    explicit NonDefaultConstructible(Arg, Args...) { }
};
static_assert(!std::is_default_constructible_v<NonDefaultConstructible>);

class NonTrivialDestructible {
    std::function<void()> on_destruction_callback_;

public:
    NonTrivialDestructible() = default;
    explicit NonTrivialDestructible(decltype(on_destruction_callback_) func)
        : on_destruction_callback_ { std::move(func) }
    {
    }
    ~NonTrivialDestructible()
    {
        if (on_destruction_callback_) {
            on_destruction_callback_();
        }
    }
    NonTrivialDestructible(const NonTrivialDestructible&) = default;
    NonTrivialDestructible(NonTrivialDestructible&&) = default;
    NonTrivialDestructible& operator=(const NonTrivialDestructible&) = default;
    NonTrivialDestructible& operator=(NonTrivialDestructible&&) = default;
};
static_assert(!std::is_trivially_destructible_v<NonTrivialDestructible>);
} // namespace ppplugin::test

#endif // PPPLUGIN_TEST_TYPES_H
