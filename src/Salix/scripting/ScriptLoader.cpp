// Salix/scripting/ScriptLoader.cpp
#include "ScriptLoader.h"
#include <iostream>

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <dlfcn.h>
#endif

namespace Salix {

    bool ScriptLoader::load_script_library(const std::string& path) {
        std::cout << "ScriptLoader: Loading script library: " << path << std::endl;

    #ifdef _WIN32
        HMODULE handle = LoadLibraryA(path.c_str());
        if (!handle) {
            std::cerr << "❌ ScriptLoader: Failed to load library." << std::endl;
            return false;
        }
    #else
        void* handle = dlopen(path.c_str(), RTLD_NOW);
        if (!handle) {
            std::cerr << "❌ ScriptLoader: " << dlerror() << std::endl;
            return false;
        }
    #endif

        std::cout << "✅ ScriptLoader: Script DLL loaded successfully." << std::endl;
        return true;
    }

}
