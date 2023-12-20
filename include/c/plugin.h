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

    template <typename T>
    struct CopyIfReference {
        template <typename = std::enable_if<std::is_reference_v<T>>>
        explicit CopyIfReference(std::remove_reference<T> by_value)
            : value { by_value }
        {
        }
        template <typename = std::enable_if<!std::is_reference_v<T>>>
        explicit CopyIfReference(T&& by_reference)
            : value { by_reference }
        {
        }

        T value;
    };

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
            return detail::segfault_handling::exec<ReturnValue>(function, std::forward<Args>(args)...);
        } else {
            return function(std::forward<Args>(args)...);
        }
    }
};
} // namespace ppplugin

#endif // PPPLUGIN_C_PLUGIN_H
