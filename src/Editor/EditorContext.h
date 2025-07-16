// =================================================================================
// Filename:    Editor/EditorContext.h
// Author:      SalixGameStudio
// Description: Defines the shared context for the editor. This struct holds
//              pointers to all relevant engine and editor systems that a
//              panel or editor tool might need to access.
// =================================================================================
#pragma once
#include <Editor/EditorAPI.h>

namespace Salix {

    // --- Forward Declarations ---
    // We use forward declarations to keep this header lightweight and avoid
    // circular dependencies.

    // Engine Systems (from InitContext)
    class IGui;
    class IRenderer;
    class AssetManager;
    class EventManager;
    class ITheme;
    class IThemeManager;
    class IFontManager;

    // Editor Systems
    class PanelManager;
    class ProjectManager;

    // Game World Data
    class Scene;
    class Entity;

    // --- The Editor Context Struct ---

    struct EDITOR_API EditorContext {
        // Pointers to core engine services, borrowed from InitContext
        IGui* gui = nullptr;
        IRenderer* renderer = nullptr;
        AssetManager* asset_manager = nullptr;
        EventManager* event_manager = nullptr;
        ITheme* active_theme = nullptr;
        IThemeManager* theme_manager = nullptr;
        IFontManager* font_manager = nullptr;
        // Pointers to editor-specific manager classes
        PanelManager* panel_manager = nullptr;
        ProjectManager* project_manager = nullptr;

        // Pointers to the active game world data
        Scene* active_scene = nullptr;
        Entity* selected_entity = nullptr; // The entity currently selected in the World Tree
    };

} // namespace Salix