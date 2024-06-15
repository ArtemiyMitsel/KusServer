#pragma once

//--------------------------------------------------------------------------------

#include <string.h>
#include <string_view>

#include "core/holy_trinity.hpp"

//--------------------------------------------------------------------------------

namespace kstd
{
template <const char* Name, size_t Size>
struct CharBuffer
{
public:
    HOLY_TRINITY_SINGLE(CharBuffer);

    CharBuffer() noexcept : m_last_element_num(0)
    {
    }

    std::string_view add(const char* a_ptr) noexcept
    {
        // TODO: optimise?
        int ret = sprintf_s(m_storage, Size - m_last_element_num, a_ptr);

        std::string_view result;
        if (ret < 0)
        {
            DLOG_ERROR("Not enough space for", Name, "buffer");
        }
        else
        {
            result(m_storage + m_last_element_num, ret);
            m_last_element_num += ret;
        }
        return result;
    }

private:
    char m_storage[Size];
    size_t m_last_element_num;
};
} // namespace str

//--------------------------------------------------------------------------------