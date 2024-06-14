#ifndef FILE_ROUTER_HPP
#define FILE_ROUTER_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "database/raw_data.hpp"

namespace core
{

class FileRouter
{
public:
    static data::RawDataArray process(const str::String& aFileName) noexcept;

private:
    static data::RawDataArray dmpParser(const str::String& aFileName) noexcept;
    static data::RawDataArray dataParser(const str::String& aFileName) noexcept;
    static data::RawDataArray csvParser(const str::String& aFileName) noexcept;

    static std::unordered_map<str::String, decltype(&dmpParser)> mRouter;

    static bool isSeparator(char c) noexcept;
    static bool isDMPSeparator(char c) noexcept;
    static bool isCSVSeparator(char c) noexcept;
};

} // namespace core

#endif // !FILE_ROUTER_HPP
