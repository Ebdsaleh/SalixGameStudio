// =================================================================================
// Filename:    src/Tests/SalixEngine/core/EngineInfo.test.cpp
// Description: Contains unit tests for EngineInfo the functions.
// ================================================================================= 

#include <doctest.h>
#include <Salix/core/EngineInfo.h>
#include <string>\n#include <algorithm>

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

// Additional stable contracts for the 847+ coverage batch.
TEST_SUITE("Salix::core::EngineInfoBatch847") {
    TEST_CASE("engine version accessor returns the same static string object on every call") {
        const auto& a=Salix::EngineInfo::get_engine_version(); const auto& b=Salix::EngineInfo::get_engine_version(); CHECK(&a==&b);
    }

    TEST_CASE("engine version metadata is nonempty") {
        CHECK_FALSE(Salix::EngineInfo::get_engine_version().empty());
    }

    TEST_CASE("engine version begins with a numeric major component") {
        const auto& v=Salix::EngineInfo::get_engine_version(); REQUIRE_FALSE(v.empty()); CHECK(v.front()>='0'); CHECK(v.front()<='9');
    }

    TEST_CASE("engine version contains two numeric component separators") {
        const auto& v=Salix::EngineInfo::get_engine_version(); CHECK(std::count(v.begin(),v.end(),'.')==2);
    }

    TEST_CASE("engine version contains an explicit prerelease separator") {
        CHECK(Salix::EngineInfo::get_engine_version().find('-')!=std::string::npos);
    }

    TEST_CASE("engine version has no whitespace characters") {
        const auto& v=Salix::EngineInfo::get_engine_version(); CHECK(v.find(' ')==std::string::npos); CHECK(v.find('\t')==std::string::npos); CHECK(v.find('\n')==std::string::npos);
    }

    TEST_CASE("engine version preserves alpha prerelease spelling") {
        const auto& v=Salix::EngineInfo::get_engine_version(); CHECK(v.find("Alpha")!=std::string::npos);
    }

    TEST_CASE("copying engine version text cannot mutate static metadata") {
        std::string copy=Salix::EngineInfo::get_engine_version(); copy[0]='9'; CHECK(Salix::EngineInfo::get_engine_version()=="0.1.0-Alpha"); CHECK(copy!="0.1.0-Alpha");
    }

    TEST_CASE("engine version length remains stable across repeated access") {
        const auto size=Salix::EngineInfo::get_engine_version().size(); for(int i=0;i<8;++i) CHECK(Salix::EngineInfo::get_engine_version().size()==size);
    }

    TEST_CASE("engine version semantic components retain expected major minor and patch prefix") {
        const auto& v=Salix::EngineInfo::get_engine_version(); CHECK(v.rfind("0.1.0",0)==0); CHECK(v.substr(0,5)=="0.1.0");
    }
}
