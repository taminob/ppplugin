#ifndef PPPLUGIN_C_PLUGIN_H
#define PPPLUGIN_C_PLUGIN_H

#include "ppplugin/detail/boost_dll_loader.h"
#include "ppplugin/errors.h"

namespace ppplugin {
class CPlugin {
public:
    [[nodiscard]] static Expected<CPlugin, LoadError> load(const std::filesystem::path& plugin_library_path);

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const;

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] CallResult<ReturnValue> call(const std::string& function_name, Args&&... args);

    template <typename VariableType>
    [[nodiscard]] CallResult<VariableType> global(const std::string& variable_name);
    template <typename VariableType>
    [[nodiscard]] CallResult<void> global(const std::string& variable_name, VariableType&& new_value);

private:
    CPlugin() = default;

private:
    boost::dll::shared_library plugin_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> CPlugin::call(const std::string& function_name, Args&&... args)
{
    static_assert(!std::is_reference_v<ReturnValue>,
        "C does not support references for its return value!");
    static_assert(!(std::is_reference_v<Args> || ...),
        "C does not support references for its arguments! "
        "Consider passing the argument with an explicit cast to the desired type.");

    return detail::boost_dll::call<false, ReturnValue>(
        plugin_, function_name, std::forward<Args>(args)...);
}

// TODO: remove code duplication here and in C++ plugin
template <typename VariableType>
CallResult<VariableType> CPlugin::global(const std::string& variable_name)
{
    auto p = detail::boost_dll::getSymbol(plugin_, variable_name);
    if (p.hasValue()) {
        return *reinterpret_cast<VariableType*>(p.value().value());
    }
    return { p.error().value() };
}

template <typename VariableType>
CallResult<void> CPlugin::global(const std::string& variable_name, VariableType&& new_value)
{
    auto p = detail::boost_dll::getSymbol(plugin_, variable_name);
    if (p.hasValue()) {
        *reinterpret_cast<VariableType*>(p.value().value()) = std::forward<VariableType>(new_value);
        return {};
    }
    return { p.error().value() };
}
} // namespace ppplugin

#endif // PPPLUGIN_C_PLUGIN_H
