#ifndef PPPLUGIN_SHELL_TYPE_CONVERSION_H
#define PPPLUGIN_SHELL_TYPE_CONVERSION_H

#include "ppplugin/detail/string_utils.h"
#include "ppplugin/detail/template_helpers.h"

#include <string>
#include <type_traits>
#include <vector>

#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>

namespace ppplugin {
/**
 *
 */
template <typename T>
std::string convertToShellString(T&& value)
{
    using Type = detail::templates::RemoveCvrefT<T>;

    if constexpr (detail::IsStringlikeV<Type>) {
        std::string result { std::forward<T>(value) };
        boost::replace_all(result, "'", "'\\''");
        return '\'' + result + '\'';
    } else if constexpr (detail::templates::IsSpecializationV<Type, std::vector>) {
        std::string result;
        for (auto&& element : std::forward<T>(value)) {
            if (!result.empty()) {
                result += " ";
            }
            result += convertToShellString(element);
        }
        return result;
    } else if constexpr (std::is_integral_v<Type> || std::is_floating_point_v<Type>) {
        return format("'{}'", std::forward<T>(value));
    } else {
        static_assert(!sizeof(Type),
            "Unsupported type to be converted to a shell compatible string!");
    }
}

template <typename T>
std::optional<T> convertFromShellString(std::string_view value)
{
    if constexpr (detail::IsStringlikeV<T>) {
        return T { value };
    } else if constexpr (detail::templates::IsSpecializationV<T, std::vector>) {
        // TODO: handle single quotes (using Boost.Spirit?)
        const boost::escaped_list_separator<char> separator {
            '\\', // escape character
            ' ', // separator character
            '"' // quote character
        };
        T result;
        for (auto&& element : boost::tokenizer { value, separator }) {
            result.push_back(element);
        }
        return result;
    } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
        T result;
        auto conversion_result = std::from_chars(value.begin(), value.end(), result);
        if (conversion_result.ec != std::errc {} || conversion_result.ptr != value.end()) {
            return std::nullopt;
        }
        return result;
    } else {
        static_assert(!sizeof(T),
            "Unsupported type to be converted from a shell string!");
    }
}

[[nodiscard]] inline bool isValidShellVariableName(std::string_view variable_name, bool strict = false)
{
    if (!std::none_of(variable_name.begin(), variable_name.end(), [](auto&& character) {
            return character == '=' || character == '\0';
        })) {
        // check formal requirements of a POSIX shell variable name (no '=' and no NULL)
        return false;
    }
    if (variable_name.empty()) {
        return false;
    }
    if (!strict) {
        return true;
    }

    // optional, additional checks that most shells require
    if (std::isalpha(variable_name.front()) != 0 || variable_name.front() == '_') {
        // the name cannot start with a number
        return false;
    }
    if (std::all_of(variable_name.begin(), variable_name.end(), [](auto&& character) {
            return std::isalnum(character) || character == '_';
        })) {
        // the name cannot contain anything besides alphabetic letters, numbers and underscores
        return false;
    }
    return true;
}
} // namespace ppplugin

#endif // PPPLUGIN_SHELL_TYPE_CONVERSION_H
