#include <doctest.h>
#include <Salix/gui/imgui/sdl/SDLImGui.h>
#include <Salix/core/ApplicationConfig.h>
#include <Tests/SalixEngine/mocking/window/MockIWindow.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <Tests/SalixEngine/mocking/events/MockIEventPoller.h>
#include <cstdint>

TEST_SUITE("Salix::gui::SDLImGuiBatch1647") {
    TEST_CASE("fresh SDL ImGui exposes no wired engine services") {
        Salix::SDLImGui gui;
        CHECK(gui.get_window() == nullptr);
        CHECK(gui.get_renderer() == nullptr);
        CHECK(gui.get_theme_manager() == nullptr);
        CHECK(gui.get_font_manager() == nullptr);
        CHECK(gui.get_icon_manager() == nullptr);
    }

    TEST_CASE("fresh SDL ImGui has no application config or ImGui context") {
        Salix::SDLImGui gui;
        CHECK(gui.get_app_config() == nullptr);
        CHECK(gui.get_context() == nullptr);
    }

    TEST_CASE("SDL ImGui rejects completely null initialization before creating context") {
        Salix::SDLImGui gui;
        CHECK_FALSE(gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr));
        CHECK(gui.get_context() == nullptr);
    }

    TEST_CASE("SDL ImGui rejects missing window even when renderer interface exists") {
        Salix::SDLImGui gui;
        MockIRenderer renderer;
        CHECK_FALSE(gui.initialize(nullptr, &renderer, nullptr, nullptr, nullptr));
        CHECK(gui.get_renderer() == nullptr);
    }

    TEST_CASE("SDL ImGui rejects missing renderer even when window interface exists") {
        Salix::SDLImGui gui;
        MockIWindow window;
        CHECK_FALSE(gui.initialize(&window, nullptr, nullptr, nullptr, nullptr));
        CHECK(gui.get_window() == nullptr);
    }

    TEST_CASE("SDL ImGui stores exact nonowning application config pointer") {
        Salix::SDLImGui gui;
        Salix::ApplicationConfig config;
        gui.set_app_config(&config);
        CHECK(gui.get_app_config() == &config);
    }

    TEST_CASE("SDL ImGui accepts null application config to clear prior pointer") {
        Salix::SDLImGui gui;
        Salix::ApplicationConfig config;
        gui.set_app_config(&config);
        REQUIRE(gui.get_app_config() == &config);
        gui.set_app_config(nullptr);
        CHECK(gui.get_app_config() == nullptr);
    }

    TEST_CASE("SDL ImGui theme reload request follows explicit request and clear handshake") {
        Salix::SDLImGui gui;
        CHECK_FALSE(gui.is_theme_reload_requested());
        gui.request_theme_reload();
        CHECK(gui.is_theme_reload_requested());
        gui.clear_theme_reload_request();
        CHECK_FALSE(gui.is_theme_reload_requested());
    }

    TEST_CASE("SDL ImGui event polling guard rejects missing event manager without registration") {
        Salix::SDLImGui gui;
        MockIEventPoller poller;
        gui.setup_event_polling(&poller, nullptr);
        CHECK(poller.raw_callback_count() == 0);
    }

    TEST_CASE("SDL ImGui registers one raw callback and destructor unregisters it") {
        MockIEventPoller poller;
        auto* fake_manager = reinterpret_cast<Salix::EventManager*>(static_cast<uintptr_t>(0x1));
        {
            Salix::SDLImGui gui;
            gui.setup_event_polling(&poller, fake_manager);
            CHECK(poller.raw_callback_count() == 1);
        }
        CHECK(poller.raw_callback_count() == 0);
    }
}
