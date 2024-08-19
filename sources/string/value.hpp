
#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>

#include "core/types/pair.hpp"

namespace str
{
struct Value
{
    std::string str;

    operator int() const noexcept;
    operator float() const noexcept;
    operator bool() const noexcept;

    operator std::string() const noexcept;

    operator core::Pair<float>() const noexcept;

    enum class Type
    {
        Nun,
        Int,
        Float,
        Bool,
        String,
        PairInt,
        PairFloat
    };
    Type getType() const noexcept;
};
} // namespace str

#endif // !VALUE_HPP