#pragma once

//--------------------------------------------------------------------------------

#include "core/holy_trinity.hpp"
#include "core/module.hpp"
#include "core/variable_storage.hpp"

#include "string/parser.hpp"

#include "token.hpp"

//--------------------------------------------------------------------------------

namespace serv
{
class Server : public core::Module
{
public:
    HOLY_TRINITY_SINGLETON(Server);
    ~Server() override = default;

    void run() noexcept override;
    void variableSetup(
        core::VariableInfoArray&& a_var_set_array) noexcept override;

private:
    Server() noexcept;

#define ENUM_NAME ServerVariables
#define VAR_FILE  "server/server_vars.inc"
#include "core/var_unpack.inc"
};

} // namespace serv

//--------------------------------------------------------------------------------
