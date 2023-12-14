#ifndef PPPLUGIN__EXAMPLES__SIMPLE_PLUGIN_H
#define PPPLUGIN__EXAMPLES__SIMPLE_PLUGIN_H

#include <iostream>
#include <thread>

#include "plugin.h"

class SimplePluginInterface : public ppplugin::Plugin {
 public:
  SimplePluginInterface() = default;

  virtual void initialize() {}
  virtual void loop() = 0;

 protected:
  template <typename T>
  void print(T&& arg) {
    std::cout << arg << std::flush;
  }
  static void sleep(std::chrono::milliseconds duration) {
    std::this_thread::sleep_for(duration);
  }
};

class SimplePluginA : public SimplePluginInterface {
 public:
  SimplePluginA() = default;

  static std::shared_ptr<SimplePluginA> create() {
    return std::make_shared<SimplePluginA>();
  }

  [[nodiscard]] std::string getName() const override { return "a"; }

  void loop() override;

 private:
  int i_{0};
};

class SimplePluginB : public SimplePluginInterface {
 public:
  SimplePluginB() = default;

  static std::shared_ptr<SimplePluginB> create() {
    return std::make_shared<SimplePluginB>();
  }

  [[nodiscard]] std::string getName() const override { return "b"; }

  void initialize() override;
  void loop() override;

 private:
  std::unique_ptr<int> i_;
};

#endif  // PPPLUGIN__EXAMPLES__SIMPLE_PLUGIN_H
