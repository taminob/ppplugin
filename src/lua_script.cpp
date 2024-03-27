#include "lua/lua_script.h"
#include "detail/compatibility_utils.h"
#include "errors.h"
#include "expected.h"
#include "lua/lua_state.h"

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace ppplugin {
Expected<LuaScript, LoadError> LuaScript::load(const std::filesystem::path& script_path, bool auto_run)
{
    LuaScript new_script;
    if (auto error = new_script.loadFile(script_path, auto_run)) {
        return *error;
    }
    return new_script;
}

LuaScript::LuaScript()
{
    luaL_openlibs(state_.state());
    state_.registerPanicHandler([](lua_State* state) -> int {
        auto error = LuaState::wrap(state).top<std::string>();
        // TODO: don't throw because will cross library boundaries;
        //       use global error state (thread-local or lock-protected)
        throw std::runtime_error(format("Lua PANIC: '{}'!", error.value_or("?")));
    });
    // TODO: setup lua_setwarnf
}

bool LuaScript::run()
{
    return lua_pcall(state_.state(), 0, LUA_MULTRET, 0) == LUA_OK;
}

std::optional<LoadError> LuaScript::loadFile(const std::filesystem::path& lua_file, bool auto_run)
{
    if (!std::filesystem::exists(lua_file)) {
        return LoadError::fileNotFound;
    }
    if (luaL_loadfile(state_.state(), lua_file.c_str()) != LUA_OK) {
        return LoadError::fileInvalid;
    }
    if (auto_run && !run()) {
        return LoadError::unknown;
    }
    return std::nullopt;
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
