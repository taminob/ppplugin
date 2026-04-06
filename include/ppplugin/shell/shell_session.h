#ifndef PPPLUGIN_SHELL_SESSION_H
#define PPPLUGIN_SHELL_SESSION_H

#include "ppplugin/detail/string_utils.h"
#include "ppplugin/detail/thread_safe.h"
#include "ppplugin/errors.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <boost/algorithm/string/replace.hpp>
#include <boost/asio.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION >= 108800
#include <boost/process.hpp>
#else
#include <boost/process/v2.hpp>
#endif // BOOST_VERSION
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace ppplugin {
class ShellSession {
public:
    explicit ShellSession(const std::string& shell_binary)
        : context_ { std::make_unique<boost::asio::io_context>() }
        , stdin_pipe_ { *context_ }
        , stdout_pipe_ { std::make_unique<decltype(stdout_pipe_)::element_type>(boost::asio::readable_pipe { *context_ }) }
        , stderr_pipe_ { *context_ }
        , io_ { stdin_pipe_, *stdout_pipe_, stderr_pipe_ }
        , shell_process_ { *context_, shell_binary, {}, io_ }
        , is_running_ { std::make_unique<decltype(is_running_)::element_type>(true) }
    {
        thread_ = std::thread { [context = context_.get()]() { assert(context); runContextLoop(*context); } };

        // TODO: make movable across threads
        shell_process_.async_wait(
            [stdout_pipe = stdout_pipe_.get(), is_running = is_running_.get()](const boost::system::error_code& /*exit_status*/, int /*exit_code*/) {
                assert(stdout_pipe);
                stdout_pipe->cancel();
                assert(is_running);
                is_running->store(false);
            });
    }
    ~ShellSession()
    {
        if (shell_process_.running()) {
            // TODO? use shell_process_.request_exit() first?
            shell_process_.terminate();
        }
        if (context_) {
            context_->stop();
        }
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    ShellSession(const ShellSession&) = delete;
    ShellSession(ShellSession&&) noexcept = default;
    ShellSession& operator=(const ShellSession&) = delete;
    ShellSession& operator=(ShellSession&&) noexcept = default;

    [[nodiscard]] CallResult<void> callWithoutResult(const std::string& function_name, const std::vector<std::string>& arguments)
    {
        auto result = callWithResult(function_name, arguments);
        return result.andThen([]() { });
    }
    [[nodiscard]] CallResult<std::string> callWithResult(const std::string& function_name, const std::vector<std::string>& arguments)
    {
        if (!shell_process_.running()) {
            return CallError { CallError::Code::unknown, "Shell is not running!" };
        }
        auto command_line = '\'' + function_name + '\'';
        for (auto&& argument : arguments) {
            // TODO: escape quotes in arguments
            command_line += " '";
            auto escaped_argument = argument;
            boost::replace_all(escaped_argument, "'", "\\'");
            command_line += argument;
            command_line += "'";
        }
        auto end_marker = boost::uuids::to_string(boost::uuids::random_generator()()) + '\n';
        command_line += " ; echo :$?" + end_marker;
        std::cout << "WRITING: " << command_line;
        boost::asio::write(stdin_pipe_, boost::asio::buffer(command_line));

        std::string result;
        // use future with empty handler to avoid blocking read() syscall;
        // this allows cancellation of pipe in case shell process dies
        boost::asio::async_read_until(*stdout_pipe_,
            boost::asio::dynamic_buffer(result),
            end_marker, boost::asio::use_future([](auto&&...) { }))
            .wait();

        if (!shell_process_.running()) {
            is_running_->store(false);
            return CallError { CallError::Code::unknown, "shell ended prematurely" };
        }

        if (!endsWith(result, end_marker)) {
            return CallError { CallError::Code::unknown, "failed to get command output" };
        }
        // remove end marker
        result.resize(result.size() - end_marker.size());

        auto exit_code_pos = result.find_last_of(':');
        assert(exit_code_pos != std::string::npos);
        auto exit_code = toInteger<int>(result.substr(exit_code_pos + 1)).value_or(std::numeric_limits<int>::max());
        std::cout << "EXIT_CODE: " << exit_code << '\n';
        if (exit_code != 0) {
            return CallError { CallError::Code::unknown, std::to_string(exit_code) };
        }
        result.resize(exit_code_pos);

        std::cout << "DONE: " << command_line << '\n';
        std::cout << "RESULT: " << result << '\n';

        return result;
    }

    [[nodiscard]] CallResult<std::string> environmentVariable(const std::string& variable_name)
    {
        return callWithResult("printenv", { variable_name }).andThen([](auto&& result) {
            assert(!result.empty());
            assert(result.back() == '\n');
            result.pop_back();
            return result;
        });
    }

    [[nodiscard]] CallResult<void> environmentVariable(const std::string& variable_name, const std::string& new_value)
    {
        const std::string variable_assignment = variable_name + "='" + new_value + '\'';
        return callWithoutResult("export", { variable_assignment });
    }

    [[nodiscard]] bool isRunning()
    {
        return shell_process_.running();
    }

    [[nodiscard]] bool isRunning() const
    {
        return is_running_->load();
    }

private:
    static void runContextLoop(boost::asio::io_context& context)
    {
        while (true) {
            try {
                auto work = boost::asio::make_work_guard(context);
                context.run();
                break;
            } catch (const std::exception& exception) {
                assert(false);
            } catch (...) {
                assert(false);
            }
        }
    }

private:
    static constexpr std::chrono::milliseconds SHELL_SHUTDOWN_TIMEOUT { 10 };

private:
    // thread for running io_context
    std::thread thread_;
    std::unique_ptr<boost::asio::io_context> context_;
    boost::asio::writable_pipe stdin_pipe_;
    std::unique_ptr<boost::asio::readable_pipe> stdout_pipe_;
    boost::asio::readable_pipe stderr_pipe_;
    boost::process::v2::process_stdio io_;
    boost::process::v2::process shell_process_;
    std::unique_ptr<std::atomic<bool>> is_running_;

    // TODO: explore possibility to import/export variables and functions for each call
};
} // namespace ppplugin

#endif // PPPLUGIN_SHELL_SESSION_H
