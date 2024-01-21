#ifndef PPPLUGIN_NOOP_PLUGIN_H
#define PPPLUGIN_NOOP_PLUGIN_H

#include "errors.h"

#include <string_view>

namespace ppplugin {
class NoopPlugin {
public:
    NoopPlugin() = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const { return true; }

    template <typename ReturnValue, typename... Args>
    CallResult<ReturnValue> call(std::string_view /*function_name*/, Args&&... /*args*/)
    {
        if constexpr (!std::is_void_v<ReturnValue>) {
            if constexpr (std::is_default_constructible_v<ReturnValue>) {
                return ReturnValue {};
            } else {
                static_assert(!sizeof(ReturnValue),
                    "Return value of noop function must be default constructible!");
            }
        }
        return CallResult<ReturnValue> {};
    }
};
} // namespace ppplugin

#endif // PPPLUGIN_NOOP_PLUGIN_H
