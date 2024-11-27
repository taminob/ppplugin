#ifndef PPPLUGIN_PLUGIN_H
#define PPPLUGIN_PLUGIN_H

#include "ppplugin/c/plugin.h"
#include "ppplugin/cpp/plugin.h"
#include "ppplugin/errors.h"
#include "ppplugin/lua/plugin.h"
#include "ppplugin/noop_plugin.h"
#include "ppplugin/python/plugin.h"

#include "ppplugin/detail/template_helpers.h"

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
    } -> std::same_as<CallResult<void>>;
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
        std::enable_if_t<(std::is_base_of_v<Plugins, std::remove_cv_t<std::remove_reference_t<P>>> || ...), bool> = true>
    GenericPlugin(P&& plugin); // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)

    ~GenericPlugin() = default;
    GenericPlugin(const GenericPlugin&) = default;
    GenericPlugin(GenericPlugin&&) noexcept = default;
    GenericPlugin& operator=(const GenericPlugin&) = default;
    GenericPlugin& operator=(GenericPlugin&&) noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const;

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] CallResult<ReturnValue> call(const std::string& function_name, Args&&... args);

    template <typename VariableType>
    CallResult<VariableType> global(const std::string& variable_name);
    // TODO: return error
    template <typename VariableType>
    void global(const std::string& variable_name, VariableType&& new_value);

    template <typename P>
    std::optional<std::reference_wrapper<P>> plugin();

private:
    detail::templates::UniqueVariant<Plugins...> plugin_;
};

using Plugin = GenericPlugin<CPlugin, CppPlugin, LuaPlugin, PythonPlugin, NoopPlugin>;

template <typename... Plugins>
template <typename P,
    std::enable_if_t<(std::is_base_of_v<Plugins, std::remove_cv_t<std::remove_reference_t<P>>> || ...), bool>>
// NOLINTNEXTLINE(bugprone-forwarding-reference-overload); enable_if condition prevents hiding copy/move ctors
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
CallResult<ReturnValue> GenericPlugin<Plugins...>::call(
    const std::string& function_name, Args&&... args) // NOLINT(cppcoreguidelines-missing-std-forward)
{
    return std::visit(
        // C++17 compatible trick to forward parameter pack in lambda capture using tuple
        [&function_name, args = std::make_tuple(std::forward<Args>(args)...)](auto& plugin) mutable -> CallResult<ReturnValue> {
            return std::apply([&plugin, &function_name](auto&&... args) {
                return plugin.template call<ReturnValue>(
                    function_name, std::forward<Args>(args)...);
            },
                std::move(args));
        },
        plugin_);
}

template <typename... Plugins>
template <typename VariableType>
CallResult<VariableType> GenericPlugin<Plugins...>::global(const std::string& variable_name)
{
    return std::visit(
        [&variable_name](auto& plugin) -> CallResult<VariableType> {
            return plugin.template global<VariableType>(variable_name);
        },
        plugin_);
}

template <typename... Plugins>
template <typename VariableType>
void GenericPlugin<Plugins...>::global(const std::string& variable_name, VariableType&& new_value)
{
    std::visit(
        [&variable_name, &new_value](auto& plugin) {
            plugin.global(variable_name, std::forward<VariableType>(new_value));
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
