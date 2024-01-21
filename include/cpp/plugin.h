#ifndef PPPLUGIN_CPP_PLUGIN_H
#define PPPLUGIN_CPP_PLUGIN_H

#include "detail/boost_dll_loader.h"
#include "errors.h"

#include <boost/dll.hpp>

#include <filesystem>
#include <string>

namespace ppplugin {
class CppPlugin {
public:
    explicit CppPlugin(const std::filesystem::path& cpp_shared_library)
    {
        if (auto shared_library = detail::boost_dll::loadSharedLibrary(cpp_shared_library)) {
            plugin_ = *shared_library;
        }
        // TODO: error handling
    }
    virtual ~CppPlugin() = default;
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
        return detail::boost_dll::call<ReturnValue, true>(
            plugin_, function_name, std::forward<Args>(args)...);
    }

private:
    boost::dll::shared_library plugin_;
};
} // namespace ppplugin

#endif // PPPLUGIN_CPP_PLUGIN_H
