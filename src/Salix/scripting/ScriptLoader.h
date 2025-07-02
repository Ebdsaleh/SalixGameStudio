// Salix/scripting/ScriptLoader
#pragma once
#include <string>

namespace Salix {
    class ScriptLoader {
    public:
        // Attempts to load the script DLL and call any dynamic initializers
        static bool load_script_library(const std::string& path);
    };
}
