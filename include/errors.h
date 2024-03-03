#ifndef PPPLUGIN_ERRORS_H
#define PPPLUGIN_ERRORS_H

#include "expected.h"

#include <memory>
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
    CallError(Code error_code)
        : error_ { error_code }
    {
    }

    CallError(Code error, std::string_view what)
        : error_ { error }
        , what_ { what }
    {
    }

    [[nodiscard]] CallError error() const { return error_; }
    [[nodiscard]] const std::string& what() const { return what_; }

    friend constexpr bool operator==(const CallError& lhs, const CallError& rhs)
    {
        return lhs.error_ == rhs.error_;
    }

private:
    Code error_;
    std::string what_;
};

enum class LoadError {
    unknown,
    fileNotFound,
    fileInvalid,
};

template <typename ReturnType>
using CallResult = Expected<ReturnType, CallError>;

} // namespace ppplugin

#endif // PPPLUGIN_ERRORS_H
