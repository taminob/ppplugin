#ifndef PPPLUGIN_DETAIL_BOOST_DLL_LOADER_H
#define PPPLUGIN_DETAIL_BOOST_DLL_LOADER_H

#include "errors.h"
#include "expected.h"

#include <boost/dll.hpp>

#include <filesystem>
#include <optional>

namespace ppplugin::detail::boost_dll {
inline CallResult<void*> getSymbol(const boost::dll::shared_library& library, const std::string& function_name, bool pointer)
{
    if (!library.is_loaded()) {
        return { CallError::Code::notLoaded };
    }
    // TODO: check ABI compatibility (same compiler + major version)?
    if (!library.has(function_name)) {
        return { CallError::Code::symbolNotFound };
    }
    // TODO: invalid number of arguments can cause segfault
    try {
        if (pointer) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            if (auto* symbol = reinterpret_cast<void*>(library.get<void*>(function_name))) {
                return symbol;
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            if (auto* symbol = reinterpret_cast<void*>(library.get<void()>(function_name))) {
                return symbol;
            }
        }
    } catch (const std::exception& exception) {
        return CallError { CallError::Code::unknown, exception.what() };
    }
    return { CallError::Code::unknown };
}

template <typename ReturnValue, bool isPointer, typename... Args>
[[nodiscard]] CallResult<ReturnValue> call(const boost::dll::shared_library& library, const std::string& function_name, Args&&... args)
{
    if constexpr (isPointer) {
        using FunctionPointerType = ReturnValue (*)(Args...);
        return getSymbol(library, function_name, true).andThen([&](void* symbol_address) {
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
        return getSymbol(library, function_name, false).andThen([&](void* symbol_address) {
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

/**
 * Attempt to load shared library from given path.
 *
 * Returned library object is not necessarily loaded, check
 * boost::dll::shared_library::is_loaded().
 */
[[nodiscard]] inline std::optional<boost::dll::shared_library> loadSharedLibrary(
    const std::filesystem::path& plugin_library_path)
{
    if (!std::filesystem::exists(plugin_library_path)) {
        return std::nullopt;
    }

    return boost::dll::shared_library {
        boost::dll::fs::path { plugin_library_path },
        boost::dll::load_mode::append_decorations
    };
}
} // namespace ppplugin::detail::boost_dll

#endif // PPPLUGIN_DETAIL_BOOST_DLL_LOADER_H
