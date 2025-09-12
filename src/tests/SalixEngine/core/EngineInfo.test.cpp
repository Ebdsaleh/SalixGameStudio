// =================================================================================
// Filename:    src/Tests/SalixEngine/core/EngineInfo.test.cpp
// Description: Contains unit tests for EngineInfo the functions.
// ================================================================================= 

#include <doctest.h>
#include <Salix/core/EngineInfo.h>
#include <string>

TEST_SUITE("Salix::core::EngineInfo") {
    TEST_CASE("get_engine_version returns correct version string") {
        // ARRANGE
        // The expected version string is defined in EngineInfo.cpp
        const std::string expected_version = "0.1.0-Alpha";

        // ACT
        const std::string actual_version = Salix::EngineInfo::get_engine_version();

        // ASSERT
        CHECK(actual_version == expected_version);
    }
}