// Salix/core/Logging.cpp
#include <Salix/core/Logging.h>
#include <iomanip>

namespace Salix {

void LogMessage(LogLevel level, const std::string& message, const char* file, int line) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    // Format time
    std::tm now_tm = *std::localtime(&now_time);
    std::ostringstream timestamp;
    timestamp << std::put_time(&now_tm, "%H:%M:%S") 
              << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    
    // Format log level
    const char* levelStr = "";
    switch (level) {
        case LogLevel::INFO:    levelStr = "INFO"; break;
        case LogLevel::WARNING: levelStr = "WARN"; break;
        case LogLevel::ERROR:   levelStr = "ERROR"; break;
        case LogLevel::DEBUG:   levelStr = "DEBUG"; break;
    }
    
    // Format output
    std::ostringstream output;
    output << "[" << timestamp.str() << "] "
           << "[" << levelStr << "] ";
    
    // Include file and line for errors and debug messages
    if (level == LogLevel::ERROR || level == LogLevel::DEBUG) {
        output << "[" << file << ":" << line << "] ";
    }
    
    output << message;
    
    // Output to console (you can modify this to output to file or other destinations)
    std::cerr << output.str() << std::endl;
    
    // TODO: Add your custom output handling here (file, IDE output, etc.)
}

} // namespace Salix