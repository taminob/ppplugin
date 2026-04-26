#ifndef PPPLUGIN_PLUGIN_MANAGER_H
#define PPPLUGIN_PLUGIN_MANAGER_H

#include "ppplugin/c/plugin.h"
#include "ppplugin/cpp/plugin.h"
#include "ppplugin/errors.h"
#include "ppplugin/expected.h"
#include "ppplugin/lua/plugin.h"
#include "ppplugin/plugin.h"
#include "ppplugin/python/plugin.h"
#include "ppplugin/shell/plugin.h"

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <map>
#include <mutex>
#include <thread>

namespace ppplugin {
template <bool enablePluginReload>
class PluginManagerTemplate {
public:
    template <typename P>
    class ReloadGuard {
    public:
        using PluginType = P;

    public:
        explicit ReloadGuard(std::unique_ptr<PluginType>* plugin)
            : mutex_ { std::make_shared<std::mutex>() }
            , plugin_ { plugin }
        {
        }

        PluginType* operator->()
        {
            auto guard = lock();
            assert(plugin_);
            return plugin_->get();
        }
        const PluginType* operator->() const
        {
            auto guard = lock();
            assert(plugin_);
            return plugin_->get();
        }

        [[nodiscard]] std::lock_guard<std::mutex> lock()
        {
            assert(mutex_);
            return std::lock_guard<std::mutex> { *mutex_ };
        }

    private:
        std::shared_ptr<std::mutex> mutex_;
        std::unique_ptr<PluginType>* plugin_;
    };
    template <typename P>
    using LoadedPlugin = std::conditional_t<enablePluginReload, ReloadGuard<P>, P>;

public:
    explicit PluginManagerTemplate(bool auto_reload = false, std::chrono::milliseconds file_check_interval = defaultFileCheckInterval());

    virtual ~PluginManagerTemplate() = default;
    PluginManagerTemplate(const PluginManagerTemplate&) = delete;
    PluginManagerTemplate(PluginManagerTemplate&&) noexcept = delete;
    PluginManagerTemplate& operator=(const PluginManagerTemplate&) = delete;
    PluginManagerTemplate& operator=(PluginManagerTemplate&&) noexcept = delete;

    /**
     * Load Lua script from given path.
     *
     * @note this is identical to load<LuaPlugin>
     */
    [[nodiscard]] Expected<LoadedPlugin<LuaPlugin>, LoadError> loadLuaPlugin(
        const std::filesystem::path& plugin_path);

    /**
     * Load Python script from given path.
     *
     * @note this is identical to load<PythonPlugin>
     */
    [[nodiscard]] Expected<LoadedPlugin<PythonPlugin>, LoadError> loadPythonPlugin(
        const std::filesystem::path& plugin_path);

    /**
     * Load POSIX shell script from given path.
     *
     * @note this is identical to load<ShellPlugin>
     */
    [[nodiscard]] Expected<LoadedPlugin<ShellPlugin>, LoadError> loadShellPlugin(
        const std::filesystem::path& plugin_path);

    /**
     * Load a C++ plugin from given library path.
     * This plugin can call functions from the shared library that were exported
     * using BOOST_DLL_ALIAS or have manually as extern declared function pointers.
     *
     * @attention The returned object has to be kept alive until all objects
     *            created by the plugin are fully destroyed.
     *            Failure to do so will result in a SEGFAULT.
     *
     * @note this is identical to load<CppPlugin>
     */
    [[nodiscard]] Expected<LoadedPlugin<CppPlugin>, LoadError> loadCppPlugin(
        const std::filesystem::path& plugin_path);

    /**
     * Load a C plugin from given library path.
     * This plugin can call any C function or C++ functions that were marked
     * as 'extern "C"'.
     *
     * @note this is identical to load<CPlugin>
     */
    [[nodiscard]] Expected<LoadedPlugin<CPlugin>, LoadError> loadCPlugin(
        const std::filesystem::path& plugin_path);

    /**
     * Load and register plugin from provided path.
     */
    template <typename P>
    [[nodiscard]] Expected<LoadedPlugin<P>, LoadError> load(const std::filesystem::path& plugin_path);

private:
    /**
     * Reload all plugins associated with the given file path.
     */
    void reloadPlugin(const std::filesystem::path& plugin_path);

    /**
     * Continuously monitor filesystem for changes to any of the stored
     * plugin files.
     */
    void monitorFiles();

private:
    static constexpr std::chrono::milliseconds defaultFileCheckInterval() { return std::chrono::milliseconds { 500 }; }

private:
    /**
     * Variant that may contain any plugin P as LoadedPlugin<P>.
     */
    using LoadedAnyPlugin = detail::templates::WrapParameterPack<std::variant, LoadedPlugin, CPlugin, CppPlugin, LuaPlugin, PythonPlugin, ShellPlugin, NoopPlugin>;
    /**
     * Collection of all loaded plugins.
     * Store a copy of each loaded plugin; this is used for auto-reloading and
     * as a safeguard for C and C++ plugins to keep a reference to their shared libraries
     * to avoid that the library will be unloaded; this might cause segfaults, e.g.
     * because the destructors cannot be resolved anymore.
     *
     * @note any access to this member must be synchronized using mutex_
     */
    std::multimap<std::filesystem::path, std::unique_ptr<Plugin>> plugins_;
    /**
     * Keep copy of LoadedPlugin to access its mutex.
     */
    std::unordered_map<Plugin*, LoadedAnyPlugin> loaded_plugins_;

    /**
     * Auto-reload plugins on change on disk.
     */
    bool auto_reload_;
    /**
     * Thread to continuously observe the stored paths for changes.
     */
    std::thread file_watch_thread_;
    /**
     * Interval of polling to detect file changes.
     */
    std::chrono::milliseconds file_check_interval_;
    /**
     * Mutex protecting all members that are accessed by multiple threads.
     */
    std::mutex mutex_;
    std::condition_variable file_check_wait_;
    /*
     * Flag indicating if the monitoring of the filesystem should be stopped.
     *
     * @note any access to this member must be synchronized using mutex_
     */
    bool stop_file_checking_ { false };
};

using PluginManager = PluginManagerTemplate<false>;
using PluginReloadManager = PluginManagerTemplate<true>;
} // namespace ppplugin

#endif // PPPLUGIN_PLUGIN_MANAGER_H
