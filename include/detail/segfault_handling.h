#ifndef PPPLUGIN_DETAIL_SEGFAULT_HANDLING_H
#define PPPLUGIN_DETAIL_SEGFAULT_HANDLING_H

#include "detail/scope_guard.h"
#include <iostream>

#if !BOOST_OS_WINDOWS
#include <csignal>
#endif // BOOST_OS_WINDOWS
#include <csetjmp>
#include <mutex>

namespace ppplugin::detail::segfault_handling {
extern std::mutex segfault_checker_mutex;
extern std::jmp_buf segfault_recover_point;

namespace helpers {
    template <typename Func>
    ScopeGuard setSignalHandler(int signal, Func&& func, bool restore_handler)
    {
        struct sigaction catch_segfaults { };
        std::cout << ("register action...\n");
        catch_segfaults.sa_handler = func;
        sigaction(signal, &catch_segfaults, nullptr);

        if (restore_handler) {
            return ScopeGuard { [signal]() {
                sigaction(signal, nullptr, nullptr);
            } };
        }
        return ScopeGuard { []() {} };
    }
} // namespace helpers

template <typename ReturnValue, typename Function, typename... Args>
ReturnValue exec(Function&& function, Args&&... args)
{
    // NOLINTBEGIN(cert-err52-cpp); exceptions are not guaranteed to work in signal handlers
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay); parameter types implementation defined, thus no explicit cast possible
    std::unique_lock<std::mutex> lock_guard { segfault_checker_mutex };
    auto restore_handler_guard = helpers::setSignalHandler(
        SIGSEGV,
        [](int signal_number) {
            if (signal_number == SIGSEGV) {
                std::cout << ("segfault!\n");
                // throw std::runtime_error("segfault...");
                std::longjmp(segfault_recover_point, SIGSEGV); // NOLINT(cert-err52-cpp)
            } else {
                std::cout << ("something else?");
            }
        },
        true);
    if (setjmp(segfault_recover_point) == 0) {
        return function(std::forward<Args>(args)...);
    }
    std::cout << ("plugin caused segfault!\n");

    sigaction(SIGSEGV, nullptr, nullptr);
    return ReturnValue {}; // TODO: handle non-constructible return values
                           // NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
                           // NOLINTEND(cert-err52-cpp)
}
} // namespace ppplugin::detail::segfault_handling

#endif // PPPLUGIN_DETAIL_SEGFAULT_HANDLING_H
