// Salix/core/ApplicationConfig.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/window/WindowConfig.h>
#include <Salix/core/InitEnums.h>

namespace Salix {

    struct ApplicationConfig { 
        WindowConfig window_config;
        RendererType renderer_type = RendererType::SDL;
        AppStateType initial_state = AppStateType::Launch;
        GuiType gui_type = GuiType::ImGui;
        TimerType timer_type = TimerType::SDL;
        int target_fps = 60;


    };
}