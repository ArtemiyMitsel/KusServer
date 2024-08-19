#pragma once

//--------------------------------------------------------------------------------

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "string/kus_string.hpp"

#include "holy_trinity.hpp"
#include "logging.hpp"

//--------------------------------------------------------------------------------

#define LOG_COMMAND_ERROR

namespace core
{
struct Command
{
    using OutputCallback = void (*)(const char*, void*) noexcept;

    HOLY_TRINITY_NOCOPY(Command);

    str::string value;
    std::vector<str::string> arguments;
    std::unordered_map<str::string, str::string> variables;

    Command(const str::string& aStr,
            OutputCallback a_outp_call,
            void* a_context = nullptr) noexcept;

    void setResultBufferSize(size_t a_result_buffer_size) noexcept;
    size_t getResultBufferSize() noexcept;
    // void returnMsg() noexcept;
    // void returnError() noexcept;

    char* getResultBuffer() noexcept;
    void callOutputFunc() noexcept;

    bool argumentSizeCheck(int a_size) noexcept;

    bool getArgumentAsNumber(int& a_result,
                             int a_arg_num,
                             int a_max_val) noexcept;

private:
    void* m_context;
    OutputCallback m_output_callback;
    size_t m_result_buffer_size;
    std::unique_ptr<char[]> m_result_buffer;
};
} // namespace core

#define WRITE_LOG_MSG_CMD(cmd, type, format, ...)                      \
    WRITE_LOG_MSG(core::Logging::LogLevel::CMD, cmd.getResultBuffer(), \
                  cmd.getResultBufferSize(), format, #type, __VA_ARGS__)

#define COMMAND_RETURN_MSG(cmd, ...) WRITE_LOG_MSG_CMD(cmd, CMD, __VA_ARGS__)
#define COMMAND_RETURN_ERROR(cmd, ...) \
    WRITE_LOG_MSG_CMD(cmd, ERROR, __VA_ARGS__)

#define COMMAND_HANDLER(name) static void name(Command& aCommand) noexept;
#define COMMAND_HANDLER_NONSTATIC(name) \
    SINGL_PRIV_VOID_METHOD(name, (core::Command & aCommand))

#define ARGUMENT_SIZE_CHECK(num)         \
    if (aCommand.argumentSizeCheck(num)) \
    {                                    \
        return;                          \
    }

// #define GET_ARG_AS_NUM(name, num)                             \
//     ARGUMENT_SIZE_CHECK(num + 1);                             \
//     auto optional_##name = aCommand.getArgumentAsNumber(num); \
//     if (!optional_##name.has_value())                         \
//     {                                                         \
//         return;                                               \
//     }                                                         \
//     int name = optional_##name.value();

//--------------------------------------------------------------------------------
