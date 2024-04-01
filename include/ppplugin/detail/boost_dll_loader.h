#ifndef PPPLUGIN_DETAIL_BOOST_DLL_LOADER_H
#define PPPLUGIN_DETAIL_BOOST_DLL_LOADER_H

#include "errors.h"

#include <boost/dll.hpp>

#include <filesystem>
#include <optional>

namespace ppplugin::detail::boost_dll {
/**
 * Attempt to load shared library from given path.
 *
 * Returned library object is not necessarily loaded, check
 * boost::dll::shared_library::is_loaded().
 */
[[nodiscard]] std::optional<boost::dll::shared_library> loadSharedLibrary(
    const std::filesystem::path& plugin_library_path);

[[nodiscard]] CallResult<void*> getFunctionSymbol(const boost::dll::shared_library& library, const std::string& function_name);
[[nodiscard]] CallResult<void*> getFunctionPointerSymbol(const boost::dll::shared_library& library, const std::string& function_name);

template <bool isPointer, typename ReturnValue, typename... Args>
[[nodiscard]] CallResult<ReturnValue> call(const boost::dll::shared_library& library, const std::string& function_name, Args&&... args)
{
    if constexpr (isPointer) {
        using FunctionPointerType = ReturnValue (*)(Args...);
        return getFunctionPointerSymbol(library, function_name).andThen([&](void* symbol_address) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            auto function = reinterpret_cast<FunctionPointerType>(symbol_address);
            if constexpr (std::is_void_v<ReturnValue>) {
                function(std::forward<Args>(args)...);
            } else {
                return function(std::forward<Args>(args)...);
            }
        });
    } else {
        using FunctionType = ReturnValue(Args...);
        return getFunctionSymbol(library, function_name).andThen([&](void* symbol_address) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            auto function = *reinterpret_cast<FunctionType*>(symbol_address);
            if constexpr (std::is_void_v<ReturnValue>) {
                function(std::forward<Args>(args)...);
            } else {
                return function(std::forward<Args>(args)...);
            }
        });
    }
}

} // namespace ppplugin::detail::boost_dll

#endif // PPPLUGIN_DETAIL_BOOST_DLL_LOADER_H
