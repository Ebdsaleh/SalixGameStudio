#pragma once
// ImGuizmo enables ImGui math operators; define them before any transitive imgui.h include.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ArchetypeFixtures.h"
#include <Editor/EditorContext.h>
#include <Editor/EditorAPI.h>
#include <Salix/core/InitContext.h>
#include <Salix/events/EventManager.h>
#include <Salix/input/ImGuiInputManager.h>
#include <Salix/gui/IconInfo.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/imgui/sdl/SDLImGui.h>
#include <Salix/rendering/sdl/SDLRenderer.h>
#include <Salix/rendering/ITexture.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <imgui/imgui_internal.h>
#include <iterator>

namespace EditorCri {
    // No SDL initialization, native windows or GPU resources.
    struct GuiFrame {
        ImGuiContext* previous = ImGui::GetCurrentContext();
        ImGuiContext* previous_engine = ImGuiInputManager::get_context_for_testing();
        ImGuiContext* current = ImGui::CreateContext();
        GuiFrame() {
            ImGui::SetCurrentContext(current);
            set_imgui_context(current);
            ImGuiInputManager::set_context_for_testing(current);
            auto& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.LogFilename = nullptr;
            io.DisplaySize = ImVec2(1280,720);
            io.DeltaTime = 1.0f/60;
            unsigned char* pixels=nullptr; int w=0,h=0;
            io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h);
            io.Fonts->SetTexID(ImTextureID(1));
        }
        ~GuiFrame() {
            ImGuiInputManager::set_context_for_testing(previous_engine);
            set_imgui_context(previous);
            ImGui::DestroyContext(current);
            ImGui::SetCurrentContext(previous);
        }
        GuiFrame(const GuiFrame&) = delete;
        GuiFrame& operator=(const GuiFrame&) = delete;
        template<class F> int draw(F action) {
            ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(20,20),ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(800,600),ImGuiCond_Always);
            ImGui::Begin("CRI002 offscreen host",nullptr,ImGuiWindowFlags_NoSavedSettings);
            const int before=ImGui::GetWindowDrawList()->VtxBuffer.Size;
            action();
            const int added=ImGui::GetWindowDrawList()->VtxBuffer.Size-before;
            ImGui::End();
            ImGui::Render();
            return added;
        }
        template<class F> void frame(F action) {
            ImGui::NewFrame();
            action();
            ImGui::Render();
        }
    };
    struct IconProbe : ImGuiIconManager {
        IconInfo unlocked, locked, missing;
        std::map<std::string,IconInfo> registry;
        const std::map<std::string,IconInfo>& get_icon_registry() override {return registry;}
        IconProbe() { unlocked.path="unlocked.png"; locked.path="locked.png"; unlocked.texture_id=ImTextureID(2); locked.texture_id=ImTextureID(3); }
        const IconInfo& get_icon_by_name(const std::string& n) const override {
            return n=="Panel Unlocked"?unlocked:n=="Panel Locked"?locked:missing;
        }
    };
    struct GuiProbe : SDLImGui {
        int renders=0, platforms=0, dialog_frames=0, theme_reloads=0;
        std::string shown_dialog;
        void render() override {++renders;}
        void update_and_render_platform_windows() override {++platforms;}
        void display_dialogs() override {++dialog_frames;}
        void request_theme_reload() override {++theme_reloads;}
        void show_dialog_by_key(std::string& key) override {shown_dialog=key;}
        Salix::IIconManager* icons=nullptr;
        Salix::IIconManager* get_icon_manager() override { return icons; }
    };
    struct FixtureTexture : ITexture {
        int get_width() const override {return 80;}
        int get_height() const override {return 40;}
        ImTextureID get_imgui_texture_id() const override {return ImTextureID(7);}
    };
    struct SpriteDraw { ITexture* texture; glm::mat4 matrix; Color color; };
    struct RenderProbe : SDLRenderer {
        float ppu=100.0f;
        ImTextureID framebuffer_texture=0;
        std::vector<SpriteDraw> sprites;
        int texture_loads=0;
        ITexture* load_texture(const char*) override {++texture_loads;return new FixtureTexture;}
        float get_pixels_per_unit() const override {return ppu;}
        void draw_sprite(ITexture* t,const glm::mat4& m,const Color& c) override {sprites.push_back({t,m,c});}
        uint32_t result=73, last_id=0;
        int creates=0, deletes=0, begins=0, ends=0, clears=0, width=0,height=0;
        struct WireBoxDraw { glm::mat4 matrix; Color color; };
        std::vector<WireBoxDraw> wire_boxes;
        void draw_wire_box(const glm::mat4& m, const Color& c) override { wire_boxes.push_back({m,c}); }
        int lines=0;
        void draw_line(const glm::vec3&,const glm::vec3&,const Color&) override { ++lines; }
        ICamera* camera=nullptr;
        uint32_t create_framebuffer(int w,int h) override { ++creates;width=w;height=h;return result; }
        void delete_framebuffer(uint32_t) override { ++deletes; }
        ImTextureID get_framebuffer_texture_id(uint32_t) override { return framebuffer_texture; }
        void begin_render_pass(uint32_t id) override { ++begins;last_id=id; }
        void end_render_pass() override { ++ends; }
        void set_viewport(int,int,int w,int h) override { width=w;height=h; }
        void clear() override { ++clears; }
        void set_active_camera(ICamera* c) override { camera=c; }
        ICamera* get_active_camera() override { return camera; }
    };
    inline Sprite2D* add_textured_sprite(Realm& realm, AssetManager& assets, uint64_t value, const char* path) {
        auto* e=realm.create_entity(id(value),"Sprite fixture");
        auto* s=e->add_element<Sprite2D>();
        REQUIRE(s);
        s->load_texture(&assets,path);
        REQUIRE(s->get_texture());
        s->pivot=Vector2(.5f,.5f);
        return s;
    }
    struct PanelContext {
        IconProbe icons; GuiProbe gui; RenderProbe renderer; EventManager events;
        InitContext init{}; EditorContext ctx;
        PanelContext() {
            register_types();
            gui.icons=&icons;
            init.icon_manager=&icons; init.renderer=&renderer; init.gui=&gui; init.event_manager=&events;
            ctx.init_context=&init; ctx.gui=&gui; ctx.renderer=&renderer; ctx.event_manager=&events;
            ctx.editor_realm_manager=std::make_unique<EditorRealmManager>(&ctx);
        }
    };
    struct ScopedWorkingDir {
        std::filesystem::path previous=std::filesystem::current_path();
        explicit ScopedWorkingDir(const TempRealm& t) { std::filesystem::current_path(std::filesystem::path(t.path()).parent_path()); }
        ~ScopedWorkingDir() { std::error_code ec; std::filesystem::current_path(previous,ec); }
    };
    inline std::string read_text(const std::string& path) {
        std::ifstream in(path,std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>());
    }
}
