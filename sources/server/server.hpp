#pragma once

//--------------------------------------------------------------------------------

#include "domain/holy_trinity.hpp"

#include "core/module.hpp"

//--------------------------------------------------------------------------------

namespace serv
{
class Server
{
public:
    static void start() noexcept;

private:
    int mVariableOffset;

    HOLY_TRINITY_NOCOPY(Server);
    Server() noexcept;
    Server& getInstace() noexcept;

    static void registerVariables() noexcept;
};
} // namespace serv

//--------------------------------------------------------------------------------
