#ifndef PPPLUGIN_PLUGIN_H
#define PPPLUGIN_PLUGIN_H

#include "detail/template_helpers.h"
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
class GenericPlugin
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    requires(IsPlugin<Plugins> && ...)
#endif // PPPLUGIN_CPP17_COMPATIBILITY
{

public:
    GenericPlugin() = default;
    template <typename P,
        typename = std::enable_if_t<(std::is_base_of_v<Plugins, P> || ...)>>
    GenericPlugin(P && plugin) // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        : plugin_(std::forward<P>(plugin))
    {
    }

    virtual ~GenericPlugin() = default;
    GenericPlugin(const GenericPlugin&) = delete;
    GenericPlugin(GenericPlugin&&) noexcept = default;
    GenericPlugin& operator=(const GenericPlugin&) = delete;
    GenericPlugin& operator=(GenericPlugin&&) noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    virtual operator bool() const
    {
        return std::visit([](const auto& plugin) -> bool { return plugin; }, plugin_);
    }

    // TODO: add specialization for void return type (empty tuple triggers nodiscard)
    template <typename... ReturnValues, typename... Args>
    [[nodiscard]] std::tuple<ReturnValues...> call(Args && ... args)
    {
        return std::visit([&args...](auto& plugin) -> std::tuple<ReturnValues...> {
            return plugin.template call<ReturnValues...>(std::forward<Args>(args)...);
        },
            plugin_);
    }

private:
    detail::templates::UniqueVariant<Plugins...> plugin_;
};

class CppPlugin {
public:
    explicit CppPlugin(const std::filesystem::path& cpp_shared_library)
    {
        std::ignore = loadCppPlugin(cpp_shared_library);
    }
    virtual ~CppPlugin() = default;
    CppPlugin(const CppPlugin&) = delete;
    CppPlugin(CppPlugin&&) = default;
    CppPlugin& operator=(const CppPlugin&) = delete;
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

    template <typename... ReturnValues, typename... Args>
    [[nodiscard]] auto call(const std::string& function_name, Args&&... args)
    {
        static_assert(sizeof...(ReturnValues) <= 1,
            "C++ does not support more than one return value! "
            "Consider wrapping the types into an std::tuple.");
        using ReturnValue = detail::templates::FirstOrT<void, ReturnValues...>;
        using FunctionType = ReturnValue (*)(Args...);
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
        if constexpr (std::is_same_v<ReturnValue, void>) {
            function(args...);
            return std::tuple<> {};
        } else {
            return std::tuple<ReturnValue> { function(args...) };
        }
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
    boost::dll::shared_library plugin_;
    LoadingError loading_error_ { LoadingError::none };
    RuntimeError runtime_error_ { RuntimeError::none };
};

class CPlugin : public CppPlugin {
public:
    explicit CPlugin(const std::filesystem::path& c_shared_library)
        : CppPlugin { c_shared_library }
    {
    }

    template <typename... ReturnValues, typename... Args>
    [[nodiscard]] auto call(const std::string& function_name, Args&&... args)
    {
        // TODO: invalid number of arguments can cause segfault
        // TODO: think of better solution than this
        static_assert(sizeof...(ReturnValues) <= 1,
            "C does not support more than one return value! "
            "Consider wrapping the types into an std::tuple.");
        using ReturnValue = detail::templates::FirstOrT<void, ReturnValues...>;
        using FunctionType = ReturnValue (*)(Args...);
        // TODO: check ABI compatibility (same compiler + major version)?
        if (!plugin_.has(function_name)) {
            throw std::runtime_error("symbol not found"); // TODO
        }
        // cannot use boost here because boost::dll expects the symbol to be
        // a variable (pointing to the function) not a function
#if BOOST_OS_WINDOWS
        auto function = reinterpret_cast<FunctionType>(
            boost::winapi::get_proc_address(plugin_.native(), function_name.c_str()));
#else
        // no other way to convert void* to function pointer
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto function = reinterpret_cast<FunctionType>(
            dlsym(plugin_.native(), function_name.c_str()));
#endif // BOOST_OS_WINDOWS
        if (!function) {
            loading_error_ = LoadingError::symbolInvalid;
            throw std::runtime_error("symbol not valid"); // TODO
        }
        if constexpr (std::is_same_v<ReturnValue, void>) {
            function(args...);
            return std::tuple<> {};
        } else {
            return std::tuple<ReturnValue> { function(args...) };
        }
    }
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
    virtual ~LuaPlugin() = default;
    LuaPlugin(const LuaPlugin&) = delete;
    LuaPlugin(LuaPlugin&&) = default;
    LuaPlugin& operator=(const LuaPlugin&) = delete;
    LuaPlugin& operator=(LuaPlugin&&) = default;

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
