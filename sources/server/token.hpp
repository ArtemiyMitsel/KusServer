#pragma once

//--------------------------------------------------------------------------------

#include <optional>
#include <string>

#include "core/command.hpp"
#include "core/holy_trinity.hpp"

//--------------------------------------------------------------------------------

namespace serv
{

class Token
{
public:
    enum Status
    {
        NUN,
        TURN_OFF,
        TURN_ON,
        MEMORY,
        PRINT
    };

    static std::optional<int> getTokenStatus(
        core::Command& a_command,
        const str::string& a_variable,
        const str::string& a_value) noexcept;
};

} // namespace serv

//--------------------------------------------------------------------------------
