// Salix/core/InitEnums.h
#pragma once

namespace Salix {

  // An enum for declaring the type of renderer in use.
    enum class RendererType {
        None, // initial setting, usually won't ever be set to this unless there's a specific reason.
        SDL, // The default option
        OpenGL, // OpenGl using the SDL API
        Vulkan,  // For the future
        DirectX, // For the future
    };

    enum class TimerType {
        SDL,
        Chrono
    };
    enum class GuiType {
        None,       //  For GameState inside user projects executable.
        ImGui,      // DearImGui
    };

     enum class AppStateType {
        None,
        Launch,
        Editor,
        Game,
        Options
    };
}  // namespace Salix