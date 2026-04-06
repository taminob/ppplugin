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
    auto result_pointer = detail::boost_dll::getSymbol(plugin_, variable_name);
    if (result_pointer.hasValue()) {
        // raw type casting necessary due to lack of type information in shared library
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return *reinterpret_cast<VariableType*>(result_pointer.value().value());
    }
    return { result_pointer.error().value() };
}

template <typename VariableType>
CallResult<void> CPlugin::global(const std::string& variable_name, VariableType&& new_value)
{
    auto result_pointer = detail::boost_dll::getSymbol(plugin_, variable_name);
    if (result_pointer.hasValue()) {
        // raw type casting necessary due to lack of type information in shared library
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        *reinterpret_cast<VariableType*>(result_pointer.value().value()) = std::forward<VariableType>(new_value);
        return {};
    }
    return { result_pointer.error().value() };
}
} // namespace ppplugin

#endif // PPPLUGIN_C_PLUGIN_H
