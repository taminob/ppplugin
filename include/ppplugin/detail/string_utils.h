#ifndef PPPLUGIN_DETAIL_STRING_UTILS_H
#define PPPLUGIN_DETAIL_STRING_UTILS_H

#include "template_helpers.h"

#include <charconv>
#include <optional>
#include <string_view>

namespace ppplugin::detail {
template <typename T>
constexpr auto IsStringlikeV = // NOLINT(readability-identifier-naming)
    templates::IsAnyOfV<T, std::string, std::string_view, const char*>;

[[nodiscard]] inline bool endsWith(std::string_view string, std::string_view end)
{
    if (string.size() < end.size()) {
        return false;
    }
    auto result = string.compare(string.size() - end.size(), end.size(), end);
    return result == 0;
}

template <typename Integer>
[[nodiscard]] inline std::optional<Integer> toInteger(std::string_view string)
{
    int value {};
    auto result = std::from_chars(string.begin(), string.end(), value);
    if (result.ec == std::errc {} && result.ptr == string.end()) {
        return value;
    }
    return std::nullopt;
}
} // namespace ppplugin::detail

#endif // PPPLUGIN_DETAIL_STRING_UTILS_H
