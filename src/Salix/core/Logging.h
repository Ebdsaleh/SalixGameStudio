// Salix/core/Logging.h

#pragma once
#include <Salix/core/Core.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace Salix {
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR,
        DEBUG
    };

    // Base logging function
    void SALIX_API LogMessage(LogLevel level, const std::string& message, 
                             const char* file = "", int line = 0);
} // namespace Salix

// Macros that automatically capture file and line information
#define LOG_INFO(message)    do { std::ostringstream oss; oss << message; \
                                Salix::LogMessage(Salix::LogLevel::INFO, oss.str(), __FILE__, __LINE__); } while(0)
#define LOG_WARNING(message) do { std::ostringstream oss; oss << message; \
                                Salix::LogMessage(Salix::LogLevel::WARNING, oss.str(), __FILE__, __LINE__); } while(0)
#define LOG_ERROR(message)   do { std::ostringstream oss; oss << message; \
                                Salix::LogMessage(Salix::LogLevel::ERROR, oss.str(), __FILE__, __LINE__); } while(0)
#define LOG_DEBUG(message)   do { std::ostringstream oss; oss << message; \
                                Salix::LogMessage(Salix::LogLevel::DEBUG, oss.str(), __FILE__, __LINE__); } while(0)

// Assertion macro that logs before asserting
#define LOG_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::ostringstream oss; \
            oss << "Assertion failed: " << #condition << ". " << message; \
            Salix::LogMessage(Salix::LogLevel::ERROR, oss.str(), __FILE__, __LINE__); \
            assert(false && message); \
        } \
    } while(0)