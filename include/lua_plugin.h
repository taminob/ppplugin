#ifndef PPPLUGIN_LUA_PLUGIN_H
#define PPPLUGIN_LUA_PLUGIN_H

#include <string>
#include <string_view>

namespace ppplugin {
class LuaPlugin {
public:
    LuaPlugin() = default;

    virtual ~LuaPlugin() = default;
    LuaPlugin(const LuaPlugin&) = default;
    LuaPlugin(LuaPlugin&&) noexcept = default;
    LuaPlugin& operator=(const LuaPlugin&) = default;
    LuaPlugin& operator=(LuaPlugin&&) noexcept = default;
};
} // namespace ppplugin

#endif // PPPLUGIN_LUA_PLUGIN_H
