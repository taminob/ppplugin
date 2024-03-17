#include "simple_plugin.h"

#include <chrono>
#include <memory>

#include <boost/dll/alias.hpp>

BOOST_DLL_ALIAS(SimplePluginA::create, create_a);

void SimplePluginA::loop()
{
    while (true) {
        print(i_);
        sleep(std::chrono::milliseconds { 750 });
    }
}

class SimplePluginB : public SimplePluginInterface {
public:
    SimplePluginB() = default;

    static std::shared_ptr<SimplePluginB> create()
    {
        return std::make_shared<SimplePluginB>();
    }

    void initialize() override { i_ = std::make_unique<int>(1); }
    void loop() override
    {
        while (i_) {
            print(*i_);
            sleep(std::chrono::milliseconds { 500 });
        }
    }

private:
    std::unique_ptr<int> i_;
};
BOOST_DLL_ALIAS(SimplePluginB::create, create_b);
