#ifndef PPPLUGIN_DETAIL_STRING_UTILS_H
#define PPPLUGIN_DETAIL_STRING_UTILS_H

#include <charconv>
#include <optional>
#include <string_view>

namespace ppplugin {
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
} // namespace ppplugin

#endif // PPPLUGIN_DETAIL_STRING_UTILS_H
