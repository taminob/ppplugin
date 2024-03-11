#ifndef PPPLUGIN_LUA_STATE_H
#define PPPLUGIN_LUA_STATE_H

#include "detail/compatibility_utils.h"
#include "detail/function_details.h"
#include "errors.h"
#include "lua_helpers.h"

#include <filesystem>
#include <functional>
#include <optional>

struct lua_State;

namespace ppplugin {
enum class LuaType {
    none = -1,
    nil = 0,
    boolean = 1,
    number = 3,
    string = 4,
    function = 6,
};

class LuaState {
public:
    LuaState();

    /**
     * Wrap given state in a LuaState and allow access to methods,
     * but do not claim ownership of the state.
     */
    static LuaState wrap(lua_State* state) { return LuaState(state); }

    [[nodiscard]] lua_State* state() { return state_.get(); }
    [[nodiscard]] const lua_State* state() const { return state_.get(); }

    using LuaCFunction = int (*)(lua_State*);
    template <typename Func>
    using IsLuaCFunction = std::is_invocable_r<int, Func, lua_State*>;

    /**
     * Push given value to top of stack.
     */
    template <typename T, typename... Args>
    void push(T&& arg, Args&&... args);

    /**
     * Pop upmost element from Lua stack.
     *
     * @param always_pop if true, always pop even if the element has a different type and
     *                   its value is not returned by the function.
     *
     * @return top-most stack value or std::nullopt if given type does not match
     */
    template <typename T, std::enable_if_t<!std::is_function_v<T>, bool> = true>
    std::optional<T> pop(bool always_pop = false);
    /**
     * Pop upmost element of type function from Lua stack.
     *
     * @return std::function wrapped in optional of given function type.
     */
    template <typename T, std::enable_if_t<std::is_function_v<T>, bool> = true>
    auto pop(bool always_pop = false);

    /**
     * Get top-most stack value.
     */
    template <typename T, std::enable_if_t<!std::is_function_v<T>, bool> = true>
    std::optional<T> top();
    template <typename T, std::enable_if_t<std::is_function_v<T>, bool> = true>
    auto top();

    /**
     * Mark top-most stack value as global variable with given name.
     */
    void markGlobal(const std::string& variable_name);

    /**
     * Push value of global variable with given name to stack.
     *
     * @return true if global of given name exists, otherwise false
     */
    bool pushGlobal(const std::string& variable_name);

    /**
     * Check if top-most stack value is of type bool.
     */
    bool isBool();
    /**
     * Check if top-most stack value is of type number.
     */
    bool isNumber();
    /**
     * Check if top-most stack value is of type string.
     */
    bool isString();
    /**
     * Check if top-most stack value is of type function.
     */
    bool isFunction();

    /**
     * Register function handler to be called in case of a Lua panic.
     */
    void registerPanicHandler(LuaCFunction handler);

private:
    /**
     * Create non-owning LuaState instance of given state.
     * The given state will remain valid after this instance is destroyed.
     */
    explicit LuaState(lua_State* state);

    [[nodiscard]] std::optional<std::string> topString();
    [[nodiscard]] std::optional<bool> topBool();
    [[nodiscard]] std::optional<int> topInt();
    [[nodiscard]] std::optional<double> topDouble();
    template <typename T>
    auto topFunction();
    const void* topPointer();

    /**
     * Pop top-most value if it is a function.
     *
     * @note Call of this function and the returned function must not have
     *       any other stack-modifying operation in-between.
     */
    template <typename Func>
    std::optional<std::function<Func>> popFunction(bool always_pop = false);

    void pushOne(double value);
    void pushOne(int value);
    void pushOne(const char* value);
    void pushOne(std::string_view value);
    void pushOne(bool value);
    void pushOne(std::nullptr_t);
    void pushOne(LuaCFunction func);

    /**
     * Call function on stack with given number of arguments from stack.
     * The function has to be in right below the arguments on the stack.
     *
     * @return 0 on success, non-zero value on error
     */
    int pcall(std::size_t argument_count, std::size_t return_count);

    /**
     * Discard top-most stack value.
     */
    void discardTop();

private:
    std::unique_ptr<lua_State, void (*)(lua_State*)> state_;
};

template <typename T, typename... Args>
void LuaState::push(T&& arg, Args&&... args)
{
    pushOne(std::forward<T>(arg));
    if constexpr (sizeof...(Args) > 0) {
        push(std::forward<Args...>(args...));
    }
}

template <typename T, std::enable_if_t<!std::is_function_v<T>, bool>>
std::optional<T> LuaState::pop(bool always_pop)
{
    auto result = top<T>();
    if (result.has_value() || always_pop) {
        discardTop();
    }
    return result;
}

template <typename T, std::enable_if_t<std::is_function_v<T>, bool>>
auto LuaState::pop(bool always_pop)
{
    // TODO: remove this specialization?
    auto result = topFunction<T>();
    if (result.has_value() || always_pop) {
        discardTop();
    }
    return result;
}

template <typename T, std::enable_if_t<!std::is_function_v<T>, bool>>
std::optional<T> LuaState::top()
{
    using PlainT = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<PlainT, double>) {
        return topDouble();
    } else if constexpr (std::is_same_v<PlainT, int>) {
        return topInt();
    } else if constexpr (std::is_same_v<PlainT, bool>) {
        return topBool();
    } else if constexpr (std::is_same_v<PlainT, std::string>) {
        return topString();
    } else {
        static_assert(!sizeof(T), "Unsupported type!");
    }
    return std::nullopt;
}

template <typename T, std::enable_if_t<std::is_function_v<T>, bool>>
auto LuaState::top()
{
    return topFunction<T>();
}

template <typename T>
auto LuaState::topFunction()
{
    using FunctionDetails = detail::templates::FunctionDetails<T>;
    constexpr auto RETURN_TYPE_COUNT = detail::templates::returnTypeCount<FunctionDetails>();

    auto top_function = [this, function_id = topPointer()](auto&&... args)
        -> CallResult<typename FunctionDetails::ReturnType> {
        if (function_id != topPointer()) {
            return CallError { CallError::Code::unknown,
                "Invalid stack content" };
        }
        if constexpr (sizeof...(args) > 0) {
            push(std::forward<decltype(args)>(args)...);
        }
        auto error = pcall(sizeof...(args), RETURN_TYPE_COUNT);
        // TODO
        if (error != 0) {
            return CallError {
                CallError::Code::unknown,
                format("Unable to call function. Code: '{}'. Error: '{}'",
                    error,
                    error == 2 ? pop<std::string>().value_or("?") : "")
            };
        }

        if constexpr (RETURN_TYPE_COUNT > 1) {
            if (auto result = PopTuple<typename FunctionDetails::ReturnType>::pop(*this)) {
                return CallResult<typename FunctionDetails::ReturnType> { *result };
            }
            return CallError { CallError::Code::unknown,
                "Wrong return type" };
        } else if constexpr (RETURN_TYPE_COUNT == 1) {
            if (auto result = pop<typename FunctionDetails::ReturnType>()) {
                return CallResult<typename FunctionDetails::ReturnType> { *result };
            }
            return CallError { CallError::Code::unknown,
                "Wrong return type" };
        } else {
            return CallResult<void> {};
        }
    };
    if (isFunction()) {
        return std::optional { top_function };
    }
    return std::optional<decltype(top_function)> { std::nullopt };
}

} // namespace ppplugin

#endif // PPPLUGIN_LUA_STATE_H
