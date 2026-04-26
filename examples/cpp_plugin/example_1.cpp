#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

// library headers
#include "ppplugin/cpp/plugin.h"
#include "ppplugin/errors.h"

// header of example plugin
#include "cpp_plugin.h"

int main()
{
    auto&& load_result = ppplugin::CppPlugin::load("./libcpp_plugin.so");
    if (!load_result) {
        std::cerr << load_result.error() << '\n';
        return 1;
    }

    // loading plugin was successful, call a function that returns "std::shared_ptr<SimplePluginA>"
    auto&& plugin = load_result.value();
    auto&& call_result = plugin.call<std::shared_ptr<SimplePluginA>>("create_a");
    if (!call_result) {
        std::cerr << call_result.error() << '\n';
        return 2;
    }

    // unwrap instance of newly created SimplePluginA and call function that was
    // unknown to this program until the plugin was loaded (defined in cpp_plugin.cpp)
    auto&& simple_plugin_a = call_result.value();
    simple_plugin_a->loop();
}
