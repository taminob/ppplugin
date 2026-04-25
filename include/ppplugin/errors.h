#ifndef PPPLUGIN_ERRORS_H
#define PPPLUGIN_ERRORS_H

#include "ppplugin/detail/compatibility_utils.h"
#include "ppplugin/expected.h"

#include <cstdint>
#include <ostream>
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
#include <source_location>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
#include <string>
#include <string_view>

namespace ppplugin {
template <typename Code>
class Error {
public:
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    Error(Code error_code
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
        ,
        const std::source_location& location = std::source_location::current()
#endif // PPPLUGIN_CPP17_COMPATIBILITY
            )
        : error_ { error_code }
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
        , location_ { location }
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    {
    }

    Error(Code error, std::string_view what
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
        ,
        const std::source_location& location = std::source_location::current()
#endif // PPPLUGIN_CPP17_COMPATIBILITY
            )
        : error_ { error }
        , what_ { what }
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
        , location_ { location }
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    {
    }

    [[nodiscard]] Code code() const { return error_; }
    [[nodiscard]] const std::string& what() const { return what_; }
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    [[nodiscard]] const std::source_location& location() const
    {
        return location_;
    }
    [[nodiscard]] std::string where() const
    {
        return format("{}:{} in {}",
            location_.file_name(),
            location_.line(),
            location_.function_name());
    }
#endif // PPPLUGIN_CPP17_COMPATIBILITY

    friend constexpr bool operator==(const Error& lhs, const Error& rhs)
    {
        return lhs.error_ == rhs.error_;
    }

private:
    Code error_;
    std::string what_;
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    std::source_location location_;
#endif // PPPLUGIN_CPP17_COMPATIBILITY
};

template <typename Code>
std::ostream& operator<<(std::ostream& out, const Error<Code>& error)
{
    auto&& code = codeToString(error.code());
    out << "[" << code << "]";
    auto&& what = error.what();
    if (!what.empty()) {
        out << " " << what;
    }
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
    out << error.where();
#endif // PPPLUGIN_CPP17_COMPATIBILITY
    return out;
}

enum class CallErrorCode : std::uint8_t {
    unknown,
    notLoaded,
    symbolNotFound,
    incorrectType,
};

[[nodiscard]] static constexpr std::string_view codeToString(CallErrorCode code)
{
    switch (code) {
    case CallErrorCode::incorrectType:
        return "incorrect type";
    case CallErrorCode::notLoaded:
        return "not loaded";
    case CallErrorCode::symbolNotFound:
        return "symbol not found";
    case CallErrorCode::unknown:
    default:
        return "unknown";
    }
}

using CallError = Error<CallErrorCode>;

enum class LoadErrorCode : std::uint8_t {
    unknown,
    fileNotFound,
    fileInvalid,
    fileNotReadable,
};

[[nodiscard]] static constexpr std::string_view codeToString(LoadErrorCode code)
{
    switch (code) {
    case LoadErrorCode::fileNotReadable:
        return "file not readable";
    case LoadErrorCode::fileInvalid:
        return "file invalid";
    case LoadErrorCode::fileNotFound:
        return "file not found";
    case LoadErrorCode::unknown:
    default:
        return "unknown";
    }
}
using LoadError = Error<LoadErrorCode>;

template <typename ReturnType>
using CallResult = Expected<ReturnType, CallError>;

} // namespace ppplugin

#endif // PPPLUGIN_ERRORS_H
