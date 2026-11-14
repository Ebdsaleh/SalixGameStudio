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

TEST_SUITE("Salix::core::IPluginBatch6447") {
    TEST_CASE("batch6447 repeated registration preserves exact call order") {
        MockIPlugin plugin;
        Salix::IPlugin& interface = plugin;
        interface.on_register_types();
        interface.on_register_types();
        REQUIRE(plugin.call_order.size() == 2);
        CHECK(plugin.call_order[0] == "register");
        CHECK(plugin.call_order[1] == "register");
    }

    TEST_CASE("batch6447 startup without registration records only startup") {
        MockIPlugin plugin;
        Salix::InitContext context{};
        Salix::IPlugin& interface = plugin;
        interface.on_startup(context);
        REQUIRE(plugin.call_order.size() == 1);
        CHECK(plugin.call_order[0] == "startup");
        CHECK(plugin.register_calls == 0);
    }

    TEST_CASE("batch6447 shutdown may be called repeatedly and remains observable") {
        MockIPlugin plugin;
        Salix::IPlugin& interface = plugin;
        interface.on_shutdown();
        interface.on_shutdown();
        CHECK(plugin.shutdown_calls == 2);
        REQUIRE(plugin.call_order.size() == 2);
        CHECK(plugin.call_order[0] == "shutdown");
        CHECK(plugin.call_order[1] == "shutdown");
    }

    TEST_CASE("batch6447 startup handler can mutate DPI without touching engine mode") {
        MockIPlugin plugin;
        plugin.startup_handler = [](Salix::InitContext& context) {
            context.dpi_scale = 6.447f;
        };
        Salix::InitContext context{};
        context.engine_mode = Salix::EngineMode::Editor;
        plugin.on_startup(context);
        CHECK(context.dpi_scale == doctest::Approx(6.447f));
        CHECK(context.engine_mode == Salix::EngineMode::Editor);
    }

    TEST_CASE("batch6447 startup handler can mutate project path without touching renderer type") {
        MockIPlugin plugin;
        plugin.startup_handler = [](Salix::InitContext& context) {
            context.project_dll_path = "Batch6447.dll";
        };
        Salix::InitContext context{};
        context.renderer_type = Salix::RendererType::OpenGL;
        plugin.on_startup(context);
        CHECK(context.project_dll_path == "Batch6447.dll");
        CHECK(context.renderer_type == Salix::RendererType::OpenGL);
    }

    TEST_CASE("batch6447 startup receives same context across repeated calls") {
        MockIPlugin plugin;
        Salix::InitContext context{};
        plugin.on_startup(context);
        REQUIRE(plugin.last_context == &context);
        plugin.on_startup(context);
        CHECK(plugin.last_context == &context);
        CHECK(plugin.startup_calls == 2);
    }

    TEST_CASE("batch6447 register startup register ordering remains exact") {
        MockIPlugin plugin;
        Salix::InitContext context{};
        plugin.on_register_types();
        plugin.on_startup(context);
        plugin.on_register_types();
        REQUIRE(plugin.call_order.size() == 3);
        CHECK(plugin.call_order[0] == "register");
        CHECK(plugin.call_order[1] == "startup");
        CHECK(plugin.call_order[2] == "register");
    }

    TEST_CASE("batch6447 shutdown startup ordering remains observable") {
        MockIPlugin plugin;
        Salix::InitContext context{};
        plugin.on_shutdown();
        plugin.on_startup(context);
        REQUIRE(plugin.call_order.size() == 2);
        CHECK(plugin.call_order[0] == "shutdown");
        CHECK(plugin.call_order[1] == "startup");
    }

    TEST_CASE("batch6447 independent plugins never share last context pointer") {
        MockIPlugin first;
        MockIPlugin second;
        Salix::InitContext a{};
        Salix::InitContext b{};
        first.on_startup(a);
        second.on_startup(b);
        CHECK(first.last_context == &a);
        CHECK(second.last_context == &b);
        CHECK(first.last_context != second.last_context);
    }

    TEST_CASE("batch6447 polymorphic ownership dispatches complete lifecycle") {
        std::unique_ptr<Salix::IPlugin> plugin = std::make_unique<MockIPlugin>();
        Salix::InitContext context{};
        CHECK_NOTHROW(plugin->on_register_types());
        CHECK_NOTHROW(plugin->on_startup(context));
        CHECK_NOTHROW(plugin->on_shutdown());
    }
}
