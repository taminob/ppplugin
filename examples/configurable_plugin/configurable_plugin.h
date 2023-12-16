#ifndef PPPLUGIN__EXAMPLES__SIMPLE_PLUGIN_H
#define PPPLUGIN__EXAMPLES__SIMPLE_PLUGIN_H

#include <iostream>
#include <memory>
#include <thread>

struct PluginConfiguration {
    std::chrono::milliseconds printInterval {};
    std::string printValue;
};

class ConfigurablePluginInterface {
public:
    explicit ConfigurablePluginInterface(PluginConfiguration config)
        : configuration_ { std::move(config) }
    {
    }
    virtual ~ConfigurablePluginInterface() = default;
    ConfigurablePluginInterface(const ConfigurablePluginInterface&) = default;
    ConfigurablePluginInterface(ConfigurablePluginInterface&&) noexcept = default;
    ConfigurablePluginInterface& operator=(const ConfigurablePluginInterface&) = default;
    ConfigurablePluginInterface& operator=(ConfigurablePluginInterface&&) noexcept = default;

    virtual void loop(const std::atomic<bool>& stop) = 0;

protected:
    template <typename T>
    void print(T&& arg)
    {
        std::cout << arg << std::flush;
    }
    static void sleep(std::chrono::milliseconds duration)
    {
        std::this_thread::sleep_for(duration);
    }
    [[nodiscard]] const PluginConfiguration& configuration() const { return configuration_; }

private:
    PluginConfiguration configuration_;
};

class ConfigurablePluginA : public ConfigurablePluginInterface {
public:
    using ConfigurablePluginInterface::ConfigurablePluginInterface;

    static std::shared_ptr<ConfigurablePluginA> create(
        PluginConfiguration config)
    {
        return std::make_shared<ConfigurablePluginA>(std::move(config));
    }

    void loop(const std::atomic<bool>& stop) override;
};

#endif // PPPLUGIN__EXAMPLES__SIMPLE_PLUGIN_H
