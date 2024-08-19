#pragma once

#include <cstdarg>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <string.h>

#include "string/kus_string.hpp"

#include "types/types.hpp"

#include "define_for_each.hpp"
#include "holy_trinity.hpp"
#include "logging_functors.hpp"

//------------------------------------------------------------------------------

namespace core
{

class Logging
{
public:
    HOLY_TRINITY_SINGLETON(Logging);

    enum class LogLevel
    {
        NUN,
        INFO,
        CMD,
        WARNING,
        ERROR
    };

    enum class OutputType
    {
        NUN,
        STDOUT,
        STDERR,
        FILE
    };

    static void setLogLevel(LogLevel aLogLevel) noexcept;
    SINGL_VOID_METHOD(setOutputType,
                      (OutputType aOutputType, const str::string& aFileName));

    void writeLog(LogLevel a_level,
                  char* tee,
                  size_t tee_size,
                  const char* format,
                  ...)
    {
        if (a_level < mLogLevel)
        {
            return;
        }

        if (tee)
        {
            va_list args_for_str;
            va_start(args_for_str, format);
            vsnprintf(tee, tee_size, format, args_for_str);
            va_end(args_for_str);
        }

        const std::lock_guard lock(m_write_mutex);

        va_list args;
        va_start(args, format);
        vfprintf(mStream, format, args);
        va_end(args);
        fflush(mStream);
    }

private:
    bool mIsFileOutput;
    LogLevel mLogLevel;
    FILE* mStream;
    std::mutex m_write_mutex;

    Logging() noexcept;
    ~Logging();
    void clear() noexcept;
};

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define WRITE_LOG_MSG(level, buff_ptr, buff_size, format, type, ...)       \
    {                                                                      \
        core::Logging::getInstance().writeLog(                             \
            level, buff_ptr, buff_size,                                    \
            "[%-5s] %-25s| %-30s[%-4d] " format ".\n", type, __FILENAME__, \
            __func__, __LINE__,                                            \
            FOR_EACH(core::LoggingFunctors::convert, __VA_ARGS__) "");     \
    }

#define WRITE_LOG_MSG_BASE(level, type, format, ...) \
    WRITE_LOG_MSG(level, NULL, 0, format, #type, __VA_ARGS__)

#define LOG_INFO(...) \
    WRITE_LOG_MSG_BASE(core::Logging::LogLevel::INFO, INFO, __VA_ARGS__)
#define LOG_WARNING(...) \
    WRITE_LOG_MSG_BASE(core::Logging::LogLevel::WARNING, WARN, __VA_ARGS__)
#define LOG_ERROR(...) \
    WRITE_LOG_MSG_BASE(core::Logging::LogLevel::ERROR, ERROR, __VA_ARGS__)

} // namespace core
