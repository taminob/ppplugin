#ifndef PPPLUGIN_PLUGIN_MANAGER_H
#define PPPLUGIN_PLUGIN_MANAGER_H

#include <boost/dll.hpp>

#include <filesystem>
#include <functional>
#include <variant>

#include "detail/compiler_info.h"
#include "detail/template_helpers.h"
#include "plugin.h"

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
     *
     */
    enum class PluginLoadingError {
        none,
        notFound,
        loadingFailed,
        symbolNotFound,
        unknown
    };

    /**
     * Load Lua script from given path.
     */
    LuaPlugin loadLuaPlugin(
        const std::filesystem::path& plugin_library_path)
    {
        LuaPlugin new_plugin { plugin_library_path };
        return new_plugin;
    }

    /**
     * Load a C++ plugin from given library path.
     * The plugin has to be returned by the given function wrapped in a std::shared_ptr
     * and the given function has to accept all given additional arguments.
     */
    template <typename... Args>
    CppPlugin loadCppPlugin(
        const std::filesystem::path& plugin_library_path)
    {
        return CppPlugin { plugin_library_path };
    }
    template <typename... Args>
    CPlugin loadCPlugin(
        const std::filesystem::path& plugin_library_path)
    {
        return CPlugin { plugin_library_path };
    }

private:
    using PluginVariant = typename detail::templates::WrapParameterPack<std::variant, std::shared_ptr, Plugins...>::Type;
    // store for each plugin its shared library to avoid that the lib will be
    // unloaded; this might cause segfaults when accessing the plugin
    using PluginLibrary = std::pair<PluginVariant, std::optional<boost::dll::shared_library>>;
    std::vector<PluginLibrary> plugins_;

    /**
     * Auto-reload plugins on change on disk.
     */
    bool auto_reload_; // TODO: implement me :)
};

using PluginManager = GenericPluginManager<Plugin>;
} // namespace ppplugin

#endif // PPPLUGIN_PLUGIN_MANAGER_H
