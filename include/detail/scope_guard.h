#ifndef PPPLUGIN_DETAIL_SCOPE_GUARD_H
#define PPPLUGIN_DETAIL_SCOPE_GUARD_H

#include <functional>
namespace ppplugin::detail {
class ScopeGuard final {
public:
    template <typename Func,
        typename = std::enable_if_t<!std::is_same_v<Func, ScopeGuard>>>
    explicit ScopeGuard(Func&& func)
        : function_ { func }
    {
    }
    ~ScopeGuard() { call(); }
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) noexcept
    {
        call();
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer); function has to be called first
        function_ = std::move(other.function_);
    }
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&& other) noexcept
    {
        if (this != &other) {
            call();
            function_ = std::move(other.function_);
        }
        return *this;
    }

    [[nodiscard]] bool isActive() const { return static_cast<bool>(function_); }
    void call()
    {
        if (isActive()) {
            function_();
        }
        cancel();
    }
    void cancel() { function_ = {}; }

private:
    std::function<void()>
        function_;
};
} // namespace ppplugin::detail

#endif // PPPLUGIN_DETAIL_SCOPE_GUARD_H
