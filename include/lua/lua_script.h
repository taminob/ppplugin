#ifndef PPPLUGIN_LUA_SCRIPT_H
#define PPPLUGIN_LUA_SCRIPT_H

#include "detail/compatibility_utils.h"
#include "errors.h"
#include "expected.h"

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
        : state_ { luaL_newstate(), &lua_close }
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

    template <typename ReturnValue, typename... Args>
    CallResult<ReturnValue> call(const std::string& function, Args&&... args)
    {
        return CallHelper<ReturnValue, Args...>(*this, function, std::forward<Args>(args)...).returnValue();
    }

protected:
    lua_State* state() { return state_.get(); }

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
            if (lua_type(state, -1) == LUA_TSTRING) {
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

    template <typename... Ts>
    struct PopTuple {
        static std::optional<std::tuple<Ts...>> pop(LuaScript& script)
        {
            if constexpr (sizeof...(Ts) > 0) {
                return PopTuple<Ts...>::pop(script);
            } else {
                return std::tuple<> {};
            }
        }
    };
    template <typename T, typename... Ts>
    struct PopTuple<T, Ts...> {
        static std::optional<std::tuple<T, Ts...>> pop(LuaScript& script)
        {
            if (auto result = script.pop<T>()) {
                if (auto others = PopTuple<Ts...>::pop(script)) {
                    return std::tuple_cat(*others, std::make_tuple(*result));
                }
            }
            return std::nullopt; // TODO: add concrete error
        }
    };

    struct StackContext {
        // TODO: push error handling function to stack
        //       and recover previous stack state on destruction
        template <typename... Args>
        explicit StackContext(Args&&... /*args*/)
        {
        }
    };

    friend struct CallHelper;
    template <typename ReturnValue, typename... Args>
    struct CallHelper {
        CallHelper(LuaScript& script, const std::string& function, Args&&... args)
        {
            StackContext context {};
            auto type = lua_getglobal(script.state(), function.c_str());
            if (type != LUA_TFUNCTION) {
                result_ = {
                    CallError::Code::unknown,
                    format("'{}' is not callable (type: '{}')!",
                        function,
                        lua_typename(script.state(), type))
                };
                return;
            }
            script.push(args...);
            // TODO: have message handler on stack for lua_pcall (last parameter)
            auto error = lua_pcall(script.state(), sizeof...(Args), 1, 0);
            if (error != LUA_OK) {
                result_ = {
                    CallError::Code::unknown,
                    format("Unable to call '{}'. Code: '{}'. Error: '{}'",
                        function,
                        error,
                        error == LUA_ERRRUN ? script.pop<std::string>().value_or("?") : "")
                };
                return;
            }
            if (auto result = script.pop<ReturnValue>()) {
                result_ = std::move(*result);
            } else {
                result_ = { CallError::Code::unknown,
                    "Wrong return type" }; // TODO: could also retrieve information about actual type
            }
        }

        auto&& returnValue() const&& { return std::move(result_); }

    private:
        CallResult<ReturnValue> result_ { CallError::Code::unknown };
    };
    template <typename... Args>
    struct CallHelper<void, Args...> {
        CallHelper(LuaScript& script, const std::string& function, Args&&... args)
        {
            StackContext context {};
            auto type = lua_getglobal(script.state(), function.c_str());
            if (type != LUA_TFUNCTION) {
                result_ = CallError {
                    CallError::Code::unknown,
                    format("'{}' is not callable (type: '{}')!",
                        function,
                        lua_typename(script.state(), type))
                };
                return;
            }
            script.push(args...);
            // TODO: have message handler on stack for lua_pcall (last parameter)
            auto error = lua_pcall(script.state(), sizeof...(Args), 1, 0);
            if (error != LUA_OK) {
                result_ = CallError { CallError::Code::unknown,
                    format("Unable to call '{}'. Code: '{}'. Error: '{}'",
                        function,
                        error,
                        error == LUA_ERRRUN ? script.pop<std::string>().value_or("?") : "") };
                return;
            }
        }

        auto&& returnValue() const&& { return result_; }

    private:
        CallResult<void> result_ { CallError::Code::unknown };
    };
    template <typename... ReturnValues, typename... Args>
    struct CallHelper<std::tuple<ReturnValues...>, Args...> {
        CallHelper(LuaScript& script, const std::string& function, Args&&... args)
        {
            StackContext context {};
            auto type = lua_getglobal(script.state(), function.c_str());
            if (type != LUA_TFUNCTION) {
                result_ = CallError {
                    CallError::Code::unknown,
                    format("'{}' is not callable (type: '{}')!",
                        function,
                        lua_typename(script.state(), type))
                };
                return;
            }
            script.push(args...);
            // TODO: have message handler on stack for lua_pcall (last parameter)
            auto error = lua_pcall(script.state(), sizeof...(Args), sizeof...(ReturnValues), 0);
            if (error != LUA_OK) {
                result_ = CallError { CallError::Code::unknown,
                    format("Unable to call '{}'. Code: '{}'. Error: '{}'",
                        function,
                        error,
                        error == LUA_ERRRUN ? script.pop<std::string>().value_or("?") : "") };
                return;
            }
            if (auto result = PopTuple<ReturnValues...>::pop(script)) {
                result_ = std::move(*result);
            } else {
                result_ = { CallError::Code::unknown }; // TODO: replace by something more specific
            }
        }

        auto&& returnValue() const&& { return std::move(result_); }

    private:
        CallResult<std::tuple<ReturnValues...>> result_ { CallError::Code::unknown };
    };

private:
    std::unique_ptr<lua_State, decltype(&lua_close)> state_;
};
} // namespace ppplugin

#endif // PPPLUGIN_LUA_SCRIPT_H
