// =================================================================================
// Filename:    src/Tests/SalixEngine/core/Logging.test.cpp
// Description: Contains unit tests for the functions of the Logging class.
// ================================================================================= 

#include <doctest.h>
#include <Salix/core/Logging.h>
#include <iostream>
#include <sstream>
#include <string>

// This helper struct uses RAII (Resource Acquisition Is Initialization) to manage
// the redirection of std::cerr. When an object of this type is created, it
// redirects std::cerr. When it's destroyed (at the end of the scope), it
// automatically restores it.
struct CerrRedirect {
    CerrRedirect(std::stringstream& local_buffer) {
        // 1. Save the original buffer of std::cerr
        original_buffer = std::cerr.rdbuf();
        // 2. Redirect std::cerr to our local stringstream buffer
        std::cerr.rdbuf(local_buffer.rdbuf());
    }

    ~CerrRedirect() {
        // 3. Restore the original buffer when the object is destroyed
        std::cerr.rdbuf(original_buffer);
    }

private:
    std::streambuf* original_buffer;
};

TEST_SUITE("Salix::core::Logging") {
    TEST_CASE("LogMessage formats INFO and WARNING logs correctly") {
        // ARRANGE
        std::stringstream captured_output;
        // Redirect std::cerr to the stringstream.
        CerrRedirect CerrRedirect(captured_output);
        SUBCASE("for INFO") {
            // ARRANGE
            const std::string info_message = "This is an info-level message.";

            // ACT
            LOG_INFO(info_message);

            // ASSERT
            std::string log_string = captured_output.str();

            // Cannot check the exact timestamp, but can verify the key parts.
            CHECK(log_string.find("[INFO]") != std::string::npos);
            CHECK(log_string.find(info_message) != std::string::npos);
            // For INFO/WARN, the file and line number should not be present.
            CHECK(log_string.find(__FILE__) == std::string::npos);
        }
        SUBCASE("for WARN") {
            // ARRANGE
            const std::string warn_message = "Warning: This may cause issues.";

            // ACT
            LOG_WARNING(warn_message);

            // ASSERT
            std::string log_string = captured_output.str();

            CHECK(log_string.find("[WARN]") != std::string::npos);
            CHECK(log_string.find(warn_message) != std::string::npos);
            CHECK(log_string.find(__FILE__) == std::string::npos);
        }
    }
    TEST_CASE("LogMessage formats ERROR and DEBUG logs with file and line") {
        // This test covers the conditional branch in your LogMessage function. 
        // ARRANGE
        std::stringstream captured_output;
        CerrRedirect redirect(captured_output);
        SUBCASE("for ERROR") {
            // ARRANGE
            const std::string error_message = "An error occurred.";

            // ACT
            LOG_ERROR(error_message);

            // ASSERT
            std::string log_string = captured_output.str();
            
            CHECK(log_string.find("[ERROR]") != std::string::npos);
            CHECK(log_string.find(error_message) != std::string::npos);
            // For ERROR/DEBUG, the file and line number SHOULD be present.
            CHECK(log_string.find("Logging.test.cpp") != std::string::npos);
        }
        SUBCASE("for DEBUG") {
            // ARRANGE
            const std::string debug_message = "this is debug message.";

            // ACT
            LOG_DEBUG(debug_message);

            // ASSERT
            std::string log_string = captured_output.str();

            CHECK(log_string.find("[DEBUG]") != std::string::npos);
            CHECK(log_string.find(debug_message) != std::string::npos);
            CHECK(log_string.find("Logging.test.cpp") != std::string::npos);
        }
    }
}

// Diagnostic severity, source-location rules and macro evaluation contracts.
#include <algorithm>
TEST_SUITE("Salix::core::LoggingExtended") {
    TEST_CASE("logging info output includes message and excludes call-site filename") {
        std::stringstream output;
        CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::INFO, "editor ready", "PrivateTest.cpp", 123);
        CHECK(output.str().find("[INFO] editor ready") != std::string::npos);
        CHECK(output.str().find("PrivateTest.cpp") == std::string::npos);
    }

    TEST_CASE("logging warning output uses WARN label and omits source location") {
        std::stringstream output;
        CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::WARNING, "cache is cold", "PrivateTest.cpp", 18);
        CHECK(output.str().find("[WARN] cache is cold") != std::string::npos);
        CHECK(output.str().find("PrivateTest.cpp") == std::string::npos);
    }

    TEST_CASE("logging error output includes explicit filename and line number") {
        std::stringstream output;
        CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::ERROR, "scene failed", "SceneLoader.cpp", 47);
        CHECK(output.str().find("[ERROR] [SceneLoader.cpp:47] scene failed") != std::string::npos);
    }

    TEST_CASE("logging debug output includes supplied nested source path") {
        std::stringstream output;
        CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::DEBUG, "heartbeat", "src/Salix/core/Engine.cpp", 225);
        CHECK(output.str().find("[DEBUG] [src/Salix/core/Engine.cpp:225] heartbeat") != std::string::npos);
    }

    TEST_CASE("logging empty messages preserve level and location without crashing") {
        std::stringstream output;
        CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::ERROR, "", "Empty.cpp", 0);
        CHECK(output.str().find("[ERROR] [Empty.cpp:0] ") != std::string::npos);
        CHECK_FALSE(output.str().empty());
    }

    TEST_CASE("logging each log call writes a separate completed newline") {
        std::stringstream output;
        CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::INFO, "first");
        Salix::LogMessage(Salix::LogLevel::INFO, "second");
        const auto logs = output.str();
        CHECK(logs.find("first\n") != std::string::npos);
        CHECK(logs.find("second\n") != std::string::npos);
        CHECK(std::count(logs.begin(), logs.end(), '\n') == 2);
    }

    TEST_CASE("logging macro expression accepts stream manipulators and formatted numbers") {
        std::stringstream output;
        CerrRedirect redirect(output);
        LOG_INFO("value=" << std::hex << 255);
        CHECK(output.str().find("value=ff") != std::string::npos);
        CHECK(output.str().find("[INFO]") != std::string::npos);
    }

    TEST_CASE("logging macro debug preserves filename and evaluates one expression once") {
        std::stringstream output;
        CerrRedirect redirect(output);
        int evaluations = 0;
        LOG_DEBUG("visit " << ++evaluations);
        CHECK(evaluations == 1);
        CHECK(output.str().find("visit 1") != std::string::npos);
        CHECK(output.str().find("Logging.test.cpp") != std::string::npos);
    }

    TEST_CASE("logging assert macro with true condition does not log or evaluate message") {
        std::stringstream output;
        CerrRedirect redirect(output);
        int message_evaluations = 0;
        LOG_ASSERT(true, (++message_evaluations, "not needed"));
        CHECK(message_evaluations == 0);
        CHECK(output.str().empty());
    }

    TEST_CASE("logging log sequence preserves severity order and distinct payloads") {
        std::stringstream output;
        CerrRedirect redirect(output);
        Salix::LogMessage(Salix::LogLevel::INFO, "init");
        Salix::LogMessage(Salix::LogLevel::WARNING, "warning");
        Salix::LogMessage(Salix::LogLevel::ERROR, "failure", "Engine.cpp", 66);
        const std::string data = output.str();
        const auto first = data.find("[INFO] init");
        const auto middle = data.find("[WARN] warning");
        const auto last = data.find("[ERROR] [Engine.cpp:66] failure");
        REQUIRE(first != std::string::npos);
        REQUIRE(middle != std::string::npos);
        REQUIRE(last != std::string::npos);
        CHECK(first < middle);
        CHECK(middle < last);
    }
}

// Additional diagnostic and deferred-delivery contracts.
#include <algorithm>
TEST_SUITE("Salix::core::LoggingBatch647") {
    TEST_CASE("INFO macro evaluates a stateful message expression exactly once") {
        std::stringstream captured;CerrRedirect redirect(captured);
        int evaluations=0;LOG_INFO("render pass "<<++evaluations);
        CHECK(evaluations==1);
        CHECK(captured.str().find("[INFO] render pass 1")!=std::string::npos);
    }

    TEST_CASE("WARNING macro evaluates a stateful message expression exactly once") {
        std::stringstream captured;CerrRedirect redirect(captured);
        int evaluations=0;LOG_WARNING("retry "<<++evaluations);
        CHECK(evaluations==1);
        CHECK(captured.str().find("[WARN] retry 1")!=std::string::npos);
    }

    TEST_CASE("ERROR macro evaluates a stateful message expression exactly once") {
        std::stringstream captured;CerrRedirect redirect(captured);
        int evaluations=0;LOG_ERROR("failure "<<++evaluations);
        CHECK(evaluations==1);
        CHECK(captured.str().find("[ERROR]")!=std::string::npos);
        CHECK(captured.str().find("failure 1")!=std::string::npos);
    }

    TEST_CASE("DEBUG macro evaluates stateful expression only once") {
        std::stringstream captured;CerrRedirect redirect(captured);
        int evaluations=0;LOG_DEBUG("iteration "<<++evaluations);
        CHECK(evaluations==1);
        CHECK(captured.str().find("iteration 1")!=std::string::npos);
    }

    TEST_CASE("INFO log does not add source location even with a supplied line number") {
        std::stringstream captured;CerrRedirect redirect(captured);
        Salix::LogMessage(Salix::LogLevel::INFO,"ready","PrivateSource.cpp",99);
        const auto output=captured.str();
        CHECK(output.find("[INFO] ready")!=std::string::npos);
        CHECK(output.find("PrivateSource.cpp")==std::string::npos);
    }

    TEST_CASE("DEBUG log preserves an explicit source filename and exact line number") {
        std::stringstream captured;CerrRedirect redirect(captured);
        Salix::LogMessage(Salix::LogLevel::DEBUG,"configured","Camera.cpp",314);
        CHECK(captured.str().find("[DEBUG] [Camera.cpp:314] configured")!=std::string::npos);
    }

    TEST_CASE("logging two sequential events writes two distinct completed lines") {
        std::stringstream captured;CerrRedirect redirect(captured);
        Salix::LogMessage(Salix::LogLevel::INFO,"first");
        Salix::LogMessage(Salix::LogLevel::INFO,"second");
        const std::string output=captured.str();
        CHECK(std::count(output.begin(),output.end(),'\n')==2);
        CHECK(output.find("first\n")<output.find("second\n"));
    }

    TEST_CASE("stream formatting from macro preserves hexadecimal and decimal independently") {
        std::stringstream captured;CerrRedirect redirect(captured);
        LOG_INFO("color="<<std::hex<<255);
        LOG_INFO("opacity="<<std::dec<<100);
        CHECK(captured.str().find("color=ff")!=std::string::npos);
        CHECK(captured.str().find("opacity=100")!=std::string::npos);
    }

    TEST_CASE("a satisfied logging assertion neither emits output nor evaluates its message") {
        std::stringstream captured;CerrRedirect redirect(captured);
        int evaluated=0;
        LOG_ASSERT(2+2==4,(++evaluated,"unreachable"));
        CHECK(evaluated==0);
        CHECK(captured.str().empty());
    }

    TEST_CASE("a message containing punctuation retains its payload verbatim") {
        std::stringstream captured;CerrRedirect redirect(captured);
        const std::string payload="Realm=Demo; Camera:0, Player@(-3,4)!";
        Salix::LogMessage(Salix::LogLevel::WARNING,payload);
        CHECK(captured.str().find(payload)!=std::string::npos);
        CHECK(captured.str().find("[WARN]")!=std::string::npos);
    }
}

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

TEST_SUITE("Salix::core::LoggingBatch1847") {
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
