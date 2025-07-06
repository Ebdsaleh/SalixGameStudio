// Editor/EditorContext.h
#pragma once
#include <Salix/core/Core.h>

namespace Salix {
    class IGui;
    class ITheme;
    class IThemeManager;
    class IFontManager;
    

    struct EDITOR_API EditorContext {
        IGui* gui = nullptr;
        ITheme* active_theme = nullptr;
        IThemeManager* theme_manager = nullptr;
        IFontManager* font_manager = nullptr;
    
    };
}