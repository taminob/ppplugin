#ifndef PPPLUGIN_ERRORS_H
#define PPPLUGIN_ERRORS_H

#include "ppplugin/detail/compatibility_utils.h"
#include "ppplugin/expected.h"

#ifndef PPPLUGIN_CPP17_COMPATIBILITY
#include <source_location>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
#include <string>
#include <string_view>

namespace ppplugin {
class CallError {
public:
    enum class Code {
        unknown,
        notLoaded,
        symbolNotFound,
        incorrectType,
    };

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    CallError(Code error_code
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

    CallError(Code error, std::string_view what
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

    friend constexpr bool operator==(const CallError& lhs, const CallError& rhs)
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

[[nodiscard]] static constexpr std::string_view codeToString(CallError::Code code)
{
    switch (code) {
    case CallError::Code::incorrectType:
        return "incorrect type";
    case CallError::Code::notLoaded:
        return "not loaded";
    case CallError::Code::symbolNotFound:
        return "symbol not found";
    case CallError::Code::unknown:
    default:
        return "unknown";
    }
}

enum class LoadError {
    unknown,
    fileNotFound,
    fileInvalid,
    fileNotReadable,
};

[[nodiscard]] static constexpr std::string_view codeToString(LoadError code)
{
    switch (code) {
    case LoadError::fileNotReadable:
        return "file not readable";
    case LoadError::fileInvalid:
        return "file invalid";
    case LoadError::fileNotFound:
        return "file not found";
    case LoadError::unknown:
    default:
        return "unknown";
    }
}

template <typename ReturnType>
using CallResult = Expected<ReturnType, CallError>;

} // namespace ppplugin

#endif // PPPLUGIN_ERRORS_H
