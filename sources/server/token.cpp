#include "token.hpp"

#include "core/logging.hpp"
#include "core/variable_storage.hpp"

std::optional<int>
serv::Token::getTokenStatus(core::Command& a_command,
                            const str::string& a_variable,
                            const str::string& a_value) noexcept
{
    std::optional<int> result;
    // TODO: map or constexpr
    // TODO: merge string name and enum defenition
    if (a_value == "turn_off")
    {
        result = int(Token::Status::TURN_OFF);
        COMMAND_RETURN_MSG(a_command, "Set token variable to 'turn_off'");
    }
    else if (a_value == "turn_on")
    {
        result = int(Token::Status::TURN_ON);
        COMMAND_RETURN_MSG(a_command, "Set token variable to 'turn_on'");
    }
    else if (a_value == "memory")
    {
        result = int(Token::Status::MEMORY);
        COMMAND_RETURN_MSG(a_command, "Set token variable to 'memory'");
    }
    else if (a_value == "print")
    {
        result = int(Token::Status::PRINT);
        COMMAND_RETURN_MSG(a_command, "Set token variable to 'print'");
    }
    else
    {
        COMMAND_RETURN_ERROR(a_command, "Can't parse token variable '%s'",
                             a_value);
    }
    return result;
}
