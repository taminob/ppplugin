#ifndef PPPLUGIN_PLUGIN_MANAGER_H
#define PPPLUGIN_PLUGIN_MANAGER_H

#include "c/plugin.h"
#include "cpp/plugin.h"
#include "detail/template_helpers.h"
#include "errors.h"
#include "expected.h"
#include "lua/plugin.h"
#include "plugin.h"
#include "python/plugin.h"

#include <boost/dll.hpp>

#include <filesystem>
#include <vector>

namespace ppplugin {
template <typename... Plugins>
class GenericPluginManager {
    static_assert(sizeof...(Plugins) > 0,
        "Plugin manager requires at least one plugin type!");

public:
    explicit GenericPluginManager(bool auto_reload = false)
        : auto_reload_ { auto_reload }
    {
    }

    virtual ~GenericPluginManager() = default;
    GenericPluginManager(const GenericPluginManager&) = default;
    GenericPluginManager(GenericPluginManager&&) noexcept = default;
    GenericPluginManager& operator=(const GenericPluginManager&) = default;
    GenericPluginManager& operator=(GenericPluginManager&&) noexcept = default;

    /**
     * Load Lua script from given path.
     */
    Expected<LuaPlugin, LoadError> loadLuaPlugin(
        const std::filesystem::path& plugin_library_path)
    {
        return LuaPlugin::load(plugin_library_path);
    }

    /**
     * Load Python script from given path.
     */
    Expected<PythonPlugin, LoadError> loadPythonPlugin(
        const std::filesystem::path& plugin_library_path)
    {
        return PythonPlugin::load(plugin_library_path);
    }

    /**
     * Load a C++ plugin from given library path.
     * This plugin can call functions from the shared library that were exported
     * using BOOST_DLL_ALIAS or have manually as extern declared function pointers.
     *
     * @attention The returned object has to be kept alive until all objects
     *            created by the plugin are fully destroyed.
     *            Failure to do so will result in a SEGFAULT.
     */
    Expected<CppPlugin, LoadError> loadCppPlugin(
        const std::filesystem::path& plugin_library_path)
    {
        return CppPlugin::load(plugin_library_path);
    }
    /**
     * Load a C plugin from given library path.
     * This plugin can call any C function or C++ functions that were marked
     * as 'extern "C"'.
     */
    Expected<CPlugin, LoadError> loadCPlugin(
        const std::filesystem::path& plugin_library_path)
    {
        return CPlugin::load(plugin_library_path);
    }

private:
    using PluginVariant = typename detail::templates::WrapParameterPack<std::variant, std::shared_ptr, Plugins...>::Type;
    // store for each plugin its shared library to avoid that the lib will be
    // unloaded; this might cause segfaults when accessing the plugin
    std::vector<PluginVariant> plugins_;

    /**
     * Auto-reload plugins on change on disk.
     */
    bool auto_reload_; // TODO: implement me :)
};

using PluginManager = GenericPluginManager<Plugin>;
} // namespace ppplugin

#endif // PPPLUGIN_PLUGIN_MANAGER_H
