#include "configurable_plugin.h"

#include <boost/dll.hpp>

BOOST_DLL_ALIAS(ConfigurablePluginA::create, create_a);

void ConfigurablePluginA::loop()
{
    while (true) {
        print(configuration().printValue);
        sleep(configuration().printInterval);
    }
}
