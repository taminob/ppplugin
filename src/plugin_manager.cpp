#include "ppplugin/plugin_manager.h"

namespace ppplugin {
// convenience helper to simplify definition of member functions
template <bool enablePluginReload, typename P>
using LoadedPlugin = typename PluginManagerTemplate<enablePluginReload>::template LoadedPlugin<P>;

template class PluginManagerTemplate<true>;
template class PluginManagerTemplate<false>;

template <bool enablePluginReload>
PluginManagerTemplate<enablePluginReload>::PluginManagerTemplate(bool auto_reload, std::chrono::milliseconds file_check_interval)
    : auto_reload_ { auto_reload }
    , file_check_interval_ { file_check_interval }
{
}

template <bool enablePluginReload>
Expected<LoadedPlugin<enablePluginReload, LuaPlugin>, LoadError> PluginManagerTemplate<enablePluginReload>::loadLuaPlugin(
    const std::filesystem::path& plugin_path)
{
    return load<LuaPlugin>(plugin_path);
}

template <bool enablePluginReload>
Expected<LoadedPlugin<enablePluginReload, PythonPlugin>, LoadError> PluginManagerTemplate<enablePluginReload>::loadPythonPlugin(
    const std::filesystem::path& plugin_path)
{
    return load<PythonPlugin>(plugin_path);
}

template <bool enablePluginReload>
Expected<LoadedPlugin<enablePluginReload, ShellPlugin>, LoadError> PluginManagerTemplate<enablePluginReload>::loadShellPlugin(
    const std::filesystem::path& plugin_path)
{
    return load<ShellPlugin>(plugin_path);
}

template <bool enablePluginReload>
Expected<LoadedPlugin<enablePluginReload, CppPlugin>, LoadError> PluginManagerTemplate<enablePluginReload>::loadCppPlugin(
    const std::filesystem::path& plugin_path)
{
    return load<CppPlugin>(plugin_path);
}

template <bool enablePluginReload>
Expected<LoadedPlugin<enablePluginReload, CPlugin>, LoadError> PluginManagerTemplate<enablePluginReload>::loadCPlugin(
    const std::filesystem::path& plugin_path)
{
    return load<CPlugin>(plugin_path);
}

template <bool enablePluginReload>
template <typename P>
Expected<LoadedPlugin<enablePluginReload, P>, LoadError> PluginManagerTemplate<enablePluginReload>::load(const std::filesystem::path& plugin_path)
{
    return P::load(plugin_path).andThen([&](auto&& new_plugin) {
        auto plugin = std::make_unique<P>(std::forward<decltype(new_plugin)>(new_plugin));
        auto it = plugins_.emplace(plugin_path, std::move(plugin));

        LoadedPlugin<P> loaded_plugin { &it->second };
        loaded_plugins_.emplace(it->second.get(), loaded_plugin);
        return loaded_plugin;
    });
}

template <bool enablePluginReload>
void PluginManagerTemplate<enablePluginReload>::reloadPlugin(const std::filesystem::path& plugin_path)
{
    if constexpr (enablePluginReload) {
        auto [begin, end] = plugins_.equal_range(plugin_path);
        for (auto it = begin; it != end; ++it) {
            auto loaded_plugin = loaded_plugins_.find(it->second.get());
            if (loaded_plugin == loaded_plugins_.end()) {
                return;
            }
            static_assert(std::is_same_v<decltype(loaded_plugin->second), int>);
            std::visit([&](auto&& loaded_plugin) {
                auto lock = loaded_plugin->second.lock();
                if (auto reloaded_plugin = detail::templates::RemoveCvrefT<decltype(loaded_plugin->second)>::PluginType::load(plugin_path)) {
                    it->second = reloaded_plugin.value();
                }
            },
                loaded_plugin->second);
        }
    }
}

template <bool enablePluginReload>
void PluginManagerTemplate<enablePluginReload>::monitorFiles()
{
    while (!stop_file_checking_) {
        std::unique_lock<std::mutex> lock { mutex_ };
        file_check_wait_.wait_for(lock, file_check_interval_, [&]() {
            return stop_file_checking_;
        });
    }
}
} // namespace ppplugin
