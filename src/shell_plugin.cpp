#include "ppplugin/shell/plugin.h"

#include "ppplugin/errors.h"
#include "ppplugin/expected.h"

#include <boost/parser/parser.hpp>

namespace ppplugin {
Expected<ShellPlugin, LoadError> ShellPlugin::load(
    const std::filesystem::path& plugin_library_path)
{
    ShellPlugin plugin;
    // TODO: handle error
    std::ignore = plugin.call<void>(".", plugin_library_path.string());
    return plugin;
}
} // namespace ppplugin
