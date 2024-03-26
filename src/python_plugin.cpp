#include "python/plugin.h"

#include "detail/compatibility_utils.h"
#include "errors.h"
#include "expected.h"

#include <filesystem>
#include <optional>
#include <string>

#include <boost/python.hpp>

//#include <Python.h>
#include <pyerrors.h>
#include <pylifecycle.h>
#include <pytypedefs.h>

namespace ppplugin {
Expected<PythonPlugin, LoadError> PythonPlugin::load(const std::filesystem::path& python_script_path)
{
    if (!std::filesystem::exists(python_script_path)) {
        return LoadError::fileNotFound;
    }
    PythonPlugin new_plugin {};
    if (auto load_error = new_plugin.interpreter_.load(python_script_path)) {
        return *load_error;
    }
    return new_plugin;
}
} // namespace ppplugin
