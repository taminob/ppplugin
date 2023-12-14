#ifndef PPPLUGIN_PLUGIN_H
#define PPPLUGIN_PLUGIN_H

#include <boost/dll.hpp>

namespace ppplugin {
class Plugin {
 public:
  Plugin() = default;

  virtual ~Plugin() = default;
  Plugin(const Plugin&) = default;
  Plugin(Plugin&&) noexcept = default;
  Plugin& operator=(const Plugin&) = default;
  Plugin& operator=(Plugin&&) noexcept = default;

  /** Returns the name of the plugin. */
  [[nodiscard]] virtual std::string getName() const = 0;
};
}  // namespace ppplugin

#endif  // PPPLUGIN_PLUGIN_H
