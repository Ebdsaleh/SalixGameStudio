#include <Salix/core/IPlugin.h>
#include <Salix/core/InitContext.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/scripting/ScriptFactory.h>
#include <memory>

namespace {

class Batch7376Plugin final : public Salix::IPlugin {
public:
    void on_register_types() override {
        Salix::ScriptFactory::get().register_script(
            "Batch7376FixtureProbe",
            [] { return std::make_unique<Salix::ScriptElement>(); });
    }

    void on_startup(Salix::InitContext& context) override {
        context.dpi_scale = 73.76f;
    }

    void on_shutdown() override {}
};

} // namespace

#if defined(_WIN32)
#define BATCH7376_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define BATCH7376_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

BATCH7376_PLUGIN_EXPORT Salix::IPlugin* create_plugin() {
#if defined(BATCH7376_NULL_PLUGIN)
    return nullptr;
#else
    static Batch7376Plugin plugin;
    return &plugin;
#endif
}
