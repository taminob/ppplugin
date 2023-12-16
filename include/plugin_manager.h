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
    template <typename Plugin>
    std::pair<std::shared_ptr<Plugin>, PluginLoadingError> loadLuaPlugin(
        const std::filesystem::path& plugin_library_path)
    {
        static_assert((std::is_base_of_v<Plugins, Plugin> || ...),
            "Plugin or one of its bases has to be registered in PluginManager");
        auto new_plugin = std::make_shared<Plugin>(plugin_library_path);
        plugins_.push_back({ PluginVariant {new_plugin}, std::nullopt });
        return {  new_plugin, PluginLoadingError::none };
    }

    /**
     * Load a C++ plugin from given library path.
     * The plugin has to be returned by the given function wrapped in a std::shared_ptr
     * and the given function has to accept all given additional arguments.
     */
    template <typename Plugin, typename... Args>
    std::pair<std::shared_ptr<Plugin>, PluginLoadingError> loadCppPlugin(
        const std::filesystem::path& plugin_library_path,
        const std::string& create_function_name,
        Args... args)
    {
        static_assert((std::is_base_of_v<Plugins, Plugin> || ...),
            "Plugin or one of its bases has to be registered in PluginManager");
        return internalLoadCppPlugin<Plugin, std::shared_ptr<Plugin>(Args...)>(
            plugin_library_path, create_function_name, args...);
    }

protected:
    /**
     * Load any plugin of given type with given creation function and arguments.
     */
    template <typename PluginType,
        typename CreateFunction,
        typename... Args>
    std::pair<std::shared_ptr<PluginType>, PluginLoadingError> internalLoadCppPlugin(
        const std::filesystem::path& plugin_library_path,
        const std::string& create_function_name,
        Args&&... create_function_args)
    {
        if (!std::filesystem::exists(plugin_library_path)) {
            return { nullptr, PluginLoadingError::notFound };
        }

        boost::dll::shared_library plugin_lib { boost::dll::fs::path { plugin_library_path } };
        if (!plugin_lib.has(create_function_name)) {
            return { nullptr, PluginLoadingError::symbolNotFound };
        }
        auto plugin_creator = boost::dll::import_alias<CreateFunction>(
            boost::dll::fs::path { plugin_library_path }, create_function_name,
            boost::dll::load_mode::append_decorations);

        // TODO: invalid number of arguments can cause segfault?
        // TODO: check ABI compatibility
        std::shared_ptr<PluginType> plugin = plugin_creator(create_function_args...);
        if (!plugin) {
            return { nullptr, PluginLoadingError::loadingFailed };
        }

        plugins_.push_back({ PluginVariant { plugin }, plugin_lib });

        return { plugin, PluginLoadingError::none };
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
