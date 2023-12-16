#ifndef PPPLUGIN_DETAIL_COMPATIBILITY_UTILS_H
#define PPPLUGIN_DETAIL_COMPATIBILITY_UTILS_H

#ifdef PPPLUGIN_CPP17_COMPATIBILITY
#define PPPLUGIN_MODULE_EXPORT
#define PPPLUGIN_MODULE_IMPORT
#define PPPLUGIN_MODULE_NAME(name)
#define PPPLUGIN_REQUIRES(...)

#include <fmt/core.h>
#include <fmt/format.h>
namespace ppplugin {
using fmt::format;
} // namespace ppplugin
#else
#define PPPLUGIN_MODULE_EXPORT export
#define PPPLUGIN_MODULE_IMPORT
#define PPPLUGIN_MODULE_NAME(name) module name
#define PPPLUGIN_REQUIRES(...) requires

#include <format>
namespace ppplugin {
using std::format;
} // namespace ppplugin
#endif // PPPLUGIN_CPP17_COMPATIBILITY

#endif // PPPLUGIN_DETAIL_COMPATIBILITY_UTILS_H
