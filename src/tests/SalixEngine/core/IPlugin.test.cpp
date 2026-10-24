#include <doctest.h>
#include <Salix/core/IPlugin.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/core/MockIPlugin.h>
#include <memory>
#include <string>

namespace {
class Batch1247DestructionPlugin : public MockIPlugin {
public:
    explicit Batch1247DestructionPlugin(bool* destroyed) : destroyed_(destroyed) {}
    ~Batch1247DestructionPlugin() override {
        if (destroyed_) {
            *destroyed_ = true;
        }
    }

private:
    bool* destroyed_;
};
}

TEST_SUITE("Salix::core::IPluginBatch1247") {
    TEST_CASE("plugin construction performs no lifecycle callbacks implicitly") {
        MockIPlugin plugin;
        CHECK(plugin.register_calls == 0);
        CHECK(plugin.startup_calls == 0);
        CHECK(plugin.shutdown_calls == 0);
        CHECK(plugin.call_order.empty());
    }

    TEST_CASE("plugin interface dispatches register types callback") {
        MockIPlugin plugin;
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_register_types();
        CHECK(plugin.register_calls == 1);
        REQUIRE(plugin.call_order.size() == 1);
        CHECK(plugin.call_order[0] == "register");
    }

    TEST_CASE("plugin interface startup receives the exact mutable InitContext reference") {
        MockIPlugin plugin;
        Salix::InitContext context{};
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_startup(context);
        CHECK(plugin.startup_calls == 1);
        CHECK(plugin.last_context == &context);
    }

    TEST_CASE("plugin interface dispatches shutdown callback without requiring startup") {
        MockIPlugin plugin;
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_shutdown();
        CHECK(plugin.shutdown_calls == 1);
        REQUIRE(plugin.call_order.size() == 1);
        CHECK(plugin.call_order[0] == "shutdown");
    }

    TEST_CASE("plugin lifecycle preserves explicit register startup shutdown ordering") {
        MockIPlugin plugin;
        Salix::InitContext context{};
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_register_types();
        interface_ptr->on_startup(context);
        interface_ptr->on_shutdown();
        REQUIRE(plugin.call_order.size() == 3);
        CHECK(plugin.call_order[0] == "register");
        CHECK(plugin.call_order[1] == "startup");
        CHECK(plugin.call_order[2] == "shutdown");
    }

    TEST_CASE("plugin interface allows repeated type registration requests to remain observable") {
        MockIPlugin plugin;
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_register_types();
        interface_ptr->on_register_types();
        CHECK(plugin.register_calls == 2);
        CHECK(plugin.startup_calls == 0);
    }

    TEST_CASE("plugin repeated startup updates the most recently supplied context reference") {
        MockIPlugin plugin;
        Salix::InitContext first{};
        Salix::InitContext second{};
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_startup(first);
        CHECK(plugin.last_context == &first);
        interface_ptr->on_startup(second);
        CHECK(plugin.last_context == &second);
        CHECK(plugin.startup_calls == 2);
    }

    TEST_CASE("plugin startup callback can intentionally configure shared initialization context") {
        MockIPlugin plugin;
        plugin.startup_handler = [](Salix::InitContext& context) {
            context.engine_mode = Salix::EngineMode::Game;
            context.project_dll_path = "PluginGame.dll";
        };

        Salix::InitContext context{};
        Salix::IPlugin* interface_ptr = &plugin;
        interface_ptr->on_startup(context);

        CHECK(context.engine_mode == Salix::EngineMode::Game);
        CHECK(context.project_dll_path == "PluginGame.dll");
    }

    TEST_CASE("separate plugin instances maintain independent lifecycle accounting") {
        MockIPlugin first;
        MockIPlugin second;
        Salix::InitContext context{};
        first.on_register_types();
        first.on_startup(context);
        second.on_shutdown();
        CHECK(first.register_calls == 1);
        CHECK(first.startup_calls == 1);
        CHECK(first.shutdown_calls == 0);
        CHECK(second.register_calls == 0);
        CHECK(second.startup_calls == 0);
        CHECK(second.shutdown_calls == 1);
    }

    TEST_CASE("plugin interface virtual destructor destroys concrete implementation through base ownership") {
        bool destroyed = false;
        {
            std::unique_ptr<Salix::IPlugin> plugin =
                std::make_unique<Batch1247DestructionPlugin>(&destroyed);
            CHECK_FALSE(destroyed);
        }
        CHECK(destroyed);
    }
}
