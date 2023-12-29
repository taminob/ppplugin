#ifndef PPPLUGIN_DETAIL_SEGFAULT_HANDLING_H
#define PPPLUGIN_DETAIL_SEGFAULT_HANDLING_H

#include "detail/scope_guard.h"
#include <iostream>

#include <csetjmp>
#include <csignal>
#include <mutex>

namespace ppplugin::detail::segfault_handling {
extern std::mutex segfault_checker_mutex;
extern std::jmp_buf segfault_recover_point;

namespace helpers {
    template <typename Func>
    ScopeGuard setSignalHandler(int signal, Func&& func, bool restore_handler)
    {
        std::cout << ("register action...\n");
        auto previous_handler = std::signal(signal, func);
        if (previous_handler != SIG_ERR && restore_handler) {
            return ScopeGuard { [signal, previous_handler]() {
                // ignore return value; nothing we can do here
                std::ignore = std::signal(signal, previous_handler);
            } };
        }
        return ScopeGuard { []() {} };
    }
} // namespace helpers

template <typename ReturnValue, typename Function, typename... Args>
ReturnValue execRecover(Function&& function, Args&&... args)
{
    // NOLINTBEGIN(cert-err52-cpp); exceptions are not guaranteed to work in signal handlers
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay);
    // parameter type of setjmp implementation defined, thus no explicit cast possible
    std::unique_lock<std::mutex> lock_guard { segfault_checker_mutex };
    auto restore_handler_guard = helpers::setSignalHandler(
        SIGSEGV,
        [](int signal_number) {
            if (signal_number == SIGSEGV) {
                // this is actually undefined behavior in the C++ standard
                std::longjmp(segfault_recover_point, SIGSEGV); // NOLINT(cert-err52-cpp)
            }
        },
        true);
    if (setjmp(segfault_recover_point) == 0) {
        return function(std::forward<Args>(args)...);
    }
    // TODO: handle non-default-constructible return values
    return ReturnValue {};
    // NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    // NOLINTEND(cert-err52-cpp)
}

template <typename ReturnValue, typename Handlers, typename Function, typename... Args>
ReturnValue execHandle(Handlers&& signal_handlers, Function&& function, Args&&... args)
{
    std::unique_lock<std::mutex> lock_guard { segfault_checker_mutex };
    std::vector<ScopeGuard> restore_handler_guards;
    restore_handler_guards.reserve(std::size(signal_handlers));
    for (auto& [signal, handler] : signal_handlers) {
        restore_handler_guards.emplace_back(
            helpers::setSignalHandler(
                signal,
                handler,
                true));
    }
    return function(std::forward<Args>(args)...);
}
} // namespace ppplugin::detail::segfault_handling

#endif // PPPLUGIN_DETAIL_SEGFAULT_HANDLING_H
