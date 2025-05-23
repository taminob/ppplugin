#ifndef PPPLUGIN_LUA_SCRIPT_H
#define PPPLUGIN_LUA_SCRIPT_H

#include "lua_state.h"
#include "ppplugin/errors.h"
#include "ppplugin/expected.h"

#include <filesystem>
#include <optional>
#include <string>

namespace ppplugin {
class LuaScript {
public:
    /**
     * Load given Lua script.
     *
     * @param auto_run if set to true, the script will be run immediately after loading;
     *                 if set to false, functions cannot be called since they haven't been
     *                 discovered yet (required execution)
     */
    static Expected<LuaScript, LoadError> load(const std::filesystem::path& script_path, bool auto_run = true);

    ~LuaScript() = default;
    LuaScript(const LuaScript&) = delete;
    LuaScript(LuaScript&&) noexcept = default;
    LuaScript& operator=(const LuaScript&) = delete;
    LuaScript& operator=(LuaScript&&) noexcept = default;

    template <typename ReturnValue, typename... Args>
    [[nodiscard]] CallResult<ReturnValue> call(const std::string& function_name, Args&&... args);

    template <typename VariableType>
    [[nodiscard]] CallResult<VariableType> global(const std::string& variable_name);
    template <typename VariableType>
    void global(const std::string& variable_name, VariableType&& new_value);

private:
    LuaScript();

    [[nodiscard]] bool run();

    [[nodiscard]] std::optional<LoadError> loadFile(const std::filesystem::path& lua_file, bool auto_run);

    static void pcall();

    template <typename... Args>
    void push(Args&&... args);

    [[nodiscard]] static std::string errorToString(int error_code);

    struct StackContext {
        // TODO: push error handling function to stack
        //       and recover previous stack state on destruction
        template <typename... Args>
        explicit StackContext(Args&&... /*args*/)
        {
        }
    };

private:
    LuaState state_;
};

template <typename ReturnValue, typename... Args>
CallResult<ReturnValue> LuaScript::call(const std::string& function_name, Args&&... args)
{
    if (state_.pushGlobal(function_name)) {
        if (auto function = state_.top<ReturnValue(Args...)>()) {
            return (*function)(std::forward<Args>(args)...);
        }
        return CallError { CallError::Code::unknown, "Symbol does not match given type" };
    }
    return { CallError::Code::symbolNotFound };
}

template <typename VariableType>
CallResult<VariableType> LuaScript::global(const std::string& variable_name)
{
    if (state_.pushGlobal(variable_name)) {
        if (auto value = state_.pop<VariableType>()) {
            return value.value();
        }
        return { CallError::Code::incorrectType };
    }
    return { CallError::Code::symbolNotFound };
}
template <typename VariableType>
void LuaScript::global(const std::string& variable_name, VariableType&& new_value)
{
    push(std::forward<VariableType>(new_value));
    state_.markGlobal(variable_name);
}

template <typename... Args>
void LuaScript::push(Args&&... args)
{
    if constexpr (sizeof...(Args) > 0) {
        state_.push(std::forward<Args>(args)...);
    }
}

} // namespace ppplugin

#endif // PPPLUGIN_LUA_SCRIPT_H
