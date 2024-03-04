#include "lua/lua_script.h"

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

namespace ppplugin {
LuaScript::LuaScript(const std::filesystem::path& script_path, bool auto_run)
{
    luaL_openlibs(state_.state());
    state_.registerPanicHandler([](lua_State* state) -> int {
        auto error = LuaState::wrap(state).top<std::string>();
        throw std::runtime_error(format("Lua PANIC: '{}'!", error.value_or("?")));
    });
    if (!load(script_path, auto_run)) {
        throw std::runtime_error("Error loading script!");
    }
    // TODO: setup lua_setwarnf
}

bool LuaScript::run()
{
    return lua_pcall(state_.state(), 0, LUA_MULTRET, 0) == LUA_OK;
}

bool LuaScript::load(const std::filesystem::path& lua_file, bool auto_run)
{
    if (!std::filesystem::exists(lua_file)) {
        return false;
    }
    if (luaL_loadfile(state_.state(), lua_file.c_str()) != LUA_OK) {
        return false;
    }
    if (auto_run) {
        return run();
    }
    return true;
}

std::string LuaScript::errorToString(int error_code)
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
} // namespace ppplugin
