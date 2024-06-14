#include "generate_code.hpp"

#include "string_algorithms.hpp"

#include "file_data/file.hpp"
#include "file_data/path.hpp"

code::CodeGenerator::CodeGenerator() noexcept
{
    getTableData();
}

void
code::CodeGenerator::getTableData() noexcept
{
    std::unordered_map<str::String, str::String> typeMap = {
        {"character", "str::String"},
        {"text",      "str::String"},
        {"integer",   "int"        },
        {"smallint",  "int"        },
        {"real",      "float"      },
        {"boolean",   "char"       },
        {"date",      "str::String"},
        {"timestamp", "str::String"}
    };

    auto words       = file::File::getWords("config", "database.psql_db");
    str::String last = "";
    for (auto& i : words)
    {
        if (i[0] == "TABLE")
        {
            last       = i[1];
            auto& temp = mTables[last];
            temp.emplace_back();
            temp.back() = {"id", "int"};
        }
        else if (!mTables.empty())
        {
            auto& temp = mTables[last];
            // temp.emplace_back({i[0], typeMap[i[1]]});
            temp.emplace_back();
            temp.back() = {i[0], typeMap[i[1]]};
        }
    }
}

void
code::CodeGenerator::makeAll() noexcept
{
    // makeDatabaseStructure();
    // makePostHandler();
    // makeGetRouter();
}

void
code::CodeGenerator::makeDatabaseStructure() noexcept
{
    mFiles.emplace_back(code::CodeFile{"database_structures", "data"});
    code::CodeFile& file = mFiles.back();

    file.addHeaderToHpp("database_wrappers", false);
    file.addHeaderToHpp("vector", true);
    file.addHeaderToHpp("unordered_map", true);
    file.addHeaderToHpp("string", true);

    CodeFunction func("reset");
    func.setReturnType("void");
    std::unordered_map<str::String, str::String> initializationMap = {
        {"str::String", "" },
        {"int",         "0"},
        {"char",        "0"}
    };
    std::unordered_map<str::String, str::String> typeToDataType = {
        {"str::String", "data::Type::STRING"},
        {"int",         "data::Type::INT"   },
        {"char",        "data::Type::BOOL"  },
        {"float",       "data::Type::FLOAT" }
    };
    for (auto& i : mTables)
    {
        auto& temp = file.makeClass(i.first + "Base");
        temp.addParent("BaseDataStruct<" + std::to_string(i.second.size()) +
                       ">");
        temp.addUsing(i.first, "UpperDataStruct");

        str::String types;
        str::String names;
        str::String nameNum;
        str::String resetFunc;
        int cnt = 0;
        for (auto& j : i.second)
        {
            temp.addVariable(j[0], j[1], initializationMap[j[1]]);
            str::String strName = "\"" + j[0] + "\"";

            names += strName + ",";
            types += typeToDataType[j[1]] + ",";

            str::String num = std::to_string(cnt++);
            nameNum += "{" + strName + "," + num + "},";
            resetFunc += "ptrs[" + num + "] = (void*)(&" +
                         StringAlgorithms::normalizeName(j[0], false, true) +
                         ");";
        }

        types.pop_back();
        names.pop_back();
        nameNum.pop_back();

        func.setBody(resetFunc);
        temp.addFunction(func);

        temp.addStaticVariable("tableName", "str::String",
                               "\"" + i.first + "\"");
        temp.addStaticVariable("types", "std::vector<data::Type>", types);
        temp.addStaticVariable("columnNames", "std::vector<str::String>",
                               names);
        temp.addStaticVariable(
            "nameToNum", "std::unordered_map<str::String, uint8_t>", nameNum);
    }
}

void
code::CodeGenerator::makePostHandler() noexcept
{
    mFiles.emplace_back(code::CodeFile{"post_router", "post"});
    code::CodeFile& file = mFiles.back();

    file.addHeaderToHpp("post_handler", false);
    file.addHeaderToHpp("unordered_map", true);

    file.addHeaderToCpp("answer_handler", false);
    file.addHeaderToCpp("journal_handler", false);
    file.addHeaderToCpp("mark_handler", false);
    file.addHeaderToCpp("plan_handler", false);
    file.addHeaderToCpp("user_handler", false);

    auto& temp = file.makeClass();

    //     temp.addFuncRouterForDatabase(
    //         "processRouter", "post::PostHandler::process",
    //         makeRouterFunction({
    //   // clang-format off
    //             {"default",       "post::PostHandler::process"       },
    //             {"user",          "post::UserHandler::process"       },
    //             {"answer",        "post::AnswerHandler::process"     },
    //             {"journal_table", "post::JournalHandler::process"    },
    //             {"mark",          "post::MarkHandler::process"       }
    //   // clang-format on
    //     }));

    addRouter(
        temp, "processRouter",
        {
            // clang-format off
            {"default",       "post::PostHandler::postSubrouter<post::PostHandler, "       },
            {"user",          "post::PostHandler::postSubrouter<post::UserHandler, "       },
            {"answer",        "post::PostHandler::postSubrouter<post::AnswerHandler, "     },
            {"journal_table", "post::PostHandler::postSubrouter<post::JournalHandler, "    },
            {"mark",          "post::PostHandler::postSubrouter<post::MarkHandler, "       }
            // clang-format on
    });

    addRouter(temp, "dropRouter",
              {
                  {"default", "post::PostHandler::drop"}
    });

    addRouter(temp, "rawDataRouter",
              {
                  {"default",       "post::PostHandler::rawDataHandler"   },
                  {"journal_table", "post::JournalHandler::rawDataHandler"},
                  {"user",          "post::UserHandler::rawDataHandler"   },
                  {"plan",          "post::PlanHandler::rawDataHandler"   }
    });
}

void
code::CodeGenerator::makeGetRouter() noexcept
{
    mFiles.emplace_back(code::CodeFile{"get_router", "get"});
    code::CodeFile& file = mFiles.back();

    file.addHeaderToHpp("get_handler", false);
    file.addHeaderToHpp("unordered_map", true);

    file.addHeaderToCpp("get_handler", false);
    file.addHeaderToCpp("user_handler", false);
    file.addHeaderToCpp("form_handler", false);
    file.addHeaderToCpp("question_handler", false);

    auto& temp = file.makeClass();

    addRouter(temp, "basicRouter",
              {
                  {"default", "get::GetHandler::process" },
                  {"user",    "get::UserHandler::process"},
                  {"form",    "get::FormHandler::process"}
    });

    addRouter(temp, "dumpRouter",
              {
                  {"default", "get::GetHandler::dump"}
    });

    auto& func =
        temp.addRouterFunction("columnNamesRouter", "std::vector<str::String>",
                               makeRouterFunction(
                                   {
                                       {"default", ""}
    },
                                   "::columnNames")[1]);
    func.setReturnType("std::vector<str::String>&");
    func.setBody("auto it = mColumnNamesRouter.find(aName);"
                 "if (it == mColumnNamesRouter.end())"
                 "it = mColumnNamesRouter.find(\"dummy\");"
                 "return it->second;");
}

void
code::CodeGenerator::generate(CodeFile::FileType aType) const noexcept
{
    for (auto& i : mFiles)
    {
        i.generate(aType);
    }
}

void
code::CodeGenerator::addRouter(
    code::CodeClass& aClass,
    const str::String& aName,
    std::unordered_map<str::String, str::String> aNameMap) const noexcept
{
    for (auto& i : aNameMap) i.second.insert(0, 1, '&');
    auto temp = makeRouterFunction(aNameMap);
    aClass.addFuncRouterForDatabase(aName, temp[0], temp[1]);
}

std::array<str::String, 2>
code::CodeGenerator::makeRouterFunction(
    const std::unordered_map<str::String, str::String>& aNameMap,
    str::String aPostfix) const noexcept
{
    bool templateOpened =
        aNameMap.find("default")->second.find('<') != str::String::npos;
    if (aPostfix.empty())
    {
        aPostfix = ">";
    }
    else
    {
        templateOpened = true;
    }
    std::array<str::String, 2> result;

    for (auto& i : mTables)
    {
        bool flag = false;
        auto it   = aNameMap.find(i.first);
        if (it == aNameMap.end())
        {
            it   = aNameMap.find("default");
            flag = true;
        }

        result[1] += "{\"" + i.first + "\",";

        str::String temp = it->second;
        if (flag || templateOpened)
        {
            if (!templateOpened) temp.push_back('<');
            temp += "data::" + StringAlgorithms::normalizeName(i.first, true) +
                    aPostfix;
        }
        result[1] += temp + "},";

        if (i.first == "dummy")
        {
            result[0] += temp;
        }
    }
    result[1].pop_back();

    return result;
}
