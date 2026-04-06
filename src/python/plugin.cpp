#include "ppplugin/errors.h"
#include "ppplugin/expected.h"
#include "ppplugin/python/plugin.h"

#include <filesystem>

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
