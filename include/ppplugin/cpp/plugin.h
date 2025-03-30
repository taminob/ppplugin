#ifndef PPPLUGIN_CPP_PLUGIN_H
#define PPPLUGIN_CPP_PLUGIN_H

#include "ppplugin/detail/boost_dll_loader.h"
#include "ppplugin/errors.h"

#include <boost/dll.hpp>

#include <filesystem>
#include <string>

namespace ppplugin {
class CppPlugin {
public:
    static Expected<CppPlugin, LoadError> load(
        const std::filesystem::path& plugin_library_path)
    {
        if (auto shared_library = detail::boost_dll::loadSharedLibrary(plugin_library_path)) {
            CppPlugin new_plugin;
            new_plugin.plugin_ = *shared_library;
            return new_plugin;
        }
        return LoadError::unknown;
    }

    ~CppPlugin() = default;
    CppPlugin(const CppPlugin&) = default;
    CppPlugin(CppPlugin&&) = default;
    CppPlugin& operator=(const CppPlugin&) = default;
    CppPlugin& operator=(CppPlugin&&) = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const
    {
        return plugin_.is_loaded();
    }

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] CallResult<ReturnValue> call(const std::string& function_name, Args&&... args)
    {
        return detail::boost_dll::call<true, ReturnValue>(
            plugin_, function_name, std::forward<Args>(args)...);
    }

    template <typename VariableType>
    CallResult<VariableType> global(const std::string& variable_name)
    {
        auto p = detail::boost_dll::getSymbol(plugin_, variable_name);
        if (p.hasValue()) {
            return *reinterpret_cast<VariableType*>(p.value().value());
        }
        return { p.error().value() };
    }
    template <typename VariableType>
    CallResult<void> global(const std::string& variable_name, VariableType&& new_value)
    {
        auto p = detail::boost_dll::getSymbol(plugin_, variable_name);
        if (p.hasValue()) {
            *reinterpret_cast<VariableType*>(p.value().value()) = std::forward<VariableType>(new_value);
            return {};
        }
        return { p.error().value() };
    }

private:
    CppPlugin() = default;

private:
    boost::dll::shared_library plugin_;
};
} // namespace ppplugin

#endif // PPPLUGIN_CPP_PLUGIN_H
