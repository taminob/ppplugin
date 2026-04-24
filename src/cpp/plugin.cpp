#include "ppplugin/cpp/plugin.h"

namespace ppplugin {
Expected<CppPlugin, LoadError> CppPlugin::load(const std::filesystem::path& plugin_library_path)
{
    if (!std::filesystem::exists(plugin_library_path)) {
        return LoadError { LoadErrorCode::fileNotFound };
    }
    if (auto shared_library = detail::boost_dll::loadSharedLibrary(plugin_library_path)) {
        CppPlugin new_plugin;
        new_plugin.plugin_ = *shared_library;
        return new_plugin;
    }
    return LoadError { LoadErrorCode::unknown };
}

CppPlugin::operator bool() const
{
    return plugin_.is_loaded();
}
} // namespace ppplugin
