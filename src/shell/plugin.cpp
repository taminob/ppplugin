#include "ppplugin/shell/plugin.h"

#include "ppplugin/errors.h"
#include "ppplugin/expected.h"

namespace ppplugin {
Expected<ShellPlugin, LoadError> ShellPlugin::load(
    const std::filesystem::path& plugin_library_path)
{
    if (!std::filesystem::exists(plugin_library_path)) {
        return LoadError { LoadErrorCode::fileNotFound };
    }

    ShellPlugin plugin;
    ;
    if (auto result = plugin.call<void>(".", plugin_library_path.string());
        !result.hasValue()) {
        return LoadError { LoadErrorCode::unknown, result.error().what() };
    }
    return plugin;
}
} // namespace ppplugin
