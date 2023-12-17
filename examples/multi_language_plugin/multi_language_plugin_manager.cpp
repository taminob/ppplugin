#include "plugin_manager.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 1) {
        return -1;
    }
    try {
        auto executable_dir = std::filesystem::path { argv[0] }.parent_path();
        auto cpp_lib_path = executable_dir / "libcpp_plugin.so";
        auto c_lib_path = executable_dir / "libc_plugin.so";
        auto lua_lib_path = executable_dir / "lua_plugin.lua";

        ppplugin::PluginManager manager;
        ppplugin::Plugin cpp_plugin = manager.loadCppPlugin(cpp_lib_path);
        ppplugin::Plugin c_plugin = manager.loadCPlugin(c_lib_path);
        ppplugin::Plugin lua_plugin = manager.loadLuaPlugin(lua_lib_path);
        std::ignore = cpp_plugin.call<>("initialize");
        std::ignore = c_plugin.call<>("initialize");
        std::ignore = lua_plugin.call<>("initialize");
    } catch (const std::exception& exception) {
        std::cerr << "A fatal error occurred: '" << exception.what() << "'\n";
        return 1;
    } catch (...) {
        std::cerr << "An unknown fatal error occurred!";
        return 1;
    }
    return 0;
}
