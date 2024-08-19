#pragma once

//--------------------------------------------------------------------------------

#include <mutex>

#include "string/parser.hpp"

#include "command.hpp"
#include "holy_trinity.hpp"
#include "module.hpp"

//--------------------------------------------------------------------------------

namespace core
{
class CommandReader : public core::Module
{
public:
    HOLY_TRINITY_SINGLETON(CommandReader);
    CommandReader() noexcept;
    void initialize() noexcept override;
    void run() noexcept override;

    void variableSetup(
        core::VariableInfoArray&& a_var_set_array) noexcept override;

    SINGL_VOID_METHOD(sendToBuffer, (std::string && a_data, void* a_context));
    SINGL_VOID_METHOD(bufferUsage, (bool a_is_used));
    SINGL_VOID_METHOD(setOutputCallback,
                      (core::Command::OutputCallback a_func = stdoutOutput));

private:
    static void stdoutOutput(const char* a_buff, void* a_context) noexcept;
    VARIABLE_HANDLER(getReaderStatus);

    std::string m_buffer;
    void* m_context;
    std::mutex m_buffer_mutex;

    core::Command::OutputCallback m_out_func;

    void cinRead(std::string&& a_inp) noexcept;
    void bufferRead(std::string&& a_inp, void** a_context) noexcept;

#define ENUM_NAME ReaderVariables
#define VAR_FILE  "command_reader_vars.inc"
#include "var_unpack.inc"
};
} // namespace core

//--------------------------------------------------------------------------------
