// Salix/core/EngineInfo.cpp
#include <Salix/core/EngineInfo.h> // Include its own header for definition

namespace Salix {
    // --- Definition and Initialization of the static const std::string member ---
    // This happens ONLY ONCE in a single .cpp file.
    // This is where 'version_id' actually gets its memory and its value.
    const std::string EngineInfo::version_id = "0.1.0-Alpha"; // <-- DEFINITION & INITIALIZATION

    // --- Definition of the static method ---
    const std::string& EngineInfo::get_engine_version() {
        return version_id; // Returns a reference to the defined static string
    }
}