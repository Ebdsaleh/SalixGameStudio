#include <doctest.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <Salix/events/IEventListener.h>
#include <Salix/events/ImGuiInputEvent.h>
#include <Salix/events/EventManager.h>
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

namespace {
struct Batch7678SDLImGuiBackend {
    bool had_video = false;
    bool had_render = false;
    std::string old_video;
    std::string old_render;
    SDL_Window* raw_window = nullptr;
    SDL_Renderer* raw_renderer = nullptr;
    MockIWindow window;
    MockIRenderer renderer;
    MockIEventPoller poller;
    Salix::ApplicationConfig config;
    Salix::SDLImGui gui;
    bool ready = false;

    Batch7678SDLImGuiBackend() {
        if (const char* value = SDL_getenv("SDL_VIDEODRIVER")) { had_video = true; old_video = value; }
        if (const char* value = SDL_getenv("SDL_RENDER_DRIVER")) { had_render = true; old_render = value; }
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
        SDL_setenv("SDL_RENDER_DRIVER", "software", 1);
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return;
        raw_window = SDL_CreateWindow("Batch7678 SDLImGui", 0, 0, 96, 64, SDL_WINDOW_HIDDEN);
        if (!raw_window) return;
        raw_renderer = SDL_CreateRenderer(raw_window, -1, SDL_RENDERER_SOFTWARE);
        if (!raw_renderer) return;
        window.native_handle = raw_window;
        window.width = 96;
        window.height = 64;
        renderer.native_handle = raw_renderer;
        config.renderer_type = Salix::RendererType::SDL;
        config.window_config.width = 96;
        config.window_config.height = 64;
        gui.set_app_config(&config);
        ready = gui.initialize(&window, &renderer, nullptr, nullptr, nullptr);
    }

    ~Batch7678SDLImGuiBackend() {
        if (ready) gui.shutdown();
        if (raw_renderer) SDL_DestroyRenderer(raw_renderer);
        if (raw_window) SDL_DestroyWindow(raw_window);
        SDL_Quit();
        if (had_video) SDL_setenv("SDL_VIDEODRIVER", old_video.c_str(), 1); else _putenv_s("SDL_VIDEODRIVER", "");
        if (had_render) SDL_setenv("SDL_RENDER_DRIVER", old_render.c_str(), 1); else _putenv_s("SDL_RENDER_DRIVER", "");
    }
};

class Batch7678ImGuiListener final : public Salix::IEventListener {
public:
    void on_event(Salix::IEvent& event) override {
        if (auto* input = dynamic_cast<Salix::ImGuiInputEvent*>(&event)) {
            ++count;
            mouse = input->is_mouse_captured();
            keyboard = input->is_keyboard_captured();
            text = input->to_string();
        }
    }
    int count = 0;
    bool mouse = false;
    bool keyboard = false;
    std::string text;
};
}

TEST_SUITE("Salix::gui::SDLImGuiBatch7678") {
    TEST_CASE("batch7678 real dummy SDL backend initializes successfully") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        CHECK(fixture.gui.get_context() != nullptr);
    }
    TEST_CASE("batch7678 successful SDL backend retains wired window and renderer interfaces") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        CHECK(fixture.gui.get_window() == &fixture.window);
        CHECK(fixture.gui.get_renderer() == &fixture.renderer);
    }
    TEST_CASE("batch7678 successful SDL backend new frame and render complete") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        CHECK_NOTHROW(fixture.gui.new_frame());
        CHECK_NOTHROW(fixture.gui.render());
    }
    TEST_CASE("batch7678 successful SDL backend dispatches ImGui input event") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        Salix::EventManager events;
        Batch7678ImGuiListener listener;
        events.subscribe(Salix::EventCategory::Input, &listener);
        fixture.gui.setup_event_polling(nullptr, nullptr);
        fixture.gui.setup_event_polling(&fixture.poller, &events);
        fixture.gui.new_frame();
        fixture.gui.render();
        events.process_queue();
        CHECK(listener.count == 1);
    }
    TEST_CASE("batch7678 successful SDL backend cursor visibility toggles config flag") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        CHECK_NOTHROW(fixture.gui.set_mouse_cursor_visible(false));
        CHECK_NOTHROW(fixture.gui.set_mouse_cursor_visible(true));
    }
    TEST_CASE("batch7678 successful SDL backend reinitializes renderer backend") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        CHECK_NOTHROW(fixture.gui.reinitialize_backend());
    }
    TEST_CASE("batch7678 successful SDL backend saves layout file") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        const auto path = std::filesystem::temp_directory_path() / "batch7678_sdl_imgui.ini";
        fixture.gui.save_layout(path.string());
        CHECK(std::filesystem::exists(path));
        std::error_code ec; std::filesystem::remove(path, ec);
    }
    TEST_CASE("batch7678 successful SDL backend loads layout file") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        const auto path = std::filesystem::temp_directory_path() / "batch7678_sdl_imgui_load.ini";
        { std::ofstream out(path); out << "[Window][Batch7678]\nPos=0,0\nSize=96,64\nCollapsed=0\n"; }
        CHECK_NOTHROW(fixture.gui.load_layout(path.string()));
        std::error_code ec; std::filesystem::remove(path, ec);
    }
    TEST_CASE("batch7678 successful SDL backend opens and tracks save dialog") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        REQUIRE(fixture.gui.open_save_file_dialog("Save7678", "Save", ".txt", ".", "x.txt"));
        CHECK(fixture.gui.is_dialog_open("Save7678"));
    }
    TEST_CASE("batch7678 successful SDL backend opens and tracks folder dialog") {
        Batch7678SDLImGuiBackend fixture;
        REQUIRE(fixture.ready);
        REQUIRE(fixture.gui.open_folder_dialog("Folder7678", "Folder", "."));
        CHECK(fixture.gui.is_dialog_open("Folder7678"));
    }
}

