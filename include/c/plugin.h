#ifndef PPPLUGIN_C_PLUGIN_H
#define PPPLUGIN_C_PLUGIN_H

#include "cpp/plugin.h"
#include "detail/segfault_handling.h"

namespace ppplugin {
class CPlugin : public CppPlugin {
public:
    explicit CPlugin(const std::filesystem::path& c_shared_library)
        : CppPlugin { c_shared_library }
    {
    }

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] auto call(const std::string& function_name, Args&&... args)
    {
        return internalCall<false, ReturnValue>(
            function_name, std::forward<Args>(args)...);
    }

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] auto safeCall(const std::string& function_name, Args&&... args)
    {
        return internalCall<true, ReturnValue>(
            function_name, std::forward<Args>(args)...);
    }

    using SignalHandler = void(*)(int);
    template <typename Handler>
    SignalHandler registerSignalHandler(int signal, Handler&& handler)
    {
        auto current_handler = std::find_if(signal_handlers_.begin(), signal_handlers_.end(),
            [signal](auto&& element) { return element.first == signal; });
        SignalHandler previous_handler { nullptr };
        if (current_handler != signal_handlers_.end()) {
            previous_handler = current_handler->second;
        }
        signal_handlers_.push_back({ signal, handler });
        return previous_handler;
    }

protected:
    template <bool catchSegfaults, typename ReturnValue, typename... Args>
    [[nodiscard]] ReturnValue internalCall(const std::string& function_name, Args&&... args)
    {
        // TODO: invalid number of arguments can cause segfault
        static_assert(!std::is_reference_v<ReturnValue>,
            "C does not support references for its return value!");
        static_assert(!(std::is_reference_v<Args> || ...),
            "C does not support references for its arguments! "
            "Consider passing the argument with an explicit cast to the desired type.");
        using FunctionType = ReturnValue (*)(Args...);

        if (!plugin().has(function_name)) {
            throw std::runtime_error("symbol not found"); // TODO
        }
        // cannot use boost here because boost::dll expects the symbol to be
        // a variable (pointing to the function) not a function
#if BOOST_OS_WINDOWS
        auto function = reinterpret_cast<FunctionType>(
            boost::winapi::get_proc_address(plugin().native(), function_name.c_str()));
#else
        // no other way to convert void* to function pointer
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto function = reinterpret_cast<FunctionType>(
            dlsym(plugin().native(), function_name.c_str()));
#endif // BOOST_OS_WINDOWS
        if (!function) {
            throw std::runtime_error("symbol not valid"); // TODO
        }
        if constexpr (catchSegfaults) {
            return detail::segfault_handling::execRecover<ReturnValue>(function, std::forward<Args>(args)...);
        } else {
            if (signal_handlers_.empty()) {
                return detail::segfault_handling::execHandle<ReturnValue>(signal_handlers_,
                    function, std::forward<Args>(args)...);
            }
            return function(std::forward<Args>(args)...);
        }
    }

private:
    std::vector<std::pair<int, SignalHandler>> signal_handlers_;
};
} // namespace ppplugin

#endif // PPPLUGIN_C_PLUGIN_H
