#include <Salix/core/IPlugin.h>
#include <Salix/core/InitContext.h>
#include <Salix/scripting/ScriptLoader.h>
#include <iostream>
#include <windows.h> // Required for LoadLibraryA

namespace Salix {

    static IPlugin* loaded_plugin = nullptr;

    using CreatePluginFn = IPlugin* (*)();

    bool ScriptLoader::load_script_library(const std::string& path, InitContext& context) {
        std::cout << "ScriptLoader: Loading script library: " << path << std::endl;

    #ifdef _WIN32
        HMODULE handle = LoadLibraryA(path.c_str());
        if (!handle) {
            std::cerr << "❌ ScriptLoader: Failed to load library." << std::endl;
            return false;
        }

        // Load plugin
        auto create_plugin = (CreatePluginFn)GetProcAddress(handle, "create_plugin");
        if (!create_plugin) {
            std::cerr << "❌ ScriptLoader: 'create_plugin' not found!" << std::endl;
            return false;
        }

    #else
        void* handle = dlopen(path.c_str(), RTLD_NOW);
        if (!handle) {
            std::cerr << "❌ ScriptLoader: " << dlerror() << std::endl;
            return false;
        }

        auto create_plugin = (CreatePluginFn)dlsym(handle, "create_plugin");
        if (!create_plugin) {
            std::cerr << "❌ ScriptLoader: 'create_plugin' not found!" << std::endl;
            return false;
        }
    #endif

        loaded_plugin = create_plugin();
        if (!loaded_plugin) {
            std::cerr << "❌ ScriptLoader: Plugin creation returned nullptr!" << std::endl;
            return false;
        }

        loaded_plugin->on_register_types();  // 🔥 Important: Cereal types
        loaded_plugin->on_startup(context);  // Optional: register user-side things

        std::cout << "✅ ScriptLoader: Script DLL loaded successfully." << std::endl;
        return true;
    }

    void ScriptLoader::shutdown_plugin() {
        if (loaded_plugin) {
            loaded_plugin->on_shutdown();
            loaded_plugin = nullptr;
        }
    }

} // namespace Salix
