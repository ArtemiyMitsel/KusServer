#include "command.hpp"

#include "string/parser.hpp"

core::Command::Command(const str::string& aStr,
                       OutputCallback a_outp_call,
                       void* a_context) noexcept
    : m_result_buffer_size(200),
      m_output_callback(a_outp_call),
      m_context(a_context)
{
    auto args = str::Parser::slice(aStr, "; \n\t");
    value     = std::move(args[0]);
    args.erase(args.begin());

    for (auto& i : args)
    {
        // TODO: empty line slice bug
        auto temp = str::Parser::slice(i, "=");
        if (temp.size() == 1)
        {
            arguments.emplace_back(std::move(temp[0]));
        }
        else
        {
            variables[std::move(temp[0])] = std::move(temp[1]);
        }
    }
}

void
core::Command::setResultBufferSize(size_t a_result_buffer_size) noexcept
{
    // TODO: automate debug line info size calc
    m_result_buffer_size =
        a_result_buffer_size + 100; // debug info (file, func, line)
    m_result_buffer.reset(nullptr);
    getResultBuffer();
}

size_t
core::Command::getResultBufferSize() noexcept
{
    return m_result_buffer_size;
}

// void
// core::Command::returnMsg() noexcept
// {
//     prepareResultBuffer();

// }

// void
// core::Command::returnError() noexcept
// {
//     prepareResultBuffer();
//     WRITE_LOG_MSG
// }

// void
// core::Command::prepareResultBuffer() noexcept
// {
//     if (!m_result_buffer)
//     {
//         m_result_buffer = std::make_unique<char[]>(m_result_buffer_size);
//     }
// }

char*
core::Command::getResultBuffer() noexcept
{
    if (!m_result_buffer)
    {
        m_result_buffer = std::make_unique<char[]>(m_result_buffer_size);
    }
    return m_result_buffer.get();
}

void
core::Command::callOutputFunc() noexcept
{
    if (m_output_callback == nullptr)
    {
        LOG_ERROR("Command output callback dosn't set");
        return;
    }
    if (strlen(m_result_buffer.get()) >= getResultBufferSize())
    {
        int expected = getResultBufferSize();
        int received = strlen(m_result_buffer.get());

        // m_result_buffer.get()[0] = 0;
        // m_result_buffer.reset(nullptr);

        // setResultBufferSize(720);

        COMMAND_RETURN_ERROR((*this),
                             "ERROR: COMMAND OUPUT TOO LARGE! "
                             "Expected len no more then '%d', received '%d'",
                             expected, received);
    }
    m_output_callback(m_result_buffer.get(), m_context);
}

bool
core::Command::argumentSizeCheck(int a_size) noexcept
{
    bool result = true;
    if (arguments.size() <= a_size)
    {
        COMMAND_RETURN_ERROR(
            (*this),
            "Incorrect number of arguments. Expected number of arguments"
            "is at least %d, actual argument count %lu",
            a_size, arguments.size());
        result = false;
    }
    return result;
}

bool
core::Command::getArgumentAsNumber(int& a_result,
                                   int a_arg_num,
                                   int a_max_val) noexcept
{
    bool ret = argumentSizeCheck(a_arg_num);

    if (ret)
    {
        try
        {
            a_result = std::stoi(arguments[a_arg_num]);
        }
        catch (const std::exception& e)
        {
            COMMAND_RETURN_ERROR((*this),
                                 "Unable to parse '%s' number, exception '%s'",
                                 arguments[a_arg_num].c_str(), e.what());
            ret = false;
        }

        if (ret)
        {
            if (a_result <= a_max_val)
            {
                COMMAND_RETURN_ERROR((*this),
                                     "Too large argument number."
                                     "Expected no more than %lu, got %lu",
                                     a_max_val, a_result);
                ret = false;
            }
        }
    }

    return ret;
}
