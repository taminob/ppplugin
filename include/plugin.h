#ifndef PPPLUGIN_PLUGIN_H
#define PPPLUGIN_PLUGIN_H

#include "detail/template_helpers.h"

#include <boost/dll.hpp>

#ifndef PPPLUGIN_CPP17_COMPATIBILITY
#include <concepts>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
#include <filesystem>
#include <optional>
#include <utility>
#include <variant>

namespace ppplugin {
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
template <typename P>
concept IsPlugin = requires(P p) {
    {
        std::convertible_to<P, bool>
    };
    // {
    //     p.template call<>(std::declval<int>())
    // } -> std::same_as<std::tuple<>>;
};
#endif // PPPLUGIN_CPP17_COMPATIBILITY

template <typename... Plugins>
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    requires(IsPlugin<Plugins> && ...)
#endif // PPPLUGIN_CPP17_COMPATIBILITY
class GenericPlugin {

public:
    GenericPlugin() = default;
    template <typename P,
        typename = std::enable_if_t<(std::is_base_of_v<Plugins, P> || ...)>>
    GenericPlugin(P&& plugin) // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        : plugin_(std::forward<P>(plugin))
    {
    }

    virtual ~GenericPlugin() = default;
    GenericPlugin(const GenericPlugin&) = delete;
    GenericPlugin(GenericPlugin&&) noexcept = default;
    GenericPlugin& operator=(const GenericPlugin&) = delete;
    GenericPlugin& operator=(GenericPlugin&&) noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    virtual operator bool() const
    {
        return std::visit([](const auto& plugin) -> bool { return plugin; }, plugin_);
    }

    // TODO: add specialization for void return type (empty tuple triggers nodiscard)
    template <typename ReturnValue, typename... Args>
    [[nodiscard]] ReturnValue call(const std::string& function_name, Args&&... args)
    {
        return std::visit(
            [&function_name, &args...](auto& plugin) {
                return plugin.template call<ReturnValue>(
                    function_name, std::forward<Args>(args)...);
            },
            plugin_);
    }

    template <typename P>
    std::optional<P> plugin()
    {
        if (auto* plugin = std::get_if<P>(plugin_)) {
            return *plugin;
        }
        return std::nullopt;
    }

private:
    detail::templates::UniqueVariant<Plugins...> plugin_;
};

class LuaPlugin;
class CppPlugin;
class CPlugin;
using Plugin = GenericPlugin<CPlugin, CppPlugin, LuaPlugin>;
} // namespace ppplugin

#endif // PPPLUGIN_PLUGIN_H
