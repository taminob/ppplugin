#ifndef PPPLUGIN_NOOP_PLUGIN_H
#define PPPLUGIN_NOOP_PLUGIN_H

#include <string_view>

namespace ppplugin {
class NoopPlugin {
public:
    NoopPlugin() = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const { return true; }

    template <typename ReturnValue, typename... Args>
    ReturnValue call(std::string_view /*function_name*/, Args&&... /*args*/)
    {
        if constexpr (!std::is_void_v<ReturnValue>) {
            if constexpr (std::is_default_constructible_v<ReturnValue>) {
                return ReturnValue {};
            } else {
                static_assert(std::is_default_constructible_v<ReturnValue>,
                    "Return value of noop function must be default constructible!");
            }
        }
    }
};
} // namespace ppplugin

#endif // PPPLUGIN_NOOP_PLUGIN_H
