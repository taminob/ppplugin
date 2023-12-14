// g++ examples/simple_plugin.cpp -shared -o simple_plugin.so -I examples -lboost_program_options -lboost_filesystem -fPIC
#include "simple_plugin.h"

#include <iostream>

BOOST_DLL_ALIAS(SimplePluginA::create, create_a);
BOOST_DLL_ALIAS(SimplePluginB::create, create_b);

void SimplePluginA::loop() {
  while (true) {
    print(i_);
    sleep(std::chrono::milliseconds{750});
  }
}

void SimplePluginB::initialize() { i_ = std::make_unique<int>(1); }

void SimplePluginB::loop() {
  while (i_) {
    print(*i_);
    sleep(std::chrono::milliseconds{500});
  }
}
