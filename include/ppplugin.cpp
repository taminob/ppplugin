// this source file includes all header files of the library;
// it allows the language server to infer the correct compilation flags for header
// files that do not have a corresponding source file; additionally, this will
// cause the compiler to compile check them in the compilation process, even if
// they are not used elsewhere in the library (external interface)

#include "ppplugin/errors.h"
#include "ppplugin/expected.h"
#include "ppplugin/noop_plugin.h"
#include "ppplugin/plugin.h"
#include "ppplugin/plugin_manager.h"

#include "ppplugin/c/plugin.h"

#include "ppplugin/cpp/plugin.h"

#include "ppplugin/lua/lua_helpers.h"
#include "ppplugin/lua/lua_script.h"
#include "ppplugin/lua/lua_state.h"
#include "ppplugin/lua/plugin.h"

#include "ppplugin/shell/plugin.h"
#include "ppplugin/shell/shell_session.h"

#include "ppplugin/python/plugin.h"
#include "ppplugin/python/python_exception.h"
#include "ppplugin/python/python_forward_defs.h"
#include "ppplugin/python/python_guard.h"
#include "ppplugin/python/python_interpreter.h"
#include "ppplugin/python/python_object.h"
#include "ppplugin/python/python_tuple.h"

#include "ppplugin/detail/boost_dll_loader.h"
#include "ppplugin/detail/compatibility_utils.h"
#include "ppplugin/detail/compiler_info.h"
#include "ppplugin/detail/function_details.h"
#include "ppplugin/detail/scope_guard.h"
#include "ppplugin/detail/string_utils.h"
#include "ppplugin/detail/template_helpers.h"
