#ifndef PPPLUGIN_C_PLUGIN_H
#define PPPLUGIN_C_PLUGIN_H

#include "detail/boost_dll_loader.h"

namespace ppplugin {
class CPlugin {
public:
    explicit CPlugin(const std::filesystem::path& c_shared_library)
    {
        if (auto shared_library = detail::boost_dll::loadSharedLibrary(c_shared_library)) {
            plugin_ = *shared_library;
        }
    }

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] auto call(const std::string& function_name, Args&&... args)
    {
        static_assert(!std::is_reference_v<ReturnValue>,
            "C does not support references for its return value!");
        static_assert(!(std::is_reference_v<Args> || ...),
            "C does not support references for its arguments! "
            "Consider passing the argument with an explicit cast to the desired type.");

        return detail::boost_dll::call<ReturnValue, false>(
            plugin_, function_name, std::forward<Args>(args)...);
    }

private:
    boost::dll::shared_library plugin_;
};
} // namespace ppplugin

#endif // PPPLUGIN_C_PLUGIN_H
