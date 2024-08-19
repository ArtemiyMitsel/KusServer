#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "core/logging.hpp"

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);

    core::Logging::setLogLevel(core::Logging::LogLevel::WARNING);

    return RUN_ALL_TESTS();
}
