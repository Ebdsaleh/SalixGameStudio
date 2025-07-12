// Salix/core/ApplicationConfig.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/window/WindowConfig.h>
#include <Salix/core/InitEnums.h>

namespace Salix {

    // A struct to hold GUI-related settings that can be configured.
    struct GuiSettings {
        // Defines dialog size as a percentage of the main window's dimensions.
        float dialog_width_ratio = 0.7f;  // Default to 70% of window width
        float dialog_height_ratio = 0.75f; // Default to 75% of window height
    };

    struct ApplicationConfig { 
        WindowConfig window_config;
        RendererType renderer_type = RendererType::SDL;
        AppStateType initial_state = AppStateType::Launch;
        GuiType gui_type = GuiType::ImGui;
        TimerType timer_type = TimerType::SDL;
        int target_fps = 60;
        GuiSettings gui_settings;


    };
}