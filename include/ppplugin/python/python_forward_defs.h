#ifndef PPPLUGIN_PYTHON_FORWARD_DEFS_H
#define PPPLUGIN_PYTHON_FORWARD_DEFS_H

// TODO: these types is not a part of the official API;
//       consider alternatives like storing void* and casting to actual
//       type in source file (via helper function)

// NOLINTNEXTLINE(bugprone-reserved-identifier,readability-identifier-naming)
struct _ts; // Python defines this type name
using PyThreadState = _ts;
// NOLINTNEXTLINE(bugprone-reserved-identifier,readability-identifier-naming)
struct _object; // Python defines this type name
using PyObject = _object;

#endif // PPPLUGIN_PYTHON_FORWARD_DEFS_H
