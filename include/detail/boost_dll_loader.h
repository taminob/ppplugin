#ifndef PPPLUGIN_DETAIL_BOOST_DLL_LOADER_H
#define PPPLUGIN_DETAIL_BOOST_DLL_LOADER_H

#include <boost/dll.hpp>

#include <filesystem>
#include <optional>

namespace ppplugin::detail::boost_dll {
inline void* getSymbol(const boost::dll::shared_library& library, const std::string& function_name, bool pointer)
{
    if (!library.is_loaded()) {
        throw std::runtime_error("plugin not loaded");
    }
    // TODO: check ABI compatibility (same compiler + major version)?
    if (!library.has(function_name)) {
        throw std::runtime_error("symbol not found"); // TODO
    }
    // TODO: invalid number of arguments can cause segfault
    if (pointer) {
        return library.get<void*>(function_name);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<void*>(library.get<void()>(function_name));
}

template <typename ReturnValue, bool isPointer, typename... Args>
[[nodiscard]] auto call(const boost::dll::shared_library& library, const std::string& function_name, Args&&... args)
{
    if constexpr (isPointer) {
        using FunctionPointerType = ReturnValue (*)(Args...);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto function = reinterpret_cast<FunctionPointerType>(getSymbol(library, function_name, true));
        if (!function) {
            throw std::runtime_error("symbol type invalid");
        }
        return function(std::forward<Args>(args)...);
    } else {
        using FunctionType = ReturnValue(Args...);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto function = *reinterpret_cast<FunctionType*>(getSymbol(library, function_name, false));
        return function(std::forward<Args>(args)...);
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
