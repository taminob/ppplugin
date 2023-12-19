#ifndef PPPLUGIN_TEST_TYPES_H
#define PPPLUGIN_TEST_TYPES_H

#include <functional>
#include <type_traits>
#include <utility>

namespace ppplugin::test {
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
