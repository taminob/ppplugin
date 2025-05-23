#include "ppplugin/detail/boost_dll_loader.h"
#include "ppplugin/errors.h"

#include <exception>
#include <filesystem>
#include <optional>
#include <string>

#include <boost/dll/shared_library.hpp>
#include <boost/dll/shared_library_load_mode.hpp>
#include <boost/filesystem/path.hpp>

namespace {
[[nodiscard]] std::optional<ppplugin::CallError> isCallable(const boost::dll::shared_library& library, const std::string& function_name)
{
    if (!library.is_loaded()) {
        return ppplugin::CallError::Code::notLoaded;
    }
    // TODO: check ABI compatibility (same compiler + major version)?
    if (!library.has(function_name)) {
        return ppplugin::CallError::Code::symbolNotFound;
    }
    // TODO: invalid number of arguments can cause segfault
    return std::nullopt;
}
} // namespace

namespace ppplugin::detail::boost_dll {
CallResult<void*> getFunctionSymbol(const boost::dll::shared_library& library, const std::string& function_name)
{
    if (auto error = isCallable(library, function_name)) {
        return { *error };
    }
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        if (auto* symbol = reinterpret_cast<void*>(library.get<void()>(function_name))) {
            return symbol;
        }
    } catch (const std::exception& exception) {
        return CallError { CallError::Code::unknown, exception.what() };
    }
    return { CallError::Code::unknown };
}

CallResult<void*> getFunctionPointerSymbol(const boost::dll::shared_library& library, const std::string& function_name)
{
    if (auto error = isCallable(library, function_name)) {
        return { *error };
    }
    try {
        if (auto* symbol = library.get<void*>(function_name)) {
            return symbol;
        }
    } catch (const std::exception& exception) {
        return CallError { CallError::Code::unknown, exception.what() };
    }
    return { CallError::Code::unknown };
}

CallResult<void**> getSymbol(const boost::dll::shared_library& library, const std::string& function_name)
{
    if (auto error = isCallable(library, function_name)) {
        return { *error };
    }
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        if (auto** symbol = &library.get<void*>(function_name)) {
            return symbol;
        }
    } catch (const std::exception& exception) {
        return CallError { CallError::Code::unknown, exception.what() };
    }
    return { CallError::Code::unknown };
}

std::optional<boost::dll::shared_library> loadSharedLibrary(
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
