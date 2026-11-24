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
#include <Salix/gui/IconInfo.h>
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <Salix/gui/imgui/sdl/SDLImGui.h>
#include <Salix/rendering/sdl/SDLRenderer.h>
#include <imgui/imgui_internal.h>
#include <iterator>

namespace EditorCri {
    // No SDL initialization, native windows or GPU resources.
    struct GuiFrame {
        ImGuiContext* previous = ImGui::GetCurrentContext();
        ImGuiContext* current = ImGui::CreateContext();
        GuiFrame() {
            ImGui::SetCurrentContext(current);
            set_imgui_context(current);
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
        IconProbe() { unlocked.path="unlocked.png"; locked.path="locked.png"; unlocked.texture_id=ImTextureID(2); locked.texture_id=ImTextureID(3); }
        const IconInfo& get_icon_by_name(const std::string& n) const override {
            return n=="Panel Unlocked"?unlocked:n=="Panel Locked"?locked:missing;
        }
    };
    struct GuiProbe : SDLImGui {
        Salix::IIconManager* icons=nullptr;
        Salix::IIconManager* get_icon_manager() override { return icons; }
    };
    struct RenderProbe : SDLRenderer {
        uint32_t result=73, last_id=0;
        int creates=0, deletes=0, begins=0, ends=0, clears=0, width=0,height=0;
        ICamera* camera=nullptr;
        uint32_t create_framebuffer(int w,int h) override { ++creates;width=w;height=h;return result; }
        void delete_framebuffer(uint32_t) override { ++deletes; }
        ImTextureID get_framebuffer_texture_id(uint32_t) override { return ImTextureID(0); }
        void begin_render_pass(uint32_t id) override { ++begins;last_id=id; }
        void end_render_pass() override { ++ends; }
        void set_viewport(int,int,int w,int h) override { width=w;height=h; }
        void clear() override { ++clears; }
        void set_active_camera(ICamera* c) override { camera=c; }
        ICamera* get_active_camera() override { return camera; }
    };
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
