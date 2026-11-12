// =================================================================================
// Filename:    src/Tests/SalixEngine/core/EngineInfo.test.cpp
// Description: Contains unit tests for EngineInfo the functions.
// ================================================================================= 

#include <doctest.h>
#include <Salix/core/EngineInfo.h>
#include <string>
#include <algorithm>

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

TEST_SUITE("Salix::core::EngineInfoBatch3247") {
 TEST_CASE("batch3247 engine version remains exact public compatibility string") { CHECK(Salix::EngineInfo::get_engine_version()=="0.1.0-Alpha"); }
 TEST_CASE("batch3247 semantic version delimiters occupy expected positions") { const auto& v=Salix::EngineInfo::get_engine_version(); REQUIRE(v.size()>=6); CHECK(v[1]=='.'); CHECK(v[3]=='.'); CHECK(v[5]=='-'); }
 TEST_CASE("batch3247 major component is zero") { const auto& v=Salix::EngineInfo::get_engine_version(); CHECK(v.substr(0,1)=="0"); }
 TEST_CASE("batch3247 minor component is one") { const auto& v=Salix::EngineInfo::get_engine_version(); CHECK(v.substr(2,1)=="1"); }
 TEST_CASE("batch3247 patch component is zero") { const auto& v=Salix::EngineInfo::get_engine_version(); CHECK(v.substr(4,1)=="0"); }
 TEST_CASE("batch3247 prerelease component is exactly Alpha") { const auto& v=Salix::EngineInfo::get_engine_version(); CHECK(v.substr(v.find('-')+1)=="Alpha"); }
 TEST_CASE("batch3247 repeated accessor returns identical c string storage") { const auto& a=Salix::EngineInfo::get_engine_version(); const auto& b=Salix::EngineInfo::get_engine_version(); CHECK(a.c_str()==b.c_str()); }
 TEST_CASE("batch3247 version metadata contains no embedded null characters") { const auto& v=Salix::EngineInfo::get_engine_version(); CHECK(v.find('\0')==std::string::npos); }
 TEST_CASE("batch3247 mutable copy cannot alter static version reference") { std::string copy=Salix::EngineInfo::get_engine_version(); copy.replace(0,5,"9.9.9"); CHECK(copy=="9.9.9-Alpha"); CHECK(Salix::EngineInfo::get_engine_version()=="0.1.0-Alpha"); }
 TEST_CASE("batch3247 repeated access preserves exact reference address") { const std::string* p=&Salix::EngineInfo::get_engine_version(); for(int i=0;i<16;++i) CHECK(&Salix::EngineInfo::get_engine_version()==p); }
}

TEST_SUITE("Salix::core::EngineInfoBatch4047") {
    TEST_CASE("batch4047 public version string has expected complete length") {
        CHECK(Salix::EngineInfo::get_engine_version().size() == 11);
    }

    TEST_CASE("batch4047 public version begins at major digit and ends at prerelease suffix") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        REQUIRE_FALSE(version.empty());
        CHECK(version.front() == '0');
        CHECK(version.back() == 'a');
    }

    TEST_CASE("batch4047 public version contains exactly one prerelease separator") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        CHECK(std::count(version.begin(), version.end(), '-') == 1);
    }

    TEST_CASE("batch4047 Alpha prerelease begins immediately after separator") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        const auto separator = version.find('-');
        REQUIRE(separator != std::string::npos);
        CHECK(version.substr(separator + 1) == "Alpha");
    }

    TEST_CASE("batch4047 semantic core is exactly zero one zero") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        const auto separator = version.find('-');
        REQUIRE(separator != std::string::npos);
        CHECK(version.substr(0, separator) == "0.1.0");
    }

    TEST_CASE("batch4047 version token contains no path separators") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        CHECK(version.find('/') == std::string::npos);
        CHECK(version.find('\\') == std::string::npos);
    }

    TEST_CASE("batch4047 repeated data pointers share the static version storage") {
        const auto& first = Salix::EngineInfo::get_engine_version();
        const auto& second = Salix::EngineInfo::get_engine_version();
        CHECK(first.data() == second.data());
    }

    TEST_CASE("batch4047 appending to a copied version cannot change static metadata") {
        std::string copy = Salix::EngineInfo::get_engine_version();
        copy += "-local";
        CHECK(copy == "0.1.0-Alpha-local");
        CHECK(Salix::EngineInfo::get_engine_version() == "0.1.0-Alpha");
    }

    TEST_CASE("batch4047 version contains three numeric semantic components") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        CHECK(version[0] == '0');
        CHECK(version[2] == '1');
        CHECK(version[4] == '0');
    }

    TEST_CASE("batch4047 repeated accessor remains byte identical across many reads") {
        const std::string snapshot = Salix::EngineInfo::get_engine_version();
        for (int i = 0; i < 32; ++i) {
            CHECK(Salix::EngineInfo::get_engine_version() == snapshot);
        }
    }
}

TEST_SUITE("Salix::core::EngineInfoBatch4847") {
    TEST_CASE("batch4847 accessor reference is exactly the exported version_id object") {
        CHECK(&Salix::EngineInfo::get_engine_version() == &Salix::EngineInfo::version_id);
    }

    TEST_CASE("batch4847 accessor data pointer is exactly exported version_id character storage") {
        CHECK(Salix::EngineInfo::get_engine_version().data() == Salix::EngineInfo::version_id.data());
    }

    TEST_CASE("batch4847 semantic core occupies exactly five characters before prerelease separator") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        const auto dash = version.find('-');
        REQUIRE(dash != std::string::npos);
        CHECK(dash == 5);
        CHECK(version.substr(0, dash).size() == 5);
    }

    TEST_CASE("batch4847 prerelease suffix occupies exactly five alphabetic characters") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        const auto dash = version.find('-');
        REQUIRE(dash != std::string::npos);
        CHECK(version.substr(dash + 1).size() == 5);
        CHECK(version.substr(dash + 1) == "Alpha");
    }

    TEST_CASE("batch4847 version contains exactly three numeric characters") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        const auto digits = std::count_if(version.begin(), version.end(), [](char ch) { return ch >= '0' && ch <= '9'; });
        CHECK(digits == 3);
    }

    TEST_CASE("batch4847 version contains exactly one uppercase ASCII character") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        const auto uppercase = std::count_if(version.begin(), version.end(), [](char ch) { return ch >= 'A' && ch <= 'Z'; });
        CHECK(uppercase == 1);
    }

    TEST_CASE("batch4847 version contains no path or URI delimiter characters") {
        const auto& version = Salix::EngineInfo::get_engine_version();
        CHECK(version.find('/') == std::string::npos);
        CHECK(version.find('\\') == std::string::npos);
        CHECK(version.find(':') == std::string::npos);
    }

    TEST_CASE("batch4847 exact compatibility token compares equal through std string compare") {
        CHECK(Salix::EngineInfo::get_engine_version().compare("0.1.0-Alpha") == 0);
    }

    TEST_CASE("batch4847 appending to a mutable copy leaves exported static data pointer unchanged") {
        const auto* before = Salix::EngineInfo::get_engine_version().data();
        std::string copy = Salix::EngineInfo::get_engine_version();
        copy += "+batch4847";
        CHECK(copy == "0.1.0-Alpha+batch4847");
        CHECK(Salix::EngineInfo::get_engine_version().data() == before);
        CHECK(Salix::EngineInfo::get_engine_version() == "0.1.0-Alpha");
    }

    TEST_CASE("batch4847 forward iteration reconstructs exact public compatibility token") {
        std::string reconstructed;
        for (const char ch : Salix::EngineInfo::get_engine_version()) reconstructed.push_back(ch);
        CHECK(reconstructed == "0.1.0-Alpha");
    }
}

TEST_SUITE("Salix::core::EngineInfoBatch5247") {
    TEST_CASE("batch5247 compatibility token remains exact") {
        CHECK(Salix::EngineInfo::get_engine_version() == "0.1.0-Alpha");
    }

    TEST_CASE("batch5247 compatibility token contains expected punctuation positions") {
        const auto& v = Salix::EngineInfo::get_engine_version();
        REQUIRE(v.size() == 11);
        CHECK(v[1] == '.');
        CHECK(v[3] == '.');
        CHECK(v[5] == '-');
    }

    TEST_CASE("batch5247 semantic core compares exactly against zero one zero") {
        const auto& v = Salix::EngineInfo::get_engine_version();
        CHECK(v.compare(0, 5, "0.1.0") == 0);
    }

    TEST_CASE("batch5247 prerelease suffix compares exactly against Alpha") {
        const auto& v = Salix::EngineInfo::get_engine_version();
        CHECK(v.compare(6, 5, "Alpha") == 0);
    }

    TEST_CASE("batch5247 version contains no ASCII whitespace") {
        const auto& v = Salix::EngineInfo::get_engine_version();
        CHECK(v.find_first_of(" \t\r\n") == std::string::npos);
    }

    TEST_CASE("batch5247 version contains no path or URI delimiters") {
        const auto& v = Salix::EngineInfo::get_engine_version();
        CHECK(v.find('/') == std::string::npos);
        CHECK(v.find('\\') == std::string::npos);
        CHECK(v.find(':') == std::string::npos);
    }

    TEST_CASE("batch5247 version contains exactly two dots and one dash") {
        const auto& v = Salix::EngineInfo::get_engine_version();
        CHECK(std::count(v.begin(), v.end(), '.') == 2);
        CHECK(std::count(v.begin(), v.end(), '-') == 1);
    }

    TEST_CASE("batch5247 version contains exactly three numeric characters") {
        const auto& v = Salix::EngineInfo::get_engine_version();
        const auto digits = std::count_if(v.begin(), v.end(), [](char ch) { return ch >= '0' && ch <= '9'; });
        CHECK(digits == 3);
    }

    TEST_CASE("batch5247 mutable erased copy never changes exported metadata") {
        std::string copy = Salix::EngineInfo::get_engine_version();
        copy.erase();
        CHECK(copy.empty());
        CHECK(Salix::EngineInfo::get_engine_version() == "0.1.0-Alpha");
    }

    TEST_CASE("batch5247 accessor and exported version object remain the same static string") {
        CHECK(&Salix::EngineInfo::get_engine_version() == &Salix::EngineInfo::version_id);
        CHECK(Salix::EngineInfo::get_engine_version().c_str() == Salix::EngineInfo::version_id.c_str());
    }
}

