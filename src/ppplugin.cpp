#include "plugin.h"
#include "plugin_manager.h"
#include "detail/segfault_handling.h"

namespace ppplugin::detail::segfault_handling {
std::mutex segfault_checker_mutex;
std::jmp_buf segfault_recover_point;
} // namespace ppplugin::detail::segfault_handling
