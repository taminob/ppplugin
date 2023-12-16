#include "configurable_plugin.h"

#include <boost/dll.hpp>

BOOST_DLL_ALIAS(ConfigurablePluginA::create, create_a);

void ConfigurablePluginA::loop(const std::atomic<bool>& stop)
{
    while (!stop) {
        print(configuration().printValue);
        sleep(configuration().printInterval);
    }
}
