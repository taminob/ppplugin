#ifndef PPPLUGIN_EXAMPLES_CPP_PLUGIN_H
#define PPPLUGIN_EXAMPLES_CPP_PLUGIN_H

#include <chrono>
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

    virtual void initialize(int /*value*/) { }

    virtual void doSomething() = 0;

protected:
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

    // must be virtual to be accessible for the program that loads this plugin;
    // otherwise, trying to call the function will cause a linker error
    virtual void loop();

    void doSomething() override;

private:
    int i_ { 0 };
};

extern int global_variable;

#endif // PPPLUGIN_EXAMPLES_CPP_PLUGIN_H
