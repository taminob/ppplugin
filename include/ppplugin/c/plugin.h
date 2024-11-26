#ifndef PPPLUGIN_C_PLUGIN_H
#define PPPLUGIN_C_PLUGIN_H

#include "ppplugin/detail/boost_dll_loader.h"
#include "ppplugin/errors.h"

namespace ppplugin {
class CPlugin {
public:
    static Expected<CPlugin, LoadError> load(
        const std::filesystem::path& plugin_library_path)
    {
        if (auto shared_library = detail::boost_dll::loadSharedLibrary(plugin_library_path)) {
            CPlugin new_plugin;
            new_plugin.plugin_ = *shared_library;
            return new_plugin;
        }
        return LoadError::unknown;
    }

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] CallResult<ReturnValue> call(const std::string& function_name, Args&&... args)
    {
        static_assert(!std::is_reference_v<ReturnValue>,
            "C does not support references for its return value!");
        static_assert(!(std::is_reference_v<Args> || ...),
            "C does not support references for its arguments! "
            "Consider passing the argument with an explicit cast to the desired type.");

        return detail::boost_dll::call<false, ReturnValue>(
            plugin_, function_name, std::forward<Args>(args)...);
    }

private:
    CPlugin() = default;

private:
    boost::dll::shared_library plugin_;
};
} // namespace ppplugin

#endif // PPPLUGIN_C_PLUGIN_H
