#include "plugin_manager.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <thread>

int main(int argc, char* argv[])
{
    if (argc < 1) {
        return -1;
    }
    try {
        auto executable_dir = std::filesystem::path { argv[0] }.parent_path();
        auto valid_lib_path = executable_dir / "libvalid_plugin.so";
        auto segfault_lib_path = executable_dir / "libsegfault_plugin.so";

        ppplugin::PluginManager manager;
        auto valid_plugin = manager.loadCPlugin(valid_lib_path);
        auto segfault_plugin = manager.loadCPlugin(segfault_lib_path);

        auto print_result = [](std::string_view message, auto&& result) {
            std::cout << message << static_cast<char>(result)
                      << " (" << result << ')' << std::endl;
        };

        auto result = valid_plugin.call<int>("function", static_cast<const char*>("abc"));
        print_result("valid call, no problem: ", result);
        // this can cause segfault, but could also result in garbage;
        // even a "safe" call cannot catch everything
        result = valid_plugin.safeCall<int>("function");
        print_result("called function with wrong arguments: ", result);
        result = segfault_plugin.safeCall<int>("function", static_cast<const char*>("xyz"));
        print_result("faulty plugin, but program survived segfault: ", result);
        std::cout << "try again without handling segfaults..." << std::endl;
        std::ignore = segfault_plugin.call<int>("function", static_cast<const char*>("xyz"));
    } catch (const std::exception& exception) {
        std::cerr << "A fatal error occurred: '" << exception.what() << "'\n";
        return 1;
    } catch (...) {
        std::cerr << "An unknown fatal error occurred!";
        return 1;
    }
    return 0;
}
