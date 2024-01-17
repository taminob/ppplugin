#ifndef PPPLUGIN_CPP_PLUGIN_H
#define PPPLUGIN_CPP_PLUGIN_H

#include <boost/dll.hpp>

#include <filesystem>
#include <string>

namespace ppplugin {
class CppPlugin {
public:
    explicit CppPlugin(const std::filesystem::path& cpp_shared_library)
    {
        std::ignore = loadCppPlugin(cpp_shared_library);
    }
    virtual ~CppPlugin() = default;
    CppPlugin(const CppPlugin&) = default;
    CppPlugin(CppPlugin&&) = default;
    CppPlugin& operator=(const CppPlugin&) = default;
    CppPlugin& operator=(CppPlugin&&) = default;

    [[nodiscard]] auto& raw() { return plugin_; }
    [[nodiscard]] const auto& raw() const { return plugin_; }

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
        using FunctionType = ReturnValue (*)(Args...);
        if (!plugin_.is_loaded()) {
            throw std::runtime_error("plugin not loaded");
        }
        // TODO: check ABI compatibility (same compiler + major version)?
        if (!plugin_.has(function_name)) {
            loading_error_ = LoadingError::symbolNotFound;
            throw std::runtime_error("symbol not found"); // TODO
        }
        // TODO: invalid number of arguments can cause segfault
        auto function = plugin_.get<FunctionType>(function_name);
        if (!function) {
            loading_error_ = LoadingError::symbolInvalid;
            throw std::runtime_error("symbol not valid"); // TODO
        }
        return function(std::forward<Args>(args)...);
    }

protected:
    /**
     * Load any plugin of given type with given creation function and arguments.
     */
    [[nodiscard]] bool loadCppPlugin(
        const std::filesystem::path& plugin_library_path)
    {
        if (!std::filesystem::exists(plugin_library_path)) {
            loading_error_ = LoadingError::notFound;
            return false;
        }

        plugin_ = boost::dll::shared_library {
            boost::dll::fs::path { plugin_library_path },
            boost::dll::load_mode::append_decorations
        };

        return plugin_.is_loaded();
    }

protected:
    auto& plugin() { return plugin_; }

private:
    boost::dll::shared_library plugin_;
    LoadingError loading_error_ { LoadingError::none };
    RuntimeError runtime_error_ { RuntimeError::none };
};
} // namespace ppplugin

#endif // PPPLUGIN_CPP_PLUGIN_H
