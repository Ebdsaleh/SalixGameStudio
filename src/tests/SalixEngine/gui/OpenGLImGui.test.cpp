#include <doctest.h>
#include <Salix/gui/imgui/opengl/OpenGLImGui.h>
#include <Salix/core/ApplicationConfig.h>
#include <Tests/SalixEngine/mocking/window/MockIWindow.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <Tests/SalixEngine/mocking/events/MockIEventPoller.h>
#include <cstdint>

TEST_SUITE("Salix::gui::OpenGLImGuiBatch1647") {
    TEST_CASE("fresh OpenGL ImGui exposes no wired engine services") {
        Salix::OpenGLImGui gui;
        CHECK(gui.get_window() == nullptr);
        CHECK(gui.get_renderer() == nullptr);
        CHECK(gui.get_theme_manager() == nullptr);
        CHECK(gui.get_font_manager() == nullptr);
        CHECK(gui.get_icon_manager() == nullptr);
    }

    TEST_CASE("fresh OpenGL ImGui has no application config or ImGui context") {
        Salix::OpenGLImGui gui;
        CHECK(gui.get_app_config() == nullptr);
        CHECK(gui.get_context() == nullptr);
    }

    TEST_CASE("OpenGL ImGui rejects completely null initialization before creating context") {
        Salix::OpenGLImGui gui;
        CHECK_FALSE(gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr));
        CHECK(gui.get_context() == nullptr);
    }

    TEST_CASE("OpenGL ImGui rejects missing window even when renderer interface exists") {
        Salix::OpenGLImGui gui;
        MockIRenderer renderer;
        CHECK_FALSE(gui.initialize(nullptr, &renderer, nullptr, nullptr, nullptr));
        CHECK(gui.get_renderer() == nullptr);
    }

    TEST_CASE("OpenGL ImGui rejects missing renderer even when window interface exists") {
        Salix::OpenGLImGui gui;
        MockIWindow window;
        CHECK_FALSE(gui.initialize(&window, nullptr, nullptr, nullptr, nullptr));
        CHECK(gui.get_window() == nullptr);
    }

    TEST_CASE("OpenGL ImGui stores exact nonowning application config pointer") {
        Salix::OpenGLImGui gui;
        Salix::ApplicationConfig config;
        gui.set_app_config(&config);
        CHECK(gui.get_app_config() == &config);
    }

    TEST_CASE("OpenGL ImGui ignores null application config replacement") {
        Salix::OpenGLImGui gui;
        Salix::ApplicationConfig config;
        gui.set_app_config(&config);
        gui.set_app_config(nullptr);
        CHECK(gui.get_app_config() == &config);
    }

    TEST_CASE("OpenGL ImGui theme reload request follows explicit request and clear handshake") {
        Salix::OpenGLImGui gui;
        CHECK_FALSE(gui.is_theme_reload_requested());
        gui.request_theme_reload();
        CHECK(gui.is_theme_reload_requested());
        gui.clear_theme_reload_request();
        CHECK_FALSE(gui.is_theme_reload_requested());
    }

    TEST_CASE("OpenGL ImGui event polling guard rejects missing event manager without registration") {
        Salix::OpenGLImGui gui;
        MockIEventPoller poller;
        gui.setup_event_polling(&poller, nullptr);
        CHECK(poller.raw_callback_count() == 0);
    }

    TEST_CASE("OpenGL ImGui registers one raw callback and destructor unregisters it") {
        MockIEventPoller poller;
        auto* fake_manager = reinterpret_cast<Salix::EventManager*>(static_cast<uintptr_t>(0x1));
        {
            Salix::OpenGLImGui gui;
            gui.setup_event_polling(&poller, fake_manager);
            CHECK(poller.raw_callback_count() == 1);
        }
        CHECK(poller.raw_callback_count() == 0);
    }
}
