// Salix/gui/imgui/SDLImGui.h
#pragma once

#include <Salix/gui/IGui.h>
#include <imgui/imgui.h>
#include <memory>
// New includes needed for the setup_event_polling method
#include <Salix/events/IEventPoller.h> 
#include <Salix/events/EventManager.h> // For dispatching ImGuiInputEvent
#include <Salix/events/ImGuiInputEvent.h> // For the event definition

// Forward declarations for specific SDL types if your Pimpl needs them directly
// struct SDL_Window; // Already handled by SDLWindow.h's native handle
// struct SDL_Renderer; // Already handled by SDLRenderer.h's native handle

namespace Salix {
    
    class SALIX_API SDLImGui : public IGui {
    public:
        SDLImGui();
        ~SDLImGui() override; // Ensure override is present for virtual destructor
        bool initialize(IWindow* window, IRenderer* renderer) override;
        void shutdown() override;
        void new_frame() override;
        void render() override;

        void update_and_render_platform_windows() override;
        void set_mouse_cursor_visible(bool visible) override;
        void save_layout(const std::string& file_path) override;
        void load_layout(const std::string& file_path) override;

        // **NEW METHOD:** Specific method to set up event system dependencies.
        // This is NOT part of the IGui interface.
        void setup_event_polling(IEventPoller* event_poller, EventManager* event_manager);

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
} // namespace Salix