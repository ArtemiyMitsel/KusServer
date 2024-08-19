#include "command_reader.hpp"

#include <iostream>
#include <string>

#include "core.hpp"

#include "command_handler.hpp"
#include "yield.hpp"

SINGLETON_DEFINITOR(core, CommandReader);

core::CommandReader::CommandReader() noexcept
    : Module("command_reader"), m_context(nullptr), m_out_func(stdoutOutput)
{
}

void
core::CommandReader::initialize() noexcept
{
    setVariable(int(ReaderVariables::READER_CIN), 1);
}

void
core::CommandReader::cinRead(std::string&& a_inp) noexcept
{
    /*
        TODO:
        https://en.cppreference.com/w/cpp/io/c/fopen
        while ((c = std::fgetc(fp)) != EOF) // Standard C I/O
        file reading loop std::putchar(c);
    */

    // if ((fseek(stdin, 0, SEEK_END), ftell(stdin)) > 0)
    // {
    //     std::getline(std::cin, a_inp);
    // }

    // auto tt = std::cin.rdbuf()->sgetc();
    // if (!feof(stdin) == 1)
    // {
        std::getline(std::cin, a_inp);
    // }
}

void
core::CommandReader::bufferRead(std::string&& a_inp, void** a_context) noexcept
{
    m_buffer_mutex.lock();
    a_inp = std::move(m_buffer);
    m_buffer.clear();
    *a_context = m_context;
    m_context  = nullptr;
    m_buffer_mutex.unlock();
}

void
core::CommandReader::run() noexcept
{
    std::string inp;

    while (Core::isRunning())
    {
        void* context = nullptr;
        inp.clear();

        for (auto i :
             {ReaderVariables::READER_CIN, ReaderVariables::READER_BUFFER})
        {
            if (!inp.empty())
            {
                break;
            }
            if (!getVariable(int(i)))
            {
                continue;
            }

            switch (i)
            {
                case ReaderVariables::READER_CIN:
                    cinRead(std::move(inp));
                    break;
                case ReaderVariables::READER_BUFFER:
                    bufferRead(std::move(inp), &context);
                    break;
                case ReaderVariables::READER_FILE:
                    break;
            }
        }

        if (!inp.empty())
        {
            CommandHandler::pushCommand(Command{inp, m_out_func, context});
        }

        // TODO: do we even need yield?
        Yield::large();
    }
}

void
core::CommandReader::stdoutOutput(const char* a_buff, void* a_context) noexcept
{
    (void)a_context;

    if (a_buff == nullptr)
    {
        LOG_ERROR("Empty command result buffer");
        return;
    }
    fprintf(stdout, "%s", a_buff);
    fflush(stdout);
}

void
core::CommandReader::variableSetup(
    core::VariableInfoArray&& a_set_array) noexcept
{
    a_set_array = getVariableHandlerRouter();
}

void
core::CommandReader::sendToBufferNonstatic(std::string&& a_data,
                                           void* a_context) noexcept
{
    while (true)
    {
        m_buffer_mutex.lock();
        if (m_buffer.empty())
        {
            m_buffer  = std::move(a_data);
            m_context = a_context;
            m_buffer_mutex.unlock();
            break;
        }
        m_buffer_mutex.unlock();
        Yield::small();
    }
}

void
core::CommandReader::bufferUsageNonstatic(bool a_is_used) noexcept
{
    if (a_is_used)
    {
        setVariable(int(ReaderVariables::READER_BUFFER), 1);
    }
    else
    {
        setVariable(int(ReaderVariables::READER_BUFFER), 0);
    }
}

void
core::CommandReader::setOutputCallbackNonstatic(
    core::Command::OutputCallback a_func) noexcept
{
    m_out_func = a_func;
}

std::optional<int>
core::CommandReader::getReaderStatus(core::Command& a_command,
                                     const str::string& a_variable,
                                     const str::string& a_value) noexcept
{
    std::optional<int> result;
    // TODO: map or constexpr
    // TODO: merge string name and enum defenition
    if (a_value == "off")
    {
        result = 0;
        COMMAND_RETURN_MSG(a_command, "Turn off reader status for '%s' stream",
                           a_variable);
    }
    else if (a_value == "on")
    {
        result = 1;
        COMMAND_RETURN_MSG(a_command, "Turn off reader status for '%s' stream",
                           a_variable);
    }
    else
    {
        COMMAND_RETURN_MSG(a_command,
                           "Undefined reader status '%s' for '%s' stream",
                           a_variable, a_value);
    }

    //     if (result.has_value() && result.value() == 1)
    //     {
    // if (a_variable ==
    //     }

    return result;
}
