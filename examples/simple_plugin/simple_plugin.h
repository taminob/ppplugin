#ifndef PPPLUGIN_EXAMPLES_SIMPLE_PLUGIN_H
#define PPPLUGIN_EXAMPLES_SIMPLE_PLUGIN_H

#include <iostream>
#include <memory>
#include <thread>

class SimplePluginInterface {
public:
    SimplePluginInterface() = default;
    virtual ~SimplePluginInterface() = default;
    SimplePluginInterface(const SimplePluginInterface&) = default;
    SimplePluginInterface(SimplePluginInterface&&) noexcept = default;
    SimplePluginInterface& operator=(const SimplePluginInterface&) = default;
    SimplePluginInterface& operator=(SimplePluginInterface&&) noexcept = default;

    virtual void initialize() { }
    virtual void loop() = 0;

protected:
    template <typename T>
    void print(T&& arg)
    {
        std::cout << std::forward<T>(arg) << std::flush;
    }
    static void sleep(std::chrono::milliseconds duration)
    {
        std::this_thread::sleep_for(duration);
    }
};

class SimplePluginA : public SimplePluginInterface {
public:
    SimplePluginA() = default;

    static std::shared_ptr<SimplePluginA> create()
    {
        return std::make_shared<SimplePluginA>();
    }

    void loop() override;

private:
    int i_ { 0 };
};

#endif // PPPLUGIN_EXAMPLES_SIMPLE_PLUGIN_H
