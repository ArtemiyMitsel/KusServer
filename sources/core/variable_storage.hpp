#pragma once

//--------------------------------------------------------------------------------

#include <atomic>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "command_register.hpp"
#include "holy_trinity.hpp"

//--------------------------------------------------------------------------------
/*
                                Class contruct

*/
//--------------------------------------------------------------------------------

namespace core
{

typedef std::optional<int> (*FPIntGlobVarToInt)(core::Command&,
                                                const str::string&,
                                                const str::string&);

struct VariableInfo
{
    std::string name;
    FPIntGlobVarToInt func;
};
using VariableInfoArray = std::vector<VariableInfo>;

class VariableStorage : public CommandRegister
{
public:
    HOLY_TRINITY_SINGLETON(VariableStorage);

    SINGL_RET_METHOD(int,
                     addVariableInfo,
                     (const VariableInfoArray& aVarSettings));
    SINGL_VOID_METHOD(reloadValuesFromFile, ());

    SINGL_VOID_METHOD(set, (int aNumber, int aValue));
    SINGL_RET_METHOD(int, get, (int aNumber));

private:
    struct Variable
    {
        std::atomic<int> value   = 0;
        FPIntGlobVarToInt parser = nullptr;

        Variable() noexcept = default;
        Variable(const Variable& other) noexcept;
    };

    std::vector<Variable> m_variables;
    std::unordered_map<std::string, int> m_name_to_var_dict;

    VariableStorage() noexcept;
    COMMAND_HANDLER_NONSTATIC(setCommandHandler);
    COMMAND_HANDLER_NONSTATIC(showVarCommandHandler);
};
} // namespace core

#define VARIABLE_HANDLER_BASE(body)                        \
    std::optional<int> body(core::Command& a_command,      \
                            const str::string& a_variable, \
                            const str::string& a_value) noexcept

#define VARIABLE_HANDLER_PTR   VARIABLE_HANDLER_BASE((*))
#define VARIABLE_HANDLER(name) static VARIABLE_HANDLER_BASE(name)

// #define VARIABLE_HANDLER(name)                                    \
//     static std::optional<int> name(core::Command& a_command,      \
//                                    const str::string& a_variable, \
//                                    const str::string& a_value) noexcept

//--------------------------------------------------------------------------------
