#ifndef PPPLUGIN_CPP_PLUGIN_H
#define PPPLUGIN_CPP_PLUGIN_H

#include "detail/boost_dll_loader.h"

#include <boost/dll.hpp>

#include <filesystem>
#include <string>

namespace ppplugin {
class CppPlugin {
public:
    explicit CppPlugin(const std::filesystem::path& cpp_shared_library)
    {
        if (auto shared_library = detail::boost_dll::loadSharedLibrary(cpp_shared_library)) {
            plugin_ = *shared_library;
        }
        // TODO: error handling
    }
    virtual ~CppPlugin() = default;
    CppPlugin(const CppPlugin&) = default;
    CppPlugin(CppPlugin&&) = default;
    CppPlugin& operator=(const CppPlugin&) = default;
    CppPlugin& operator=(CppPlugin&&) = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const
    {
        return loading_error_ == LoadingError::none && runtime_error_ == RuntimeError::none;
    }

    enum class LoadingError {
        none,
        notFound,
        loadingFailed,
        symbolNotFound,
        symbolInvalid,
        unknown,
    };
    enum class RuntimeError {
        none,
        segfault, // check if https://www.gnu.org/software/libsigsegv/ can detect segfault in shared lib only via local SIGSEGV handler
        unknown,
    };

    [[nodiscard]] LoadingError loadingError() const { return loading_error_; }
    [[nodiscard]] RuntimeError runtimeError() const { return runtime_error_; }

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] auto call(const std::string& function_name, Args&&... args)
    {
        return detail::boost_dll::call<ReturnValue, true>(
            plugin_, function_name, std::forward<Args>(args)...);
    }

private:
    boost::dll::shared_library plugin_;
    LoadingError loading_error_ { LoadingError::none };
    RuntimeError runtime_error_ { RuntimeError::none };
};
} // namespace ppplugin

#endif // PPPLUGIN_CPP_PLUGIN_H
