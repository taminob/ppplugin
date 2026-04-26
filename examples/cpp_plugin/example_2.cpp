#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "ppplugin/cpp/plugin.h"
#include "ppplugin/errors.h"
#include "ppplugin/plugin_manager.h"

#include "cpp_plugin.h"

int main(int /*argc*/, char* argv[])
{
    auto executable_dir = std::filesystem::path { argv[0] }.parent_path();
    auto library_path = executable_dir / "libcpp_plugin.so";
    // setup manager - has to stay alive for as long as you want to use the plugins
    ppplugin::PluginManager manager;
    // load plugin library and exit on error
    auto plugin = manager.loadCppPlugin(std::filesystem::path { library_path })
                      .valueOrElse([](const auto& error) -> ppplugin::CppPlugin {
                          std::cerr << "Unable to load plugin: " << error;
                          std::exit(1); // NOLINT(concurrency-mt-unsafe)
                      });

    auto simple_plugin_a = plugin.call<std::shared_ptr<SimplePluginA>>("create_a").valueOr(nullptr);
    // create instance of type SimplePluginB which is fully unknown to this program;
    // the entire class definition is in cpp_plugin.cpp which is not compiled into this binary
    auto simple_plugin_b = plugin.call<std::shared_ptr<SimplePluginInterface>>("create_b").valueOr(nullptr);

    if (simple_plugin_a) {
        simple_plugin_a->initialize(1000);
        simple_plugin_a->doSomething();
    }
    if (simple_plugin_b) {
        simple_plugin_a->initialize(2000);
        simple_plugin_b->doSomething();
    }
}
