#include <ppplugin/plugin_manager.h>

#include <iostream>

int main(int argc, char* /*argv*/[])
{
    auto plugin = ppplugin::LuaPlugin::load("a.lua");
    if (plugin) {
        std::cout << plugin->call<int>("Main", argc).value().value() << '\n';
    } else {
        std::cerr << "Error: " << static_cast<int>(plugin.error().value()) << '\n';
    }
}
