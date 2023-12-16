#ifndef PPPLUGIN_PLUGIN_H
#define PPPLUGIN_PLUGIN_H

#include <boost/dll.hpp>
#include <optional>
#include <utility>

namespace ppplugin {
class Plugin {
public:
    Plugin() = default;

    virtual ~Plugin() = default;
    Plugin(const Plugin&) = default;
    Plugin(Plugin&&) noexcept = default;
    Plugin& operator=(const Plugin&) = default;
    Plugin& operator=(Plugin&&) noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    virtual operator bool() const = 0;
};

template <typename P, typename E>
class GenericPlugin : public Plugin {
public:
    GenericPlugin() = default;

    operator bool() const override { return true; }
};

template <typename P>
class CppPlugin : public Plugin {
public:
    explicit CppPlugin(boost::dll::shared_library cpp_shared_library)
        : cpp_shared_library_ { std::move(cpp_shared_library) }
    {
    }

    P& plugin() { return *plugin_; }
    const P& plugin() const { return *plugin_; }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    operator bool() const override
    {
        return loading_error_ == LoadingError::none && runtime_error_ == RuntimeError::none;
    }

    enum class LoadingError {
        none,
        notFound,
        loadingFailed,
        symbolNotFound,
        unknown,
    };
    enum class RuntimeError {
        none,
        segfault, // check if https://www.gnu.org/software/libsigsegv/ can detect segfault in shared lib only via local SIGSEGV handler
        unknown,
    };

    [[nodiscard]] LoadingError loadingError() const { return loading_error_; }
    [[nodiscard]] RuntimeError runtimeError() const { return runtime_error_; }

private:
    boost::dll::shared_library cpp_shared_library_;
    std::unique_ptr<P> plugin_;
    LoadingError loading_error_ { LoadingError::none };
    RuntimeError runtime_error_ { RuntimeError::none };
};

template <typename P>
class LuaPlugin : public Plugin {
public:
    LuaPlugin() = default;

    P& plugin() { return *plugin_; }

private:
    std::unique_ptr<P> plugin_;
};
} // namespace ppplugin

#endif // PPPLUGIN_PLUGIN_H
