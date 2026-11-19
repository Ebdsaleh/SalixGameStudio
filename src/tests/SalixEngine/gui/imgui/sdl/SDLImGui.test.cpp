#include <doctest.h>
#include <Salix/gui/DialogBox.h>
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

TEST_SUITE("Salix::gui::SDLImGuiBatch7578") {
    TEST_CASE("batch7578 fresh reinitialize exits safely without ImGui context") {
        Salix::SDLImGui gui;
        CHECK_NOTHROW(gui.reinitialize_backend());
    }
    TEST_CASE("batch7578 fresh dialog state reports missing key closed") {
        Salix::SDLImGui gui;
        CHECK_FALSE(gui.is_dialog_open("Missing7578"));
        CHECK_FALSE(gui.is_dialog_closed_this_frame("Missing7578"));
    }
    TEST_CASE("batch7578 missing dialog result returns default record") {
        Salix::SDLImGui gui;
        const auto result = gui.get_dialog_result("Missing7578");
        CHECK_FALSE(result.is_ok);
        CHECK(result.file_path_name.empty());
        CHECK(result.file_name.empty());
        CHECK(result.folder_path.empty());
    }
    TEST_CASE("batch7578 null raw input is rejected before SDL inspection") {
        Salix::SDLImGui gui;
        CHECK_FALSE(gui.process_raw_input_event(nullptr));
    }
    TEST_CASE("batch7578 null dialog registration is rejected") {
        Salix::SDLImGui gui;
        std::unique_ptr<Salix::DialogBox> empty;
        CHECK_FALSE(gui.register_dialog(std::move(empty)));
    }
    TEST_CASE("batch7578 default dialog registers and can be retrieved") {
        Salix::SDLImGui gui;
        auto dialog = std::make_unique<Salix::DialogBox>();
        Salix::DialogBox* raw = dialog.get();
        REQUIRE(gui.register_dialog(std::move(dialog)));
        CHECK(gui.get_dialog("") == raw);
    }
    TEST_CASE("batch7578 duplicate default dialog key is rejected") {
        Salix::SDLImGui gui;
        REQUIRE(gui.register_dialog(std::make_unique<Salix::DialogBox>()));
        CHECK_FALSE(gui.register_dialog(std::make_unique<Salix::DialogBox>()));
    }
    TEST_CASE("batch7578 empty show key is ignored") {
        Salix::SDLImGui gui;
        std::string key;
        gui.show_dialog_by_key(key);
        CHECK_FALSE(gui.is_dialog_open(key));
    }
    TEST_CASE("batch7578 nonempty show key becomes active without registry access") {
        Salix::SDLImGui gui;
        std::string key = "Active7578";
        gui.show_dialog_by_key(key);
        CHECK(gui.is_dialog_open(key));
    }
    TEST_CASE("batch7578 display removes active key missing from registry without ImGui backend") {
        Salix::SDLImGui gui;
        std::string key = "Orphan7578";
        gui.show_dialog_by_key(key);
        REQUIRE(gui.is_dialog_open(key));
        CHECK_NOTHROW(gui.display_dialogs());
        CHECK_FALSE(gui.is_dialog_open(key));
    }
}

