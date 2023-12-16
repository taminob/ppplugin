#ifndef PPPLUGIN_EXAMPLES_LUA_PLUGIN_H
#define PPPLUGIN_EXAMPLES_LUA_PLUGIN_H

class LuaPluginInterface {
public:
    LuaPluginInterface() = default;
    virtual ~LuaPluginInterface() = default;
    LuaPluginInterface(const LuaPluginInterface&) = default;
    LuaPluginInterface(LuaPluginInterface&&) = default;
    LuaPluginInterface& operator=(const LuaPluginInterface&) = default;
    LuaPluginInterface& operator=(LuaPluginInterface&&) = default;

    virtual void initialize() = 0;
    virtual void loop() = 0;
};

#endif // PPPLUGIN_EXAMPLES_LUA_PLUGIN_H
