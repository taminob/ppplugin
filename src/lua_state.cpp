#include "lua/lua_state.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

// TODO: move to cmake?
namespace {
constexpr auto MINIMUM_LUA_VERSION = 502;
} // namespace
static_assert(LUA_VERSION_NUM >= MINIMUM_LUA_VERSION);

namespace ppplugin {
LuaState::LuaState()
    : state_ { luaL_newstate(), &lua_close }
{
}

LuaState::LuaState(lua_State* state)
    : state_ { state, [](lua_State*) {} }
{
}

std::optional<std::string> LuaState::topString()
{
    if (isString()) {
        std::size_t result_length {};
        const char* result = lua_tolstring(state(), -1, &result_length);
        return std::string { result, result_length };
    }
    return std::nullopt;
}

std::optional<bool> LuaState::topBool()
{
    if (isBool()) {
        return lua_toboolean(state(), -1);
    }
    return std::nullopt;
}

std::optional<int> LuaState::topInt()
{
    // TODO: since Lua 5.3 lua_isinteger can be used
    if (isNumber()) {
        return lua_tointeger(state(), -1);
    }
    return std::nullopt;
}

std::optional<double> LuaState::topDouble()
{
    if (isNumber()) {
        return lua_tonumber(state(), -1);
    }
    return std::nullopt;
}

const void* LuaState::topPointer()
{
    return lua_topointer(state(), -1);
}

void LuaState::discardTop()
{
    lua_pop(state(), 1);
}

void LuaState::pushOne(double value)
{
    lua_pushnumber(state(), value);
}

void LuaState::pushOne(long long value)
{
    lua_pushinteger(state(), value);
}

void LuaState::pushOne(const char* value)
{
    pushOne(std::string_view { value });
}

void LuaState::pushOne(std::string_view value)
{
    lua_pushlstring(state(), value.data(), value.size());
}

void LuaState::pushOne(bool value)
{
    lua_pushboolean(state(), static_cast<int>(value));
}

void LuaState::pushOne(std::nullptr_t)
{
    lua_pushnil(state());
}

void LuaState::pushOne(LuaCFunction func)
{
    lua_pushcfunction(state(), func);
}

bool LuaState::isBool()
{
    return lua_type(state(), -1) == LUA_TBOOLEAN;
}

bool LuaState::isNumber()
{
    return lua_type(state(), -1) == LUA_TNUMBER;
}

bool LuaState::isString()
{
    return lua_type(state(), -1) == LUA_TSTRING;
}

bool LuaState::isFunction()
{
    return lua_type(state(), -1) == LUA_TFUNCTION;
}

bool LuaState::isNil()
{
    return lua_type(state(), -1) == LUA_TNIL;
}

void LuaState::registerPanicHandler(LuaCFunction handler)
{
    lua_atpanic(state(), handler);
}

int LuaState::pcall(std::size_t argument_count, std::size_t return_count)
{
    // TODO: have message handler on stack for lua_pcall (last parameter)
    return lua_pcall(state(), argument_count, return_count, 0);
}

void LuaState::markGlobal(const std::string& variable_name)
{
    lua_setglobal(state(), variable_name.c_str());
}

bool LuaState::pushGlobal(const std::string& variable_name)
{
    lua_getglobal(state(), variable_name.c_str());
    if (isNil()) {
        discardTop();
        return false;
    }
    return true;
}
} // namespace ppplugin
