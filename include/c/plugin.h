#ifndef PPPLUGIN_C_PLUGIN_H
#define PPPLUGIN_C_PLUGIN_H

#include "cpp/plugin.h"

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
        return function(std::forward<Args>(args)...);
    }
};
} // namespace ppplugin

#endif // PPPLUGIN_C_PLUGIN_H
