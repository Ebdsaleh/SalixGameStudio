#include <doctest.h>
#include <SDL.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <filesystem>
#include <imgui.h>
#include <Salix/gui/DialogBox.h>
#include <Salix/gui/imgui/opengl/OpenGLImGui.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/events/EventManager.h>
#include <Salix/gui/imgui/sdl/SDLImGui.h>
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
struct Batch7678OpenGLCoreContext {
    bool had_video = false;
    bool had_render = false;
    std::string old_video;
    std::string old_render;
    SDL_Window* raw_window = nullptr;
    SDL_Renderer* raw_renderer = nullptr;
    MockIWindow window;
    MockIRenderer renderer;
    Salix::ApplicationConfig config;
    Salix::SDLImGui context_owner;
    bool ready = false;

    Batch7678OpenGLCoreContext() {
        if (const char* value = SDL_getenv("SDL_VIDEODRIVER")) { had_video = true; old_video = value; }
        if (const char* value = SDL_getenv("SDL_RENDER_DRIVER")) { had_render = true; old_render = value; }
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
        SDL_setenv("SDL_RENDER_DRIVER", "software", 1);
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return;
        raw_window = SDL_CreateWindow("Batch7678 OpenGL Core Context", 0, 0, 64, 48, SDL_WINDOW_HIDDEN);
        if (!raw_window) return;
        raw_renderer = SDL_CreateRenderer(raw_window, -1, SDL_RENDERER_SOFTWARE);
        if (!raw_renderer) return;
        window.native_handle = raw_window;
        renderer.native_handle = raw_renderer;
        config.renderer_type = Salix::RendererType::SDL;
        context_owner.set_app_config(&config);
        ready = context_owner.initialize(&window, &renderer, nullptr, nullptr, nullptr);
    }

    ~Batch7678OpenGLCoreContext() {
        if (ready) context_owner.shutdown();
        if (raw_renderer) SDL_DestroyRenderer(raw_renderer);
        if (raw_window) SDL_DestroyWindow(raw_window);
        SDL_Quit();
        if (had_video) SDL_setenv("SDL_VIDEODRIVER", old_video.c_str(), 1); else _putenv_s("SDL_VIDEODRIVER", "");
        if (had_render) SDL_setenv("SDL_RENDER_DRIVER", old_render.c_str(), 1); else _putenv_s("SDL_RENDER_DRIVER", "");
    }
};
}

TEST_SUITE("Salix::gui::OpenGLImGuiBatch7678") {
    TEST_CASE("batch7678 DLL owned context accepts OpenGL GUI cursor suppression calls") {
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
        Salix::OpenGLImGui gui;
        CHECK_NOTHROW(gui.set_mouse_cursor_visible(false));
        CHECK_NOTHROW(gui.set_mouse_cursor_visible(true));
    }
    TEST_CASE("batch7678 DLL owned context saves OpenGL GUI layout") {
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
        Salix::OpenGLImGui gui;
        const auto path = std::filesystem::temp_directory_path() / "batch7678_gl_gui.ini";
        gui.save_layout(path.string());
        CHECK(std::filesystem::exists(path));
        std::error_code ec; std::filesystem::remove(path, ec);
    }
    TEST_CASE("batch7678 DLL owned context loads OpenGL GUI layout") {
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
        Salix::OpenGLImGui gui;
        const auto path = std::filesystem::temp_directory_path() / "batch7678_gl_gui_load.ini";
        { std::ofstream out(path); out << "[Window][Batch7678]\nPos=0,0\nSize=10,10\nCollapsed=0\n"; }
        CHECK_NOTHROW(gui.load_layout(path.string()));
        std::error_code ec; std::filesystem::remove(path, ec);
    }
    TEST_CASE("batch7678 DLL owned context applies default dialog geometry") {
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
        Salix::OpenGLImGui gui;
        CHECK_NOTHROW(gui.set_common_dialog_properties());
    }
    TEST_CASE("batch7678 DLL owned context applies configured dialog geometry") {
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
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
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
        Salix::OpenGLImGui gui;
        const auto result = gui.populate_dialog_result("");
        CHECK_FALSE(result.is_ok);
        CHECK(result.file_name.empty());
    }
    TEST_CASE("batch7678 create dialog registers unique key") {
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
        Salix::OpenGLImGui gui;
        std::string key = "Create7678";
        std::string title = "Create";
        auto* dialog = gui.create_dialog(key, title, Salix::DialogType::File, true);
        REQUIRE(dialog != nullptr);
        CHECK(gui.get_dialog(key) == dialog);
    }
    TEST_CASE("batch7678 duplicate create dialog returns original instance") {
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
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
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
        Salix::OpenGLImGui gui;
        SDL_Event event{};
        event.type = SDL_KEYDOWN;
        event.key.keysym.sym = SDLK_a;
        CHECK_FALSE(gui.process_raw_input_event(&event));
    }
    TEST_CASE("batch7678 DLL owned context opens and tracks load dialog") {
        Batch7678OpenGLCoreContext context; REQUIRE(context.ready);
        Salix::OpenGLImGui gui;
        REQUIRE(gui.open_load_file_dialog("Load7678", "Load", ".txt", "."));
        CHECK(gui.is_dialog_open("Load7678"));
    }
}

namespace {
struct Batch7778OpenGLImGuiFixture {
    std::filesystem::path previous=std::filesystem::current_path();
    bool had=false;std::string old;bool sdl_ready=false;
    Salix::OpenGLRenderer renderer;
    Salix::OpenGLImGui gui;
    Salix::ApplicationConfig config;
    MockIEventPoller poller;
    Salix::EventManager events;
    bool renderer_ready=false;
    bool gui_ready=false;
    Batch7778OpenGLImGuiFixture(){
        auto p=std::filesystem::absolute(std::filesystem::path(__FILE__)).parent_path();
        while(p.has_parent_path()&&!std::filesystem::exists(p/"Assets"/"Shaders"))p=p.parent_path();
        if(std::filesystem::exists(p/"Assets"/"Shaders"))std::filesystem::current_path(p);
        if(const char* v=SDL_getenv("SDL_VIDEODRIVER")){had=true;old=v;}
        SDL_Quit();SDL_setenv("SDL_VIDEODRIVER","windows",1);sdl_ready=SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)==0;
        if(!sdl_ready)return;
        Salix::WindowConfig wc;wc.title="Batch7778 OpenGLImGui";wc.width=128;wc.height=96;
        renderer_ready=renderer.initialize(wc); if(!renderer_ready)return;
        config.renderer_type=Salix::RendererType::OpenGL;config.window_config=wc;gui.set_app_config(&config);
        gui_ready=gui.initialize(renderer.get_window(),&renderer,nullptr,nullptr,nullptr);
    }
    ~Batch7778OpenGLImGuiFixture(){
        if(gui_ready)gui.shutdown(); renderer.shutdown(); SDL_Quit();
        if(had)SDL_setenv("SDL_VIDEODRIVER",old.c_str(),1);else _putenv_s("SDL_VIDEODRIVER","");
        std::error_code ec;std::filesystem::current_path(previous,ec);
    }
};
}

TEST_SUITE("Salix::gui::OpenGLImGuiBatch7778") {
    TEST_CASE("batch7778 real OpenGL ImGui initializes backend and context") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.renderer_ready); REQUIRE(f.gui_ready); CHECK(f.gui.get_context()!=nullptr);
    }
    TEST_CASE("batch7778 real OpenGL ImGui retains renderer and window interfaces") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); CHECK(f.gui.get_renderer()==&f.renderer); CHECK(f.gui.get_window()==f.renderer.get_window());
    }
    TEST_CASE("batch7778 real OpenGL ImGui new frame and render execute") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); CHECK_NOTHROW(f.gui.new_frame()); CHECK_NOTHROW(f.gui.render());
    }
    TEST_CASE("batch7778 real OpenGL ImGui registers raw event polling") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); f.gui.setup_event_polling(&f.poller,&f.events); CHECK(f.poller.raw_callback_count()==1u);
    }
    TEST_CASE("batch7778 real OpenGL ImGui raw SDL event reaches backend callback") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); f.gui.setup_event_polling(&f.poller,&f.events);
        SDL_Event e{};e.type=SDL_MOUSEMOTION;e.motion.x=7;e.motion.y=8;CHECK_NOTHROW(f.poller.emit_raw(&e));
    }
    TEST_CASE("batch7778 real OpenGL ImGui reinitializes backend") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); CHECK_NOTHROW(f.gui.reinitialize_backend());
    }
    TEST_CASE("batch7778 real OpenGL ImGui platform window update is safe with default flags") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); CHECK_NOTHROW(f.gui.update_and_render_platform_windows());
    }
    TEST_CASE("batch7778 real OpenGL ImGui opens save dialog") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); REQUIRE(f.gui.open_save_file_dialog("GLSave7778","Save",".txt",".","a.txt")); CHECK(f.gui.is_dialog_open("GLSave7778"));
    }
    TEST_CASE("batch7778 real OpenGL ImGui opens folder dialog") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); REQUIRE(f.gui.open_folder_dialog("GLFolder7778","Folder",".")); CHECK(f.gui.is_dialog_open("GLFolder7778"));
    }
    TEST_CASE("batch7778 real OpenGL ImGui dispatches input event during frame") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); f.gui.setup_event_polling(&f.poller,&f.events);
        f.gui.new_frame(); f.gui.render(); CHECK_FALSE(f.events.is_queue_empty()); f.events.process_queue(); CHECK(f.events.is_queue_empty());
    }
}
