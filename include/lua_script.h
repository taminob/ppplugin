#ifndef PPPLUGIN_LUA_SCRIPT_H
#define PPPLUGIN_LUA_SCRIPT_H

#include "detail/compatibility_utils.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

// TODO: move to cmake?
constexpr auto MINIMUM_LUA_VERSION = 502;
static_assert(LUA_VERSION_NUM > MINIMUM_LUA_VERSION);

namespace ppplugin {
class LuaScript {
public:
    /**
     * Load given Lua script.
     *
     * @param auto_run if set to true, the script will be run immediately after loading;
     *                 if set to false, functions cannot be called since they haven't been
     *                 discovered yet (required execution)
     */
    explicit LuaScript(const std::filesystem::path& script_path, bool auto_run = true)
        : l_ { luaL_newstate(), &lua_close }
    {
        luaL_openlibs(state());
        registerPanicHandler([](lua_State* state) -> int {
            auto error = top<std::string>(state);
            throw std::runtime_error(format("Lua PANIC: '{}'!", error.value_or("?")));
        });
        if (!load(script_path, auto_run)) {
            throw std::runtime_error("Error loading script!");
        }
    }

    virtual ~LuaScript() = default;
    LuaScript(const LuaScript&) = delete;
    LuaScript(LuaScript&&) noexcept = default;
    LuaScript& operator=(const LuaScript&) = delete;
    LuaScript& operator=(LuaScript&&) noexcept = default;

    template <typename... ReturnValues, typename... Args>
    std::tuple<ReturnValues...> call(const std::string& function, Args&&... args)
    {
        StackContext context {};
        auto type = lua_getglobal(state(), function.c_str());
        if (type != LUA_TFUNCTION) {
            throw std::runtime_error(format("'{}' is not callable (type: '{}')!", function,
                lua_typename(state(), type)));
        }
        push(args...);
        auto result = lua_pcall(state(), sizeof...(Args), sizeof...(ReturnValues), 0);
        if (result != LUA_OK) {
            // TODO: only LUA_ERRRUN puts error message on stack top
            throw std::runtime_error(format("Unable to call '{}'. Result: '{}'. Error: '{}'",
                function, result, top<std::string>().value_or(lua_tostring(state(), -1))));
        }
        // TODO: retrieve return value(s)
        return {};
    }

protected:
    lua_State* state() { return l_.get(); }

    using LuaCFunction = std::function<int(lua_State*)>;
    template <typename Func>
    using IsLuaCFunction = std::is_invocable_r<int, Func, lua_State*>;

    bool run()
    {
        return lua_pcall(state(), 0, LUA_MULTRET, 0) == LUA_OK;
    }

    bool load(const std::filesystem::path& lua_file, bool auto_run)
    {
        if (!std::filesystem::exists(lua_file)) {
            return false;
        }
        if (luaL_loadfile(state(), lua_file.c_str()) != LUA_OK) {
            return false;
        }
        if (auto_run && !run()) {
            return false;
        }
        return true;
    }

    template <typename... Args>
    void push(Args&&... args)
    {
        if constexpr (sizeof...(Args) > 0) {
            pushImpl(state(), args...);
        }
    }

    template <typename T, typename... Args>
    static void pushImpl(lua_State* state, T&& arg, Args&&... args)
    {
        pushOne(state, std::forward<T>(arg));
        if constexpr (sizeof...(Args) > 0) {
            pushImpl(state, std::forward<Args...>(args...));
        }
    }

    static void pushOne(lua_State* state, double value)
    {
        lua_pushnumber(state, value);
    }

    static void pushOne(lua_State* state, int value)
    {
        lua_pushinteger(state, value);
    }

    static void pushOne(lua_State* state, const char* value)
    {
        pushOne(state, std::string_view { value });
    }
    static void pushOne(lua_State* state, std::string_view value)
    {
        lua_pushlstring(state, value.data(), value.size());
    }

    static void pushOne(lua_State* state, bool value)
    {
        lua_pushboolean(state, static_cast<int>(value));
    }

    static void pushOne(lua_State* state, std::nullptr_t)
    {
        lua_pushnil(state);
    }

    template <typename Func>
    static void pushOne(lua_State* state, Func&& func)
    {
        static_assert(IsLuaCFunction<Func>::value,
            "Pushed function does not accept the correct arguments or"
            "has the wrong return type!");
        lua_pushcfunction(state, func);
    }

    template <typename T>
    std::optional<T> top() { return top<T>(state()); }

    template <typename T>
    static std::optional<T> top(lua_State* state)
    {
        using PlainT = std::remove_cv_t<std::remove_reference_t<T>>;
        if constexpr (std::is_same_v<PlainT, double>) {
            if (lua_type(state, -1) == LUA_TNUMBER) {
                return lua_tonumber(state, -1);
            }
        } else if constexpr (std::is_same_v<PlainT, int>) {
            // TODO: since Lua 5.3 lua_isinteger can be used
            if (lua_type(state, -1) == LUA_TNUMBER) {
                return lua_tointeger(state, -1);
            }
        } else if constexpr (std::is_same_v<PlainT, bool>) {
            if (lua_type(state, -1) == LUA_TBOOLEAN) {
                return lua_toboolean(state, -1);
            }
        } else if constexpr (std::is_same_v<PlainT, std::string>) {
            if (lua_type(state, -1) == LUA_TNUMBER) {
                std::size_t result_length {};
                const char* result = lua_tolstring(state, -1, &result_length);
                return std::string { result, result_length };
            }
        } else {
            static_assert(!sizeof(T), "Unsupported type!");
        }
        return std::nullopt;
    }

    template <typename T>
    void setGlobal(const std::string& variable_name, T&& value)
    {
        push(value);
        lua_setglobal(state(), variable_name.c_str());
    }

    /**
     * Pop upmost element from Lua's stack.
     *
     * @param always_pop if true, always pop even if the element has a different type and
     *                   its value is not returned by the function.
     */
    template <typename T>
    std::optional<T> pop(bool always_pop = false) { return pop<T>(state(), always_pop); }

    template <typename T>
    static std::optional<T> pop(lua_State* state, bool always_pop = false)
    {
        auto result = top<T>(state);
        if (result.has_value() || always_pop) {
            lua_pop(state, 1);
        }
        return result;
    }

    template <typename Func>
    void registerPanicHandler(Func&& handler)
    {
        static_assert(IsLuaCFunction<Func>::value,
            "Given panic handler does not accept the correct arguments or"
            "has the wrong return type!");
        lua_atpanic(state(), handler);
    }

    static std::string errorToString(int error_code)
    {
        switch (error_code) {
        case LUA_OK:
            return "ok";
        case LUA_YIELD:
            return "yield";
        case LUA_ERRERR:
            return "error";
        case LUA_ERRRUN:
            return "run error";
        case LUA_ERRFILE:
            return "file error";
        case LUA_ERRSYNTAX:
            return "syntax error";
        case LUA_ERRMEM:
            return "memory error";
        default:
            return "";
        }
    }

    struct StackContext {
        template <typename... Args>
        explicit StackContext(Args&&... args)
        {
        }
    };

private:
    std::unique_ptr<lua_State, decltype(&lua_close)> l_;
};
} // namespace ppplugin

#endif // PPPLUGIN_LUA_SCRIPT_H
