// Salix/gui/IGui.h
#pragma once
#include <Salix/core/Core.h> // For SALIX_API
#include <string> // For std::string
#include <imgui/imgui.h> // Only include ImGui's core header if SALIX_GUI_IMGUI is defined


namespace Salix {
    // Forward declarations for engine interfaces that a GUI might need to interact with
    class IWindow;
    class IRenderer;
    class IThemeManager;
    class IFontManager;
    class SALIX_API IGui {
    public:
        virtual ~IGui() = default;

        // Initializes the GUI system.
        // The concrete implementation will cast the 'window' and 'renderer'
        // to their specific types (e.g., SDLWindow*, SDLRenderer*) internally.
        virtual bool initialize(
            IWindow* window, IRenderer* renderer,
            IThemeManager* theme_manager, IFontManager* font_manager) = 0;

        // Shuts down the GUI system and cleans up resources.
        virtual void shutdown() = 0;

        // Starts a new GUI frame.
        // This method should be called once per frame before any UI elements are built.
        // It handles input processing for the GUI.
        virtual void new_frame() = 0;

        // Renders the GUI elements that have been built for the current frame.
        // This method should be called after all UI building calls (e.g., ImGui::Begin/End)
        // have been made for the current frame.
        virtual void render() = 0;

        // Handles updates and rendering for platform-specific GUI windows (e.g., docking viewports).
        // This abstracts away multi-window/multi-viewport management if the GUI library supports it.
        virtual void update_and_render_platform_windows() = 0;

        // --- Basic GUI Configuration / Persistence ---

        // Controls the visibility of the GUI's mouse cursor.
        virtual void set_mouse_cursor_visible(bool visible) = 0;
        
        virtual IFontManager* get_font_manager() = 0;
        // Saves the current GUI layout/configuration to a file.
        virtual void save_layout(const std::string& file_path) = 0;

        // Loads a GUI layout/configuration from a file.
        virtual void load_layout(const std::string& file_path) = 0;
    };
}