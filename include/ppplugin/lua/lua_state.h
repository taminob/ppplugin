#ifndef PPPLUGIN_LUA_STATE_H
#define PPPLUGIN_LUA_STATE_H

#include "lua_helpers.h"
#include "ppplugin/detail/compatibility_utils.h"
#include "ppplugin/detail/function_details.h"
#include "ppplugin/detail/template_helpers.h"
#include "ppplugin/errors.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>

struct lua_State;

namespace ppplugin {
class LuaState {
public:
    LuaState();

    /**
     * Wrap given state in a LuaState and allow access to methods,
     * but do not claim ownership of the state.
     */
    [[nodiscard]] static LuaState wrap(lua_State* state) { return LuaState(state); }

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
    [[nodiscard]] std::optional<T> pop(bool always_pop = false);
    /**
     * Pop upmost element of type function from Lua stack.
     *
     * @return std::function wrapped in optional of given function type.
     */
    template <typename T, std::enable_if_t<std::is_function_v<T>, bool> = true>
    [[nodiscard]] auto pop(bool always_pop = false);

    /**
     * Get top-most stack value.
     */
    template <typename T, std::enable_if_t<!std::is_function_v<T>, bool> = true>
    [[nodiscard]] std::optional<T> top();
    template <typename T, std::enable_if_t<std::is_function_v<T>, bool> = true>
    [[nodiscard]] auto top();

    /**
     * Mark top-most stack value as global variable with given name.
     */
    void markGlobal(const std::string& variable_name);

    /**
     * Push value of global variable with given name to stack.
     *
     * @return true if global of given name exists, otherwise false
     */
    [[nodiscard]] bool pushGlobal(const std::string& variable_name);

    /**
     * Check if top-most stack value is of type bool.
     */
    [[nodiscard]] bool isBool();
    /**
     * Check if top-most stack value is of type number.
     */
    [[nodiscard]] bool isNumber();
    /**
     * Check if top-most stack value is of type string.
     */
    [[nodiscard]] bool isString();
    /**
     * Check if top-most stack value is of type function.
     */
    [[nodiscard]] bool isFunction();
    /**
     * Check if top-most stack value is of type table (either array or map).
     */
    [[nodiscard]] bool isTable();

    /**
     * Register function handler to be called in case of a Lua panic.
     */
    void registerPanicHandler(LuaCFunction handler);

    /**
     * Dump stack between given indices to stream.
     */
    void dumpStack(std::ostream& out, int start_index = 1, int end_index = -1
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
        ,
        const std::source_location& location = std::source_location::current()
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    );

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
    [[nodiscard]] auto topFunction();
    [[nodiscard]] const void* topPointer();
    template <typename T>
    [[nodiscard]] std::optional<T> topMap();
    template <typename T>
    [[nodiscard]] std::optional<T> topArray();

    /**
     * Check if top-most stack value is of type nil.
     */
    [[nodiscard]] bool isNil();

    /**
     * Pop top-most value if it is a function.
     *
     * @note Call of this function and the returned function must not have
     *       any other stack-modifying operation in-between.
     */
    template <typename Func>
    std::optional<std::function<Func>> popFunction(bool always_pop = false);

    void pushOne(double value);
    // NOLINTBEGIN(google-runtime-int)
    // TODO: use template instead
    void pushOne(unsigned int value) { pushOne(static_cast<long long>(value)); }
    void pushOne(int value) { pushOne(static_cast<long long>(value)); }
    void pushOne(unsigned long value) { pushOne(static_cast<long long>(value)); }
    void pushOne(long value) { pushOne(static_cast<long long>(value)); }
    void pushOne(unsigned long long value) { pushOne(static_cast<long long>(value)); }
    void pushOne(long long value);
    // NOLINTEND(google-runtime-int)
    void pushOne(const char* value);
    void pushOne(std::string_view value);
    void pushOne(bool value);
    void pushOne(std::nullptr_t);
    void pushOne(LuaCFunction func);

    // TODO: support additional container types
    template <typename T>
    void pushOne(const std::vector<T>& value);
    template <typename K, typename V>
    void pushOne(const std::map<K, V>& value);

    /**
     * Start creation of table.
     * All following push() calls will be used alternating as key and value.
     *
     * @return index of table which must be passed to endTable() to complete table creation
     */
    int startTable(std::size_t size_hint, bool is_array);

    /**
     * Finalize table.
     * Stack must contain as many (key, value) pairs as the given table size indicates and
     * no other elements are allowed on the stack in-between.
     */
    void endTable(int table_index, std::size_t table_size);

    /**
     * Call function on stack with given number of arguments from stack.
     * The function has to be in right below the arguments on the stack.
     *
     * @return 0 on success, non-zero value on error
     */
    [[nodiscard]] int pcall(std::size_t argument_count, std::size_t return_count);

    /**
     * Discard top-most stack value.
     */
    void discardTop();

    /**
     * Push next table item (first key, second value) to stack.
     * Before first call, top-most stack value must be of type table.
     * Before next call, pop the value (top element of stack).
     * To abort iteration, remove both key and value from stack.
     *
     * @param is_first_iteration Must be true to start the iteration process
     *
     * @return true if the next key/value was pushed to the stack,
     *         false if there are no more elements in the table
     */
    bool pushNextTableItem(bool is_first_iteration);

private:
    std::unique_ptr<lua_State, void (*)(lua_State*)> state_;
};

template <typename T, typename... Args>
void LuaState::push(T&& arg, Args&&... args)
{
    pushOne(std::forward<T>(arg));
    if constexpr (sizeof...(Args) > 0) {
        push(std::forward<Args>(args)...);
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
    using PlainT = detail::templates::RemoveCvrefT<T>;
    if constexpr (std::is_same_v<PlainT, double>) {
        return topDouble();
    } else if constexpr (std::is_same_v<PlainT, int>) {
        return topInt();
    } else if constexpr (std::is_same_v<PlainT, bool>) {
        return topBool();
    } else if constexpr (std::is_same_v<PlainT, std::string>) {
        return topString();
    } else if constexpr (detail::templates::IsSpecializationV<T, std::map>) {
        return topMap<T>();
    } else if constexpr (detail::templates::IsSpecializationV<T, std::vector>) {
        return topArray<T>();
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
        // TODO: proper error checking
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

template <typename T>
std::optional<T> LuaState::topMap()
{
    if (isTable()) {
        T result;

        bool is_first_iteration = true;
        while (pushNextTableItem(is_first_iteration)) {
            is_first_iteration = false;

            // pop value but keep key to iterate to next element
            auto value = pop<typename T::mapped_type>(true);
            auto key = top<typename T::key_type>();

            if (key.has_value() && value.has_value()) {
                result.emplace(*std::move(key), *std::move(value));
            } else {
                discardTop(); // abort iteration by discarding key
                // key or value have wrong type
                return std::nullopt;
            }
        }
        return result;
    }
    return std::nullopt;
}

template <typename T>
std::optional<T> LuaState::topArray()
{
    // retrieve table as sorted map first since iteration is not guaranteed to
    // be in correct order
    auto array_map = topMap<std::map<int, typename T::value_type>>();
    if (!array_map.has_value()) {
        // not a table or does not match given type
        return std::nullopt;
    }

    T result;
    for (auto& [index, value] : *array_map) {
        if (index - 1 != static_cast<int>(result.size())) { // Lua indices start at 1
            // keys not continuous, thus it cannot be turned into an array
            return std::nullopt;
        }
        result.push_back(std::move(value));
    }
    return result;
}

template <typename T>
void LuaState::pushOne(const std::vector<T>& value)
{
    auto table_index = startTable(value.size(), true);
    for (std::size_t index = 0; index < value.size(); ++index) {
        pushOne(index + 1); // Lua arrays start at 1
        push(value[index]);
    }
    endTable(table_index, value.size());
}

template <typename K, typename V>
void LuaState::pushOne(const std::map<K, V>& value)
{
    auto table_index = startTable(value.size(), false);
    for (auto&& [key, element] : value) {
        pushOne(key);
        push(element);
    }
    endTable(table_index, value.size());
}
} // namespace ppplugin

#endif // PPPLUGIN_LUA_STATE_H
