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