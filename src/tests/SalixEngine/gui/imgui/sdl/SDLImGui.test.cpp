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

// ---- Relocated from src/tests/SalixEngine/gui/IGui.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/gui/IGui.h>
#include <Salix/core/ApplicationConfig.h>
#include <Tests/SalixEngine/mocking/gui/MockIGui.h>
#include <Tests/SalixEngine/mocking/gui/MockIFontManager.h>
#include <Tests/SalixEngine/mocking/gui/MockIThemeManager.h>
#include <Tests/SalixEngine/mocking/gui/MockIIconManager.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <Tests/SalixEngine/mocking/window/MockIWindow.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::gui::IGuiBatch1447") {
    TEST_CASE("GUI interface initialization wires exact engine service pointers") {
        MockIGui gui;
        MockIWindow window;
        MockIRenderer renderer;
        MockIThemeManager themes;
        MockIFontManager fonts;
        MockIIconManager icons;
        Salix::IGui* interface_ptr = &gui;
        CHECK(interface_ptr->initialize(&window, &renderer, &themes, &fonts, &icons));
        CHECK(interface_ptr->get_window() == &window);
        CHECK(interface_ptr->get_renderer() == &renderer);
        CHECK(interface_ptr->get_theme_manager() == &themes);
        CHECK(interface_ptr->get_font_manager() == &fonts);
        CHECK(interface_ptr->get_icon_manager() == &icons);
    }

    TEST_CASE("GUI interface controlled initialization failure preserves supplied service identity") {
        MockIGui gui;
        gui.initialize_result = false;
        MockIWindow window;
        CHECK_FALSE(gui.initialize(&window, nullptr, nullptr, nullptr, nullptr));
        CHECK(gui.initialize_calls == 1);
        CHECK(gui.get_window() == &window);
        CHECK(gui.get_renderer() == nullptr);
    }

    TEST_CASE("GUI frame lifecycle methods are independently observable") {
        MockIGui gui;
        Salix::IGui* interface_ptr = &gui;
        interface_ptr->new_frame();
        interface_ptr->render();
        interface_ptr->update_and_render_platform_windows();
        interface_ptr->shutdown();
        CHECK(gui.new_frame_calls == 1);
        CHECK(gui.render_calls == 1);
        CHECK(gui.platform_window_calls == 1);
        CHECK(gui.shutdown_calls == 1);
    }

    TEST_CASE("GUI mouse cursor and layout persistence requests retain exact arguments") {
        MockIGui gui;
        Salix::IGui* interface_ptr = &gui;
        interface_ptr->set_mouse_cursor_visible(false);
        interface_ptr->save_layout("Layouts/editor.ini");
        interface_ptr->load_layout("Layouts/game.ini");
        CHECK_FALSE(gui.mouse_cursor_visible);
        CHECK(gui.mouse_cursor_calls == 1);
        CHECK(gui.last_saved_layout == "Layouts/editor.ini");
        CHECK(gui.last_loaded_layout == "Layouts/game.ini");
        CHECK(gui.save_layout_calls == 1);
        CHECK(gui.load_layout_calls == 1);
    }

    TEST_CASE("GUI save file dialog request transports complete dialog metadata") {
        MockIGui gui;
        Salix::IGui* interface_ptr = &gui;
        CHECK(interface_ptr->open_save_file_dialog(
            "save", "Save Scene", ".scene", "Projects", "main.scene"));
        CHECK(gui.open_save_calls == 1);
        CHECK(gui.last_dialog_key == "save");
        CHECK(gui.last_dialog_title == "Save Scene");
        CHECK(gui.last_dialog_filters == ".scene");
        CHECK(gui.last_dialog_path == "Projects");
        CHECK(gui.last_dialog_filename == "main.scene");
    }

    TEST_CASE("GUI load and folder dialog requests preserve their distinct workflows") {
        MockIGui gui;
        Salix::IGui* interface_ptr = &gui;
        CHECK(interface_ptr->open_load_file_dialog("load", "Load Scene", ".scene", "Scenes"));
        CHECK(gui.open_load_calls == 1);
        CHECK(gui.last_dialog_key == "load");
        CHECK(gui.last_dialog_path == "Scenes");
        CHECK(interface_ptr->open_folder_dialog("folder", "Choose Folder", "Assets"));
        CHECK(gui.open_folder_calls == 1);
        CHECK(gui.last_dialog_key == "folder");
        CHECK(gui.last_dialog_path == "Assets");
    }

    TEST_CASE("GUI dialog open closed and result state remain independently queryable") {
        MockIGui gui;
        gui.last_dialog_key = "dialog";
        gui.dialog_is_open = true;
        gui.dialog_closed_this_frame = false;
        gui.dialog_result.is_ok = true;
        gui.dialog_result.file_name = "picked.asset";
        CHECK(gui.is_dialog_open("dialog"));
        CHECK_FALSE(gui.is_dialog_closed_this_frame("dialog"));
        CHECK_FALSE(gui.is_dialog_open("other"));
        const auto result = gui.get_dialog_result("dialog");
        CHECK(result.is_ok);
        CHECK(result.file_name == "picked.asset");
    }

    TEST_CASE("GUI raw input processing transports exact native event and configured consumption result") {
        MockIGui gui;
        gui.raw_input_result = true;
        int native = 77;
        Salix::IGui* interface_ptr = &gui;
        CHECK(interface_ptr->process_raw_input_event(&native));
        CHECK(gui.raw_input_calls == 1);
        CHECK(gui.last_native_event == &native);
    }

    TEST_CASE("GUI application configuration remains nonowning and independently replaceable") {
        MockIGui gui;
        Salix::ApplicationConfig first;
        Salix::ApplicationConfig second;
        Salix::IGui* interface_ptr = &gui;
        interface_ptr->set_app_config(&first);
        CHECK(interface_ptr->get_app_config() == &first);
        interface_ptr->set_app_config(&second);
        CHECK(interface_ptr->get_app_config() == &second);
        interface_ptr->set_app_config(nullptr);
        CHECK(interface_ptr->get_app_config() == nullptr);
    }

    TEST_CASE("GUI backend reinitialization and theme reload handshake are explicit state transitions") {
        MockIGui gui;
        Salix::IGui* interface_ptr = &gui;
        CHECK_FALSE(interface_ptr->is_theme_reload_requested());
        interface_ptr->request_theme_reload();
        CHECK(interface_ptr->is_theme_reload_requested());
        interface_ptr->reinitialize_backend();
        CHECK(gui.reinitialize_calls == 1);
        interface_ptr->clear_theme_reload_request();
        CHECK_FALSE(interface_ptr->is_theme_reload_requested());
        CHECK(interface_ptr->get_context() == nullptr);
    }
}
