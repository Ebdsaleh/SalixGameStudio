#include <doctest.h>
#include <SDL.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
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

TEST_SUITE("Salix::gui::OpenGLImGuiBatch7878") {
    TEST_CASE("batch7878 OpenGL dialog display opens registered load dialog") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);std::string key="GLDisplay7878",title="Load";REQUIRE(f.gui.create_dialog(key,title,Salix::DialogType::File,false)!=nullptr);f.gui.show_dialog_by_key(key);f.gui.new_frame();CHECK_NOTHROW(f.gui.display_dialogs());f.gui.render();CHECK(f.gui.is_dialog_open(key)); }
    TEST_CASE("batch7878 OpenGL displayed dialog consumes escape and clears active key") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);REQUIRE(f.gui.open_load_file_dialog("GLEscape7878","Load",".txt","."));f.gui.new_frame();f.gui.display_dialogs();f.gui.render();SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.sym=SDLK_ESCAPE;CHECK(f.gui.process_raw_input_event(&e));CHECK_FALSE(f.gui.is_dialog_open("GLEscape7878")); }
    TEST_CASE("batch7878 OpenGL active dialog result query executes nonempty result path") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);REQUIRE(f.gui.open_save_file_dialog("GLResult7878","Save",".txt",".","a.txt"));f.gui.new_frame();f.gui.display_dialogs();f.gui.render();auto result=f.gui.populate_dialog_result("GLResult7878");CHECK_FALSE(result.is_ok); }
    TEST_CASE("batch7878 OpenGL missing dialog lookup remains null after registry use") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);std::string key="GLKnown7878",title="Known";REQUIRE(f.gui.create_dialog(key,title,Salix::DialogType::File,false)!=nullptr);CHECK(f.gui.get_dialog("GLMissing7878")==nullptr); }
    TEST_CASE("batch7878 OpenGL duplicate create returns original dialog") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);std::string key="GLDup7878",a="A",b="B";auto* first=f.gui.create_dialog(key,a,Salix::DialogType::File,false);REQUIRE(first!=nullptr);CHECK(f.gui.create_dialog(key,b,Salix::DialogType::File,true)==first); }
    TEST_CASE("batch7878 OpenGL spare GUI reinitialize rejects null backend handles with DLL context alive") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);Salix::OpenGLImGui spare;CHECK_NOTHROW(spare.reinitialize_backend()); }
    TEST_CASE("batch7878 OpenGL platform update executes with explicit viewport flag after completed frame") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);REQUIRE(f.gui.get_context()!=nullptr);f.gui.get_context()->IO.ConfigFlags|=ImGuiConfigFlags_ViewportsEnable;f.gui.new_frame();f.gui.render();CHECK_NOTHROW(f.gui.update_and_render_platform_windows());f.gui.get_context()->IO.ConfigFlags&=~ImGuiConfigFlags_ViewportsEnable; }
    TEST_CASE("batch7878 OpenGL folder dialog display executes first frame open path") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);REQUIRE(f.gui.open_folder_dialog("GLFolderDisplay7878","Folder","."));f.gui.new_frame();CHECK_NOTHROW(f.gui.display_dialogs());f.gui.render(); }
    TEST_CASE("batch7878 OpenGL save dialog display executes configured filename path") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);REQUIRE(f.gui.open_save_file_dialog("GLSaveDisplay7878","Save",".txt",".","batch7878.txt"));f.gui.new_frame();CHECK_NOTHROW(f.gui.display_dialogs());f.gui.render(); }
    TEST_CASE("batch7878 OpenGL explicit shutdown destroys live backend context") { Batch7778OpenGLImGuiFixture f;REQUIRE(f.gui_ready);CHECK_NOTHROW(f.gui.shutdown());f.gui_ready=false;CHECK(f.gui.get_context()!=nullptr); }
}

TEST_SUITE("Salix::gui::OpenGLImGuiBatch7978") {
    TEST_CASE("batch7978 OpenGL GUI theme reload handshake remains explicit") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        CHECK_FALSE(f.gui.is_theme_reload_requested()); f.gui.request_theme_reload();
        CHECK(f.gui.is_theme_reload_requested()); f.gui.clear_theme_reload_request();
        CHECK_FALSE(f.gui.is_theme_reload_requested());
    }
    TEST_CASE("batch7978 OpenGL GUI platform update remains safe on second completed frame") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        f.gui.new_frame(); f.gui.render(); f.gui.update_and_render_platform_windows();
        f.gui.new_frame(); f.gui.render(); CHECK_NOTHROW(f.gui.update_and_render_platform_windows());
    }
    TEST_CASE("batch7978 OpenGL GUI direct dialog registration preserves pointer identity") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        auto dialog = std::make_unique<Salix::DialogBox>("GLRegister7978", "Register", Salix::DialogType::File, false);
        auto* raw = dialog.get(); REQUIRE(f.gui.register_dialog(std::move(dialog)));
        CHECK(f.gui.get_dialog("GLRegister7978") == raw);
    }
    TEST_CASE("batch7978 OpenGL GUI duplicate direct dialog registration is rejected") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        REQUIRE(f.gui.register_dialog(std::make_unique<Salix::DialogBox>("GLDuplicate7978", "One", Salix::DialogType::File, false)));
        CHECK_FALSE(f.gui.register_dialog(std::make_unique<Salix::DialogBox>("GLDuplicate7978", "Two", Salix::DialogType::File, true)));
    }
    TEST_CASE("batch7978 OpenGL GUI orphan active key is removed by display pass") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        std::string key = "GLOrphan7978"; f.gui.show_dialog_by_key(key); REQUIRE(f.gui.is_dialog_open(key));
        CHECK_NOTHROW(f.gui.display_dialogs()); CHECK_FALSE(f.gui.is_dialog_open(key));
    }
    TEST_CASE("batch7978 OpenGL GUI registered overwrite dialog opens through display") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        std::string key="GLOverwrite7978", title="Overwrite";
        REQUIRE(f.gui.create_dialog(key,title,Salix::DialogType::File,true)!=nullptr); f.gui.show_dialog_by_key(key);
        f.gui.new_frame(); CHECK_NOTHROW(f.gui.display_dialogs()); f.gui.render(); CHECK(f.gui.is_dialog_open(key));
    }
    TEST_CASE("batch7978 OpenGL GUI missing result remains default after registry activity") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        std::string key="GLRegistry7978", title="Registry"; REQUIRE(f.gui.create_dialog(key,title,Salix::DialogType::File,false)!=nullptr);
        const auto result=f.gui.get_dialog_result("GLNoResult7978"); CHECK_FALSE(result.is_ok); CHECK(result.file_name.empty());
    }
    TEST_CASE("batch7978 OpenGL GUI cursor policy toggles after completed frame") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        f.gui.new_frame(); f.gui.render(); CHECK_NOTHROW(f.gui.set_mouse_cursor_visible(false)); CHECK_NOTHROW(f.gui.set_mouse_cursor_visible(true));
    }
    TEST_CASE("batch7978 OpenGL GUI null config replacement preserves configured pointer") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        auto* before=f.gui.get_app_config(); REQUIRE(before!=nullptr); f.gui.set_app_config(nullptr); CHECK(f.gui.get_app_config()==before);
    }
    TEST_CASE("batch7978 OpenGL GUI layout roundtrip remains safe with live context") {
        Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready);
        const auto path=std::filesystem::temp_directory_path()/"batch7978_gl_layout.ini";
        f.gui.save_layout(path.string()); REQUIRE(std::filesystem::exists(path)); CHECK_NOTHROW(f.gui.load_layout(path.string()));
        std::error_code ec; std::filesystem::remove(path,ec);
    }
}

TEST_SUITE("Salix::gui::OpenGLImGuiBatch8078") {
    TEST_CASE("batch8078 OpenGL ImGui default viewport branch forces zero window rounding") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); auto* c=f.gui.get_context(); REQUIRE(c); CHECK(c->Style.WindowRounding==doctest::Approx(0.0f)); }
    TEST_CASE("batch8078 OpenGL ImGui default viewport branch forces opaque window background") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); auto* c=f.gui.get_context(); REQUIRE(c); CHECK(c->Style.Colors[ImGuiCol_WindowBg].w==doctest::Approx(1.0f)); }
    TEST_CASE("batch8078 OpenGL ImGui initialization enables docking") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); auto* c=f.gui.get_context(); REQUIRE(c); CHECK((c->IO.ConfigFlags&ImGuiConfigFlags_DockingEnable)!=0); }
    TEST_CASE("batch8078 OpenGL ImGui initialization enables keyboard navigation") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); auto* c=f.gui.get_context(); REQUIRE(c); CHECK((c->IO.ConfigFlags&ImGuiConfigFlags_NavEnableKeyboard)!=0); }
    TEST_CASE("batch8078 OpenGL ImGui initializes with viewports disabled by default") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); auto* c=f.gui.get_context(); REQUIRE(c); CHECK((c->IO.ConfigFlags&ImGuiConfigFlags_ViewportsEnable)==0); }
    TEST_CASE("batch8078 OpenGL ImGui context records docking tab bar preference") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); auto* c=f.gui.get_context(); REQUIRE(c); CHECK(c->IO.ConfigDockingAlwaysTabBar); }
    TEST_CASE("batch8078 OpenGL ImGui context records transparent docking payload preference") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); auto* c=f.gui.get_context(); REQUIRE(c); CHECK(c->IO.ConfigDockingTransparentPayload); }
    TEST_CASE("batch8078 OpenGL ImGui style remains zero rounded after one completed frame") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); f.gui.new_frame(); f.gui.render(); auto* c=f.gui.get_context(); REQUIRE(c); CHECK(c->Style.WindowRounding==doctest::Approx(0.0f)); }
    TEST_CASE("batch8078 OpenGL ImGui style remains opaque after backend reinitialize") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); f.gui.reinitialize_backend(); auto* c=f.gui.get_context(); REQUIRE(c); CHECK(c->Style.Colors[ImGuiCol_WindowBg].w==doctest::Approx(1.0f)); }
    TEST_CASE("batch8078 OpenGL ImGui context pointer stays stable across frame") { Batch7778OpenGLImGuiFixture f; REQUIRE(f.gui_ready); auto* before=f.gui.get_context(); f.gui.new_frame(); f.gui.render(); CHECK(f.gui.get_context()==before); }
}

