#ifndef PPPLUGIN_PLUGIN_H
#define PPPLUGIN_PLUGIN_H

#include "lua_script.h"
#include <boost/dll.hpp>

#ifndef PPPLUGIN_CPP17_COMPATIBILITY
#include <concepts>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
#include <filesystem>
#include <optional>
#include <utility>
#include <variant>

namespace ppplugin {
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
// TODO: does not work
template <typename P, typename... ReturnValues, typename... Args>
concept CallableWithAnything(P p)
{
    {
        p.call<ReturnValues...>(std::declval<Args...>())
    } -> std::tuple<ReturnValues...>
}
template <typename P>
concept IsPlugin = requires(P p) {
    {
        std::convertible_to<P, bool>
    }
    ,
        { CallableWithAnything<P, int> },
        { P::create() },
}
#endif // PPPLUGIN_CPP17_COMPATIBILITY

template <typename... Plugins>
class GenericPlugin {
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    requires(IsPlugin<Plugins> && ...)
#endif // PPPLUGIN_CPP17_COMPATIBILITY
        public:
    GenericPlugin() = default;

    virtual ~GenericPlugin() = default;
    GenericPlugin(const GenericPlugin&) = default;
    GenericPlugin(GenericPlugin&&) noexcept = default;
    GenericPlugin& operator=(const GenericPlugin&) = default;
    GenericPlugin& operator=(GenericPlugin&&) noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    virtual operator bool() const
    {
        return std::visit([](const auto& plugin) -> bool { return plugin; }, plugin_);
    }

    template <typename... ReturnValues, typename... Args>
    [[nodiscard]] std::tuple<ReturnValues...> call(Args&&... args)
    {
        return std::visit([&args...](const auto& plugin) -> bool {
            return plugin.template call<ReturnValues...>(args...);
        },
            plugin_);
    }

private:
    std::variant<Plugins...> plugin_;
};

class CPlugin {
public:
    CPlugin() = default;
    explicit CPlugin(boost::dll::shared_library c_shared_library)
        : c_shared_library_ { std::move(c_shared_library) }
    {
    }

private:
    boost::dll::shared_library c_shared_library_;
};

class CppPlugin {
public:
    explicit CppPlugin(const std::filesystem::path& cpp_shared_library)
    {
        std::ignore = loadCppPlugin(cpp_shared_library);
    }

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
    [[nodiscard]] ReturnValue call(const std::string& function_name, Args&&... args)
    {
        // TODO: check ABI compatibility
        if (!plugin_.has(function_name)) {
            loading_error_ = LoadingError::symbolNotFound;
            return ReturnValue{}; // TODO
        }
        // TODO: invalid number of arguments can cause segfault
        auto function = plugin_.get<ReturnValue(*)(Args...)>(function_name);
        if (!function) {
            loading_error_ = LoadingError::symbolInvalid;
            return ReturnValue{}; // TODO
        }
        return function(args...);
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

private:
    boost::dll::shared_library plugin_;
    LoadingError loading_error_ { LoadingError::none };
    RuntimeError runtime_error_ { RuntimeError::none };
};

template <typename P>
class CppObjectPlugin {
public:
};

class LuaPlugin {
public:
    explicit LuaPlugin(const std::filesystem::path& lua_script_path)
        : plugin_ { lua_script_path }
    {
    }

    [[nodiscard]] auto& raw() { return plugin_; }
    [[nodiscard]] const auto& raw() const { return plugin_; }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const { return true; }

    template <typename... ReturnValues, typename... Args>
    [[nodiscard]] std::tuple<ReturnValues...> call(const std::string& function_name, Args&&... args)
    {
        return plugin_.call<ReturnValues...>(function_name, std::forward<Args>(args)...);
    }

private:
    LuaScript plugin_;
};

using Plugin = GenericPlugin<CPlugin, CppPlugin, LuaPlugin>;
} // namespace ppplugin

#endif // PPPLUGIN_PLUGIN_H
