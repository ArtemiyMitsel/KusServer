#pragma once

//--------------------------------------------------------------------------------

#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

#include "kus_string.hpp"
#include "variable.hpp"

//--------------------------------------------------------------------------------

namespace str
{

class Parser
{
public:
    Parser() noexcept = delete;

    // static std::optional<Variable> makeVariable(
    //     const str::string& aStr) noexcept;
    // static std::vector<Variable> getVariablesFromFile(
    //     const str::string aFilename) noexcept;
    // static std::vector<Variable> getVariablesFromFile(
    //     const str::string& aFolderName, const str::string aFilename)
    //     noexcept;

    static std::vector<str::string> slice(
        const str::string& aStr,
        const str::string& aDelimiters,
        const str::string& aErase = "") noexcept;

    enum class Type
    {
        NUN,
        UPPER,
        LOWER
    };
    static void normalize(str::string& aStr, Type aType) noexcept;
    static str::string normalize(const str::string& aStr, Type aType) noexcept;
    // static char* normalize(char* a_str, Type aType) noexcept;
};

} // namespace str

//--------------------------------------------------------------------------------
