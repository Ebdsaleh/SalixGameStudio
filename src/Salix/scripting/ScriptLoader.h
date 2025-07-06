// Salix/scripting/ScriptLoader
#pragma once
#include <Salix/core/Core.h>
#include <string>

namespace Salix {
    struct InitContext;
    class SALIX_API ScriptLoader {
    public:
        // Attempts to load the script DLL and call any dynamic initializers
        static bool load_script_library(const std::string& path, InitContext& context);
        void shutdown_plugin();
    };
}
