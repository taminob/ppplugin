#ifndef PPPLUGIN_PYTHON_EXCEPTION_H
#define PPPLUGIN_PYTHON_EXCEPTION_H

#include <optional>
#include <string>

namespace ppplugin {
class PythonException {
public:
    PythonException() = default;
    [[nodiscard]] static std::optional<PythonException> latest();

    [[nodiscard]] std::string toString() const;

    explicit operator bool()
    {
        return type_ && value_ && traceback_;
    }

private:
    std::optional<std::string> type_;
    std::optional<std::string> value_;
    std::optional<std::string> traceback_;
};
} // namespace ppplugin

#endif // PPPLUGIN_PYTHON_EXCEPTION_H
