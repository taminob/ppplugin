#ifndef PPPLUGIN_ERRORS_H
#define PPPLUGIN_ERRORS_H

#include "ppplugin/detail/compatibility_utils.h"
#include "ppplugin/expected.h"

#include <memory>
#ifndef PPPLUGIN_CPP17_COMPATIBILITY
#include <source_location>
#endif // PPPLUGIN_CPP17_COMPATIBILITY
#include <string>

namespace ppplugin {
class CallError {
public:
    enum class Code {
        unknown,
        notLoaded,
        symbolNotFound,
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

    [[nodiscard]] CallError error() const { return error_; }
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

enum class LoadError {
    unknown,
    fileNotFound,
    fileInvalid,
    fileNotReadable,
};

template <typename ReturnType>
using CallResult = Expected<ReturnType, CallError>;

} // namespace ppplugin

#endif // PPPLUGIN_ERRORS_H
