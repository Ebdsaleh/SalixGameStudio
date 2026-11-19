#include <doctest.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <imgui_internal.h>
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

TEST_SUITE("Salix::gui::SDLImGuiBatch7778") {
    TEST_CASE("batch7778 SDL ImGui raw mouse event flows through registered mock poller") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); Salix::EventManager events; f.gui.setup_event_polling(&f.poller,&events);
        SDL_Event e{};e.type=SDL_MOUSEMOTION;e.motion.x=77;e.motion.y=78;CHECK_NOTHROW(f.poller.emit_raw(&e));
    }
    TEST_CASE("batch7778 SDL ImGui raw nonescape key event remains safe") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); Salix::EventManager events; f.gui.setup_event_polling(&f.poller,&events);
        SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.sym=SDLK_a;CHECK_NOTHROW(f.poller.emit_raw(&e));
    }
    TEST_CASE("batch7778 SDL ImGui load dialog opens and tracks key") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); REQUIRE(f.gui.open_load_file_dialog("Load7778","Load",".txt",".")); CHECK(f.gui.is_dialog_open("Load7778"));
    }
    TEST_CASE("batch7778 SDL ImGui escape closes active file dialog") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); REQUIRE(f.gui.open_load_file_dialog("Escape7778","Load",".txt","."));
        SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.sym=SDLK_ESCAPE;CHECK(f.gui.process_raw_input_event(&e)); CHECK_FALSE(f.gui.is_dialog_open("Escape7778"));
    }
    TEST_CASE("batch7778 SDL ImGui creates overwrite dialog with exact registry identity") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); std::string key="Create7778";std::string title="Create";
        auto* d=f.gui.create_dialog(key,title,Salix::DialogType::File,true); REQUIRE(d!=nullptr); CHECK(f.gui.get_dialog(key)==d);
    }
    TEST_CASE("batch7778 SDL ImGui creates folder dialog wrapper") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); std::string key="FolderCreate7778";std::string title="Folder";
        auto* d=f.gui.create_dialog(key,title,Salix::DialogType::Folder,false); REQUIRE(d!=nullptr); CHECK(f.gui.get_dialog(key)==d);
    }
    TEST_CASE("batch7778 SDL ImGui show key marks created dialog active") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); std::string key="Show7778";std::string title="Show";REQUIRE(f.gui.create_dialog(key,title,Salix::DialogType::File,false)!=nullptr);
        f.gui.show_dialog_by_key(key); CHECK(f.gui.is_dialog_open(key));
    }
    TEST_CASE("batch7778 SDL ImGui nonempty missing result remains default") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); auto result=f.gui.populate_dialog_result("Missing7778"); CHECK_FALSE(result.is_ok);
    }
    TEST_CASE("batch7778 SDL ImGui platform windows update follows completed frame") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); f.gui.new_frame(); f.gui.render(); CHECK_NOTHROW(f.gui.update_and_render_platform_windows());
    }
    TEST_CASE("batch7778 SDL ImGui shutdown unregisters mock raw callback") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); Salix::EventManager events; f.gui.setup_event_polling(&f.poller,&events); REQUIRE(f.poller.raw_callback_count()==1u);
        f.gui.shutdown(); f.ready=false; CHECK(f.poller.raw_callback_count()==0u);
    }
}

TEST_SUITE("Salix::gui::SDLImGuiBatch7878") {
    TEST_CASE("batch7878 SDL dialog display opens registered load dialog") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);std::string key="SDLDisplay7878",title="Load";REQUIRE(f.gui.create_dialog(key,title,Salix::DialogType::File,false)!=nullptr);f.gui.show_dialog_by_key(key);f.gui.new_frame();CHECK_NOTHROW(f.gui.display_dialogs());f.gui.render();CHECK(f.gui.is_dialog_open(key)); }
    TEST_CASE("batch7878 SDL displayed dialog consumes escape and clears active key") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);REQUIRE(f.gui.open_load_file_dialog("SDLEscape7878","Load",".txt","."));f.gui.new_frame();f.gui.display_dialogs();f.gui.render();SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.sym=SDLK_ESCAPE;CHECK(f.gui.process_raw_input_event(&e));CHECK_FALSE(f.gui.is_dialog_open("SDLEscape7878")); }
    TEST_CASE("batch7878 SDL active dialog result query executes nonempty result path") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);REQUIRE(f.gui.open_save_file_dialog("SDLResult7878","Save",".txt",".","a.txt"));f.gui.new_frame();f.gui.display_dialogs();f.gui.render();auto result=f.gui.populate_dialog_result("SDLResult7878");CHECK_FALSE(result.is_ok); }
    TEST_CASE("batch7878 SDL missing dialog lookup remains null after registry use") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);std::string key="SDLKnown7878",title="Known";REQUIRE(f.gui.create_dialog(key,title,Salix::DialogType::File,false)!=nullptr);CHECK(f.gui.get_dialog("SDLMissing7878")==nullptr); }
    TEST_CASE("batch7878 SDL duplicate create returns original dialog") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);std::string key="SDLDup7878",a="A",b="B";auto* first=f.gui.create_dialog(key,a,Salix::DialogType::File,false);REQUIRE(first!=nullptr);CHECK(f.gui.create_dialog(key,b,Salix::DialogType::File,true)==first); }
    TEST_CASE("batch7878 SDL spare GUI reinitialize rejects null backend handles with DLL context alive") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);Salix::SDLImGui spare;CHECK_NOTHROW(spare.reinitialize_backend()); }
    TEST_CASE("batch7878 SDL platform update executes with explicit viewport flag after completed frame") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);REQUIRE(f.gui.get_context()!=nullptr);f.gui.get_context()->IO.ConfigFlags|=ImGuiConfigFlags_ViewportsEnable;f.gui.new_frame();f.gui.render();CHECK_NOTHROW(f.gui.update_and_render_platform_windows());f.gui.get_context()->IO.ConfigFlags&=~ImGuiConfigFlags_ViewportsEnable; }
    TEST_CASE("batch7878 SDL folder dialog display executes first frame open path") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);REQUIRE(f.gui.open_folder_dialog("SDLFolderDisplay7878","Folder","."));f.gui.new_frame();CHECK_NOTHROW(f.gui.display_dialogs());f.gui.render(); }
    TEST_CASE("batch7878 SDL save dialog display executes configured filename path") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);REQUIRE(f.gui.open_save_file_dialog("SDLSaveDisplay7878","Save",".txt",".","batch7878.txt"));f.gui.new_frame();CHECK_NOTHROW(f.gui.display_dialogs());f.gui.render(); }
    TEST_CASE("batch7878 SDL explicit shutdown unregisters backend and callback safely") { Batch7678SDLImGuiBackend f;REQUIRE(f.ready);Salix::EventManager events;f.gui.setup_event_polling(&f.poller,&events);REQUIRE(f.poller.raw_callback_count()==1u);CHECK_NOTHROW(f.gui.shutdown());f.ready=false;CHECK(f.poller.raw_callback_count()==0u); }
}

TEST_SUITE("Salix::gui::SDLImGuiBatch7978") {
    TEST_CASE("batch7978 SDL GUI rejects interfaces whose native handles are both null") {
        Salix::SDLImGui gui; MockIWindow window; MockIRenderer renderer;
        CHECK_FALSE(gui.initialize(&window,&renderer,nullptr,nullptr,nullptr));
    }
    TEST_CASE("batch7978 SDL GUI rejects null renderer native handle beside nonnull window handle") {
        Salix::SDLImGui gui; MockIWindow window; MockIRenderer renderer;
        window.native_handle=reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x7978));
        CHECK_FALSE(gui.initialize(&window,&renderer,nullptr,nullptr,nullptr));
    }
    TEST_CASE("batch7978 SDL GUI rejects null window native handle beside nonnull renderer handle") {
        Salix::SDLImGui gui; MockIWindow window; MockIRenderer renderer;
        renderer.native_handle=reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x7978));
        CHECK_FALSE(gui.initialize(&window,&renderer,nullptr,nullptr,nullptr));
    }
    TEST_CASE("batch7978 SDL GUI platform windows update remains safe after completed frame") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); f.gui.new_frame(); f.gui.render();
        CHECK_NOTHROW(f.gui.update_and_render_platform_windows());
    }
    TEST_CASE("batch7978 SDL GUI theme reload handshake remains explicit") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); CHECK_FALSE(f.gui.is_theme_reload_requested());
        f.gui.request_theme_reload(); CHECK(f.gui.is_theme_reload_requested()); f.gui.clear_theme_reload_request();
        CHECK_FALSE(f.gui.is_theme_reload_requested());
    }
    TEST_CASE("batch7978 SDL GUI direct dialog registration preserves two distinct keys") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready);
        REQUIRE(f.gui.register_dialog(std::make_unique<Salix::DialogBox>("SDLRegister7978A","A",Salix::DialogType::File,false)));
        REQUIRE(f.gui.register_dialog(std::make_unique<Salix::DialogBox>("SDLRegister7978B","B",Salix::DialogType::Folder,false)));
        CHECK(f.gui.get_dialog("SDLRegister7978A")!=nullptr); CHECK(f.gui.get_dialog("SDLRegister7978B")!=nullptr);
    }
    TEST_CASE("batch7978 SDL GUI orphan active key is removed by display pass") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); std::string key="SDLOrphan7978";
        f.gui.show_dialog_by_key(key); REQUIRE(f.gui.is_dialog_open(key)); CHECK_NOTHROW(f.gui.display_dialogs()); CHECK_FALSE(f.gui.is_dialog_open(key));
    }
    TEST_CASE("batch7978 SDL GUI common dialog geometry accepts cleared application config") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); f.gui.set_app_config(nullptr);
        f.gui.new_frame(); CHECK_NOTHROW(f.gui.set_common_dialog_properties()); f.gui.render(); CHECK(f.gui.get_app_config()==nullptr);
    }
    TEST_CASE("batch7978 SDL GUI cursor policy toggles after completed frame") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready); f.gui.new_frame(); f.gui.render();
        CHECK_NOTHROW(f.gui.set_mouse_cursor_visible(false)); CHECK_NOTHROW(f.gui.set_mouse_cursor_visible(true));
    }
    TEST_CASE("batch7978 SDL GUI layout roundtrip remains safe with live context") {
        Batch7678SDLImGuiBackend f; REQUIRE(f.ready);
        const auto path=std::filesystem::temp_directory_path()/"batch7978_sdl_layout.ini";
        f.gui.save_layout(path.string()); REQUIRE(std::filesystem::exists(path)); CHECK_NOTHROW(f.gui.load_layout(path.string()));
        std::error_code ec; std::filesystem::remove(path,ec);
    }
}

