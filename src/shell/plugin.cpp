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
    auto result = plugin.call<void>(".", plugin_library_path.string());
    if (result.hasValue()) {
        return plugin;
    }
    return LoadError { LoadErrorCode::unknown, result.error().value().what() };
}
} // namespace ppplugin
