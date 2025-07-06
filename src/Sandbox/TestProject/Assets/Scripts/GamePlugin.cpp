// src/Sandbox/TestProject/Assets/Scripts/GamePlugin.cpp  
// inside TestProject.dll
#include <Salix/core/IPlugin.h>
#include <Salix/core/InitContext.h>

using namespace Salix;

class TestProjectPlugin : public IPlugin {
public:
    void on_register_types() override {
        // No need to register PlayerMovement here anymore.
    }

    void on_startup(InitContext& context) override {
        // Optional custom setup
    }

    void on_shutdown() override {
        // Optional teardown
    }
};

extern "C" __declspec(dllexport) IPlugin* create_plugin() {
    static TestProjectPlugin plugin;
    return &plugin;
}
