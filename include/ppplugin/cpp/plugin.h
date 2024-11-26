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

private:
    CppPlugin() = default;

private:
    boost::dll::shared_library plugin_;
};
} // namespace ppplugin

#endif // PPPLUGIN_CPP_PLUGIN_H
