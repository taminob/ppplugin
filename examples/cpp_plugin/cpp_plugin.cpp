#include "cpp_plugin.h"

#include <chrono>
#include <iostream>
#include <memory>

#include <boost/dll/alias.hpp>

BOOST_DLL_ALIAS(SimplePluginA::create, create_a);

int global_variable = 0;

void SimplePluginA::loop()
{
    while (true) {
        doSomething();
    }
}

void SimplePluginA::doSomething()
{
    std::cout << "A: " << i_ << ", global: " << global_variable << '\n';
    sleep(std::chrono::seconds { 1 });
}

class SimplePluginB : public SimplePluginInterface {
public:
    SimplePluginB() = default;

    static std::shared_ptr<SimplePluginB> create()
    {
        return std::make_shared<SimplePluginB>();
    }

    void initialize(int value) override { i_ = std::make_unique<int>(value); }
    void doSomething() override
    {
        std::cout << "B: " << *i_ << ", global: " << global_variable << '\n';
        sleep(std::chrono::seconds { 2 });
    }

    // cannot be accessed by program that loads this plugin because
    // it is hidden in this source file without a virtual function
    // in the parent class
    void inaccessible() { }

private:
    std::unique_ptr<int> i_;
};
BOOST_DLL_ALIAS(SimplePluginB::create, create_b);
