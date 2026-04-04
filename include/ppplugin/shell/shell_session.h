#ifndef PPPLUGIN_SHELL_SESSION_H
#define PPPLUGIN_SHELL_SESSION_H

#include "ppplugin/detail/string_utils.h"
#include "ppplugin/errors.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include <boost/algorithm/string/replace.hpp>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/process/v2/process.hpp>
#include <boost/uuid.hpp>

namespace ppplugin {
class ShellSession {
public:
    explicit ShellSession(const std::string& shell_binary)
        : context_ { std::make_unique<boost::asio::io_context>() }
        , stdin_pipe_ { *context_ }
        , stdout_pipe_ { *context_ }
        , stderr_pipe_ { *context_ }
        , io_ { stdin_pipe_, stdout_pipe_, stderr_pipe_ }
        , shell_process_ { *context_, shell_binary, {}, io_ }
        , wait_for_result_mutex_ { std::make_unique<std::mutex>() }
        , wait_for_result_ { std::make_unique<std::condition_variable>() }
    {
        thread_ = std::thread { [context = context_.get(), wait_for_result = wait_for_result_.get()]() { assert(context); runContextLoop(*context); wait_for_result->notify_all(); } };
    }
    ~ShellSession()
    {
        if (shell_process_.running()) {
            shell_process_.interrupt();
            std::this_thread::sleep_for(SHELL_SHUTDOWN_TIMEOUT);
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
    ShellSession(ShellSession&&) = default;
    ShellSession& operator=(const ShellSession&) = delete;
    ShellSession& operator=(ShellSession&&) = default;

    CallResult<void> callWithoutResult(const std::string& function_name, const std::vector<std::string>& arguments)
    {
        auto result = callWithResult(function_name, arguments);
        return result.andThen([]() { });
    }
    CallResult<std::string> callWithResult(const std::string& function_name, const std::vector<std::string>& arguments)
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
        constexpr auto BUFFER_SIZE = 1024;
        // for the abort condition to work, the end marker must fully fit into the buffer
        assert(end_marker.size() < BUFFER_SIZE);
        std::array<char, BUFFER_SIZE> buffer {};

        std::function<void(const boost::system::error_code& error, std::size_t bytes_transferred)> read_handler = [this, &result, &buffer, &end_marker, &read_handler](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (error.failed()) {
                // TODO? report error to user (error.what())?
                std::cout << "ERROR while reading: " << error.what() << '\n';
                return;
            }
            result.append(buffer.data(), bytes_transferred);
            std::cout << "PROGRESS... (" << result << ")\n";
            if (endsWith(result, end_marker)) {
                std::lock_guard lock { *wait_for_result_mutex_ };
                is_result_available_ = true;
                wait_for_result_->notify_all();
            } else {
                stdout_pipe_.async_read_some(boost::asio::buffer(buffer), read_handler);
            }
        };
        is_result_available_ = false;
        stdout_pipe_.async_read_some(boost::asio::buffer(buffer), read_handler);
        std::unique_lock lock { *wait_for_result_mutex_ };
        while (!wait_for_result_->wait_for(lock, std::chrono::milliseconds { 50 }, [this]() { return is_result_available_ || !shell_process_.running(); })) { }
        if (!shell_process_.running()) {
            return CallError { CallError::Code::unknown, std::to_string(shell_process_.exit_code()) };
        }
        assert(result.size() > end_marker.size());
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

    CallResult<std::string> environmentVariable(const std::string& variable_name)
    {
        // auto it = environment_variables_.find(variable_name);
        // if (it != environment_variables_.end()) {
        //     return it->second;
        // }
        // return CallError { CallError::Code::symbolNotFound };
        return callWithResult("printenv", { variable_name }).andThen([](auto&& result) {
            assert(!result.empty());
            assert(result.back() == '\n');
            result.pop_back();
            return result;
        });
    }

    CallResult<void> environmentVariable(const std::string& variable_name, const std::string& new_value)
    {
        std::string variable_assignment = variable_name + "='" + new_value + '\'';
        return callWithoutResult("export", { variable_assignment }).andThen([]() { });
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
            } catch (...) {
            }
        }
    }

private:
    static constexpr std::chrono::milliseconds SHELL_SHUTDOWN_TIMEOUT { 10 };

private:
    std::thread thread_;
    std::unique_ptr<boost::asio::io_context> context_;
    boost::asio::writable_pipe stdin_pipe_;
    boost::asio::readable_pipe stdout_pipe_;
    boost::asio::readable_pipe stderr_pipe_;
    boost::process::process_stdio io_;
    boost::process::process shell_process_;

    std::unique_ptr<std::mutex> wait_for_result_mutex_;
    std::unique_ptr<std::condition_variable> wait_for_result_;
    bool is_result_available_ { false };

    // TODO: explore possibility to import/export variables and functions for each call
};
} // namespace ppplugin

#endif // PPPLUGIN_SHELL_SESSION_H
