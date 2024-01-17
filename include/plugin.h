#ifndef PPPLUGIN_PLUGIN_H
#define PPPLUGIN_PLUGIN_H

#include "c/plugin.h"
#include "cpp/plugin.h"
#include "lua/plugin.h"
#include "noop_plugin.h"

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
concept IsPlugin = requires(P plugin) {
    {
        std::convertible_to<const P, bool>
    };
    {
        plugin.template call<void>(std::declval<std::string>())
    } -> std::same_as<void>;
};
#endif // PPPLUGIN_CPP17_COMPATIBILITY

template <typename... Plugins>
class GenericPlugin {
public:
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    static_assert((IsPlugin<Plugins> && ...), "All plugins must adhere to the Plugin interface.");
#endif // PPPLUGIN_CPP17_COMPATIBILITY

    GenericPlugin() = default;
    template <typename P,
        typename = std::enable_if_t<(std::is_base_of_v<Plugins, P> || ...)>>
    GenericPlugin(P&& plugin); // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)

    virtual ~GenericPlugin() = default;
    GenericPlugin(const GenericPlugin&) = delete;
    GenericPlugin(GenericPlugin&&) noexcept = default;
    GenericPlugin& operator=(const GenericPlugin&) = delete;
    GenericPlugin& operator=(GenericPlugin&&) noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const;

    template <typename... Args>
    void call(const std::string& function_name, Args&&... args);
    template <typename ReturnValue, typename... Args>
    [[nodiscard]] ReturnValue call(const std::string& function_name, Args&&... args);

    template <typename P>
    std::optional<std::reference_wrapper<P>> plugin();

private:
    detail::templates::UniqueVariant<Plugins...> plugin_;
};

using Plugin = GenericPlugin<CPlugin, CppPlugin, LuaPlugin, NoopPlugin>;

template <typename... Plugins>
template <typename P, typename>
GenericPlugin<Plugins...>::GenericPlugin(P&& plugin)
    : plugin_(std::forward<P>(plugin))
{
}

template <typename... Plugins>
GenericPlugin<Plugins...>::operator bool() const
{
    return std::visit([](const auto& plugin) -> bool { return plugin; }, plugin_);
}

template <typename... Plugins>
template <typename ReturnValue, typename... Args>
ReturnValue GenericPlugin<Plugins...>::call(const std::string& function_name, Args&&... args)
{
    return std::visit(
        [&function_name, &args...](auto& plugin) {
            return plugin.template call<ReturnValue>(
                function_name, std::forward<Args>(args)...);
        },
        plugin_);
}

template <typename... Plugins>
template <typename P>
std::optional<std::reference_wrapper<P>> GenericPlugin<Plugins...>::plugin()
{
    if (auto* plugin = std::get_if<P>(plugin_)) {
        return *plugin;
    }
    return std::nullopt;
}

} // namespace ppplugin

#endif // PPPLUGIN_PLUGIN_H
