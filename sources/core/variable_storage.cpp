#include "variable_storage.hpp"

#include <map>

#include "core/logging.hpp"

#include "file_system/file_read.hpp"
#include "file_system/path.hpp"

#include "string/parser.hpp"
#include "string/separators.hpp"

SINGLETON_DEFINITOR(core, VariableStorage);

//--------------------------------------------------------------------------------

core::VariableStorage::VariableStorage() noexcept
{
    registerCommand("var_set", setCommandHandler);
    registerCommand("var_show", showVarCommandHandler);
}

core::VariableStorage::Variable::Variable(const Variable& other) noexcept
{
    value  = int(other.value);
    parser = other.parser;
}

//--------------------------------------------------------------------------------

void
core::VariableStorage::setNonstatic(int a_number, int a_value) noexcept
{
    if (a_number >= m_variables.size())
    {
        LOG_ERROR("Wrong variable id")
    }
    else
    {
        m_variables[a_number].value = a_value;
    }
}

int
core::VariableStorage::getNonstatic(int a_number) noexcept
{
    int result = 0;
    if (a_number >= m_variables.size())
    {
        LOG_ERROR("Wrong variable id")
    }
    else
    {
        result = m_variables[a_number].value;
    }
    return result;
}

//--------------------------------------------------------------------------------

int
core::VariableStorage::addVariableInfoNonstatic(
    const VariableInfoArray& aVarSettings) noexcept
{
    int start_num = m_variables.size();
    m_variables.resize(start_num + aVarSettings.size());

    int num = start_num;
    for (auto& i : aVarSettings)
    {
        m_variables[num].parser    = i.func;
        m_name_to_var_dict[i.name] = num;
        ++num;
    }

    return start_num;
}

void
core::VariableStorage::reloadValuesFromFileNonstatic() noexcept
{
    // TODO: actual command
    core::Command dummy_result("dummy", nullptr);

    auto settings = fs::FileRead::getWordsMap(
        fs::ReadFromStoredFile("main_settings.cfg"), str::Separator::variable);
    for (auto& var : settings)
    {
        auto it = m_name_to_var_dict.find(var.first);
        if (it != m_name_to_var_dict.end())
        {
            int num = it->second;
            auto value_opt =
                m_variables[num].parser(dummy_result, var.first, var.second);
            if (value_opt.has_value())
            {
                m_variables[num].value = value_opt.value();
                LOG_INFO("Variable '%s' was set with value '%s'", var.first,
                         var.second);
            }
        }
        else
        {
            LOG_ERROR("No variable with name '%s' have been registered",
                      var.first);
        }
    }
}

//--------------------------------------------------------------------------------

void
core::VariableStorage::setCommandHandlerNonstatic(
    core::Command& aCommand) noexcept
{
    if (aCommand.variables.empty())
    {
        COMMAND_RETURN_ERROR(aCommand, "No variable values specified");
    }
    if (!aCommand.arguments.empty())
    {
        COMMAND_RETURN_ERROR(aCommand, "There is unrecognised word '%s'",
                             *aCommand.arguments.begin());
    }

    for (const auto& i : aCommand.variables)
    {
        // TODO: in separate function
        auto it = m_name_to_var_dict.find(i.first);
        if (it != m_name_to_var_dict.end())
        {
            int num  = it->second;
            auto val = m_variables[num].parser(aCommand, i.first, i.second);
            if (val.has_value())
            {
                // TODO: multiple output for multiple variables

                m_variables[num].value = val.value();
                // COMMAND_RETURN_MSG(
                //     aCommand,
                //     "Successfuly assigned value '%s' to variable '%s'",
                //     i.second, i.first);
            }
            // else
            // {
            //     COMMAND_RETURN_ERROR(aCommand,
            //                          "Unable to set value for '%s' variable:
            //                          " "corrupted variable value '%s'",
            //                          i.first, i.second);
            // }
        }
        else
        {
            COMMAND_RETURN_ERROR(
                aCommand,
                "Unable to set value for '%s' variable: no such variable",
                i.first);
        }
    }
}

void
core::VariableStorage::showVarCommandHandlerNonstatic(
    core::Command& aCommand) noexcept
{
    std::map<std::string_view, int> sorted_name_to_var_dict;
    sorted_name_to_var_dict.insert(m_name_to_var_dict.begin(),
                                   m_name_to_var_dict.end());

    std::string result;
    for (const auto& i : sorted_name_to_var_dict)
    {
        result += "\t'";
        result += i.first;
        result += "' = ";
        result += std::to_string(m_variables[i.second].value);
        result += "\n";
    }

    aCommand.setResultBufferSize(result.size() + 30);
    COMMAND_RETURN_MSG(aCommand, "\nVariable list:\n%sList end", result);
}
