#include <doctest.h>
#include <fstream>
#include <filesystem>
#include <imgui.h>
#include <Salix/gui/DialogBox.h>
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

TEST_SUITE("Salix::gui::OpenGLImGuiBatch7578") {
    TEST_CASE("batch7578 fresh reinitialize exits safely without ImGui context") {
        Salix::OpenGLImGui gui;
        CHECK_NOTHROW(gui.reinitialize_backend());
    }
    TEST_CASE("batch7578 fresh dialog state reports missing key closed") {
        Salix::OpenGLImGui gui;
        CHECK_FALSE(gui.is_dialog_open("Missing7578"));
        CHECK_FALSE(gui.is_dialog_closed_this_frame("Missing7578"));
    }
    TEST_CASE("batch7578 missing dialog result returns default record") {
        Salix::OpenGLImGui gui;
        const auto result = gui.get_dialog_result("Missing7578");
        CHECK_FALSE(result.is_ok);
        CHECK(result.file_path_name.empty());
        CHECK(result.file_name.empty());
        CHECK(result.folder_path.empty());
    }
    TEST_CASE("batch7578 null raw input is rejected before SDL inspection") {
        Salix::OpenGLImGui gui;
        CHECK_FALSE(gui.process_raw_input_event(nullptr));
    }
    TEST_CASE("batch7578 null dialog registration is rejected") {
        Salix::OpenGLImGui gui;
        std::unique_ptr<Salix::DialogBox> empty;
        CHECK_FALSE(gui.register_dialog(std::move(empty)));
    }
    TEST_CASE("batch7578 default dialog registers and can be retrieved") {
        Salix::OpenGLImGui gui;
        auto dialog = std::make_unique<Salix::DialogBox>();
        Salix::DialogBox* raw = dialog.get();
        REQUIRE(gui.register_dialog(std::move(dialog)));
        CHECK(gui.get_dialog("") == raw);
    }
    TEST_CASE("batch7578 duplicate default dialog key is rejected") {
        Salix::OpenGLImGui gui;
        REQUIRE(gui.register_dialog(std::make_unique<Salix::DialogBox>()));
        CHECK_FALSE(gui.register_dialog(std::make_unique<Salix::DialogBox>()));
    }
    TEST_CASE("batch7578 empty show key is ignored") {
        Salix::OpenGLImGui gui;
        std::string key;
        gui.show_dialog_by_key(key);
        CHECK_FALSE(gui.is_dialog_open(key));
    }
    TEST_CASE("batch7578 nonempty show key becomes active without registry access") {
        Salix::OpenGLImGui gui;
        std::string key = "Active7578";
        gui.show_dialog_by_key(key);
        CHECK(gui.is_dialog_open(key));
    }
    TEST_CASE("batch7578 display removes active key missing from registry without ImGui backend") {
        Salix::OpenGLImGui gui;
        std::string key = "Orphan7578";
        gui.show_dialog_by_key(key);
        REQUIRE(gui.is_dialog_open(key));
        CHECK_NOTHROW(gui.display_dialogs());
        CHECK_FALSE(gui.is_dialog_open(key));
    }
}

namespace {
struct Batch7678ImGuiContextGuard {
    ImGuiContext* context = nullptr;
    Batch7678ImGuiContextGuard() {
        context = ImGui::CreateContext();
        ImGui::SetCurrentContext(context);
        ImGui::GetIO().DisplaySize = ImVec2(800.0f, 600.0f);
    }
    ~Batch7678ImGuiContextGuard() {
        if (ImGui::GetCurrentContext()) ImGui::DestroyContext();
    }
};
}

TEST_SUITE("Salix::gui::OpenGLImGuiBatch7678") {
    TEST_CASE("batch7678 standalone context toggles OpenGL GUI cursor suppression") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        gui.set_mouse_cursor_visible(false);
        CHECK((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) != 0);
        gui.set_mouse_cursor_visible(true);
        CHECK((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0);
    }
    TEST_CASE("batch7678 standalone context saves OpenGL GUI layout") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        const auto path = std::filesystem::temp_directory_path() / "batch7678_gl_gui.ini";
        gui.save_layout(path.string());
        CHECK(std::filesystem::exists(path));
        std::error_code ec; std::filesystem::remove(path, ec);
    }
    TEST_CASE("batch7678 standalone context loads OpenGL GUI layout") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        const auto path = std::filesystem::temp_directory_path() / "batch7678_gl_gui_load.ini";
        { std::ofstream out(path); out << "[Window][Batch7678]\nPos=0,0\nSize=10,10\nCollapsed=0\n"; }
        CHECK_NOTHROW(gui.load_layout(path.string()));
        std::error_code ec; std::filesystem::remove(path, ec);
    }
    TEST_CASE("batch7678 standalone context applies default dialog geometry") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        CHECK_NOTHROW(gui.set_common_dialog_properties());
    }
    TEST_CASE("batch7678 standalone context applies configured dialog geometry") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        Salix::ApplicationConfig config;
        config.window_config.width = 1000;
        config.window_config.height = 500;
        config.gui_settings.dialog_width_ratio = 0.5f;
        config.gui_settings.dialog_height_ratio = 0.4f;
        gui.set_app_config(&config);
        CHECK_NOTHROW(gui.set_common_dialog_properties());
    }
    TEST_CASE("batch7678 empty populate result returns default without dialog query") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        const auto result = gui.populate_dialog_result("");
        CHECK_FALSE(result.is_ok);
        CHECK(result.file_name.empty());
    }
    TEST_CASE("batch7678 create dialog registers unique key") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        std::string key = "Create7678";
        std::string title = "Create";
        auto* dialog = gui.create_dialog(key, title, Salix::DialogType::File, true);
        REQUIRE(dialog != nullptr);
        CHECK(gui.get_dialog(key) == dialog);
    }
    TEST_CASE("batch7678 duplicate create dialog returns original instance") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        std::string key = "Duplicate7678";
        std::string title = "One";
        auto* first = gui.create_dialog(key, title, Salix::DialogType::File, false);
        REQUIRE(first != nullptr);
        std::string second_title = "Two";
        auto* second = gui.create_dialog(key, second_title, Salix::DialogType::File, true);
        CHECK(second == first);
    }
    TEST_CASE("batch7678 non escape key raw input remains unconsumed") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        SDL_Event event{};
        event.type = SDL_KEYDOWN;
        event.key.keysym.sym = SDLK_a;
        CHECK_FALSE(gui.process_raw_input_event(&event));
    }
    TEST_CASE("batch7678 standalone context opens and tracks load dialog") {
        Batch7678ImGuiContextGuard context;
        Salix::OpenGLImGui gui;
        REQUIRE(gui.open_load_file_dialog("Load7678", "Load", ".txt", "."));
        CHECK(gui.is_dialog_open("Load7678"));
        ImGuiFileDialog::Instance()->Close();
    }
}

