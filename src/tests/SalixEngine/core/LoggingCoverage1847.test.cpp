#include <doctest.h>
#include <Salix/core/Logging.h>
#include <sstream>
#include <string>
#include <algorithm>

namespace {
class Batch1847CerrRedirect {
public:
    explicit Batch1847CerrRedirect(std::stringstream& buffer)
        : original(std::cerr.rdbuf(buffer.rdbuf())) {}
    ~Batch1847CerrRedirect() { std::cerr.rdbuf(original); }
private:
    std::streambuf* original;
};
}

TEST_SUITE("Salix::core::LoggingCoverage1847") {
    TEST_CASE("unknown log level keeps an empty severity label without crashing") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        Salix::LogMessage(static_cast<Salix::LogLevel>(999), "mystery", "Unknown.cpp", 12);
        CHECK(output.str().find("[] mystery") != std::string::npos);
    }

    TEST_CASE("unknown log level does not opt into error source-location formatting") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        Salix::LogMessage(static_cast<Salix::LogLevel>(-1), "mystery", "ShouldNotAppear.cpp", 44);
        CHECK(output.str().find("ShouldNotAppear.cpp") == std::string::npos);
        CHECK(output.str().find("mystery") != std::string::npos);
    }

    TEST_CASE("warning preserves embedded newline payloads verbatim") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::WARNING, "first line\nsecond line", "Ignored.cpp", 3);
        CHECK(output.str().find("first line\nsecond line") != std::string::npos);
        const auto text = output.str();
        CHECK(std::count(text.begin(), text.end(), '\n') >= 2);
    }

    TEST_CASE("debug formatting preserves a negative diagnostic line number") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::DEBUG, "probe", "Debug.cpp", -7);
        CHECK(output.str().find("[DEBUG] [Debug.cpp:-7] probe") != std::string::npos);
    }

    TEST_CASE("error formatting preserves source paths containing spaces") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::ERROR, "failed", "C:/My Project/Scene.cpp", 81);
        CHECK(output.str().find("[C:/My Project/Scene.cpp:81]") != std::string::npos);
    }

    TEST_CASE("info payload containing bracket tokens is not reinterpreted") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        const std::string payload = "[user] [DEBUG] literal";
        Salix::LogMessage(Salix::LogLevel::INFO, payload, "Ignored.cpp", 1);
        CHECK(output.str().find(payload) != std::string::npos);
        CHECK(output.str().find("Ignored.cpp") == std::string::npos);
    }

    TEST_CASE("empty warning still emits a complete severity line") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::WARNING, "", "Ignored.cpp", 0);
        CHECK(output.str().find("[WARN] \n") != std::string::npos);
    }

    TEST_CASE("direct debug calls keep one newline per completed record") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::DEBUG, "one", "D.cpp", 1);
        Salix::LogMessage(Salix::LogLevel::DEBUG, "two", "D.cpp", 2);
        const auto text = output.str();
        CHECK(std::count(text.begin(), text.end(), '\n') == 2);
        CHECK(text.find("one") < text.find("two"));
    }

    TEST_CASE("macro and direct debug calls share the same severity spelling") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        LOG_DEBUG("macro");
        Salix::LogMessage(Salix::LogLevel::DEBUG, "direct", "Direct.cpp", 9);
        const auto text = output.str();
        const auto first = text.find("[DEBUG]");
        REQUIRE(first != std::string::npos);
        CHECK(text.find("[DEBUG]", first + 1) != std::string::npos);
    }

    TEST_CASE("timestamp prefix keeps HH MM SS and millisecond separators") {
        std::stringstream output; Batch1847CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::INFO, "shape", "Ignored.cpp", 0);
        const auto text = output.str();
        REQUIRE(text.size() > 14);
        CHECK(text[0] == '[');
        CHECK(text[3] == ':');
        CHECK(text[6] == ':');
        CHECK(text[9] == '.');
        CHECK(text[13] == ']');
    }
}
