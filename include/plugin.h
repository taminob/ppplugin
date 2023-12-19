#ifndef PPPLUGIN_PLUGIN_H
#define PPPLUGIN_PLUGIN_H

#include "detail/segfault_handling.h"
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
template <typename P>
concept IsPlugin = requires(P p) {
    {
        std::convertible_to<P, bool>
    };
    // {
    //     p.template call<>(std::declval<int>())
    // } -> std::same_as<std::tuple<>>;
};
#endif // PPPLUGIN_CPP17_COMPATIBILITY

template <typename... Plugins>
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    requires(IsPlugin<Plugins> && ...)
#endif // PPPLUGIN_CPP17_COMPATIBILITY
class GenericPlugin {

public:
    GenericPlugin() = default;
    template <typename P,
        typename = std::enable_if_t<(std::is_base_of_v<Plugins, P> || ...)>>
    GenericPlugin(P&& plugin) // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
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
    [[nodiscard]] std::tuple<ReturnValues...> call(const std::string& function_name, Args&&... args)
    {
        return std::visit(
            [&function_name, &args...](auto& plugin) -> std::tuple<ReturnValues...> {
                return plugin.template call<ReturnValues...>(
                    function_name, std::forward<Args>(args)...);
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
            function(std::forward<Args>(args)...);
            return std::tuple<> {};
        } else {
            return std::tuple<ReturnValue> { function(std::forward<Args>(args)...) };
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
    auto& plugin() { return plugin_; }

private:
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

    template <typename T>
    struct CopyIfReference {
        template <typename = std::enable_if<std::is_reference_v<T>>>
        explicit CopyIfReference(std::remove_reference<T> by_value)
            : value { by_value }
        {
        }
        template <typename = std::enable_if<!std::is_reference_v<T>>>
        explicit CopyIfReference(T&& by_reference)
            : value { by_reference }
        {
        }

        T value;
    };

    template <typename... ReturnValues, typename... Args>
    [[nodiscard]] auto call(const std::string& function_name, Args&&... args)
    {
        return internalCall<false, ReturnValues...>(
            function_name, std::forward<Args>(args)...);
    }

    template <typename... ReturnValues, typename... Args>
    [[nodiscard]] auto safeCall(const std::string& function_name, Args&&... args)
    {
        return internalCall<true, ReturnValues...>(
            function_name, std::forward<Args>(args)...);
    }

protected:
    template <bool catchSegfaults, typename... ReturnValues, typename... Args>
    [[nodiscard]] auto internalCall(const std::string& function_name, Args&&... args)
    {
        // TODO: invalid number of arguments can cause segfault
        // TODO: think of better solution than this
        static_assert(sizeof...(ReturnValues) <= 1,
            "C does not support more than one return value! "
            "Consider wrapping the types into an std::tuple.");
        static_assert(!(std::is_reference_v<ReturnValues> || ...),
            "C does not support references for its return value!");
        static_assert(!(std::is_reference_v<Args> || ...),
            "C does not support references for its arguments! "
            "Consider passing the argument with an explicit cast to the desired type.");
        using ReturnValue = detail::templates::FirstOrT<void, ReturnValues...>;
        using FunctionType = ReturnValue (*)(Args...);
        // TODO: check ABI compatibility (same compiler + major version)?
        if (!plugin().has(function_name)) {
            throw std::runtime_error("symbol not found"); // TODO
        }
        // cannot use boost here because boost::dll expects the symbol to be
        // a variable (pointing to the function) not a function
#if BOOST_OS_WINDOWS
        auto function = reinterpret_cast<FunctionType>(
            boost::winapi::get_proc_address(plugin().native(), function_name.c_str()));
#else
        // no other way to convert void* to function pointer
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto function = reinterpret_cast<FunctionType>(
            dlsym(plugin().native(), function_name.c_str()));
#endif // BOOST_OS_WINDOWS
        if (!function) {
            throw std::runtime_error("symbol not valid"); // TODO
        }
        if constexpr (std::is_same_v<ReturnValue, void>) {
            if constexpr (catchSegfaults) {
                detail::segfault_handling::exec<ReturnValue>(function, std::forward<Args>(args)...);
            } else {
                function(std::forward<Args>(args)...);
            }
            return std::tuple<> {};
        } else {
            if constexpr (catchSegfaults) {
                return std::tuple<ReturnValue> {
                    detail::segfault_handling::exec<ReturnValue>(function, std::forward<Args>(args)...)
                };
            } else {
                return std::tuple<ReturnValue> { function(std::forward<Args>(args)...) };
            }
        }
    }
};

/**
 * Plugin that has a creator function that creates an object
 * of a known base class.
 */
template <typename P>
class CppObjectPlugin {
public:
    // TODO
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
