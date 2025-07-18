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
    struct InitContext;
    class IGui;
    class IRenderer;
    class AssetManager;
    class EventManager;
    class ITheme;
    class IThemeManager;
    class IFontManager;
    class SceneManager;

    // Editor Systems
    class PanelManager;
    class ProjectManager;

    // Game World Data
    class Project;
    class Scene;
    class Entity;
    class Element;
    class EditorCamera;
    class Camera;

    // --- The Editor Context Struct ---

    struct EDITOR_API EditorContext {
        // Pointers to core engine services, borrowed from InitContext
        const InitContext* init_context  = nullptr;
        IGui* gui = nullptr;
        IRenderer* renderer = nullptr;
        AssetManager* asset_manager = nullptr;
        EventManager* event_manager = nullptr;
        ITheme* active_theme = nullptr;
        IThemeManager* theme_manager = nullptr;
        IFontManager* font_manager = nullptr;
        SceneManager* scene_manager = nullptr;
        // Pointers to editor-specific manager classes
        PanelManager* panel_manager = nullptr;
        ProjectManager* project_manager = nullptr;
        EditorCamera* editor_camera = nullptr;
        // Pointers to the active game world data
        Project* active_project = nullptr;
        Scene* active_scene = nullptr;
        Camera* main_camera = nullptr;
        Element* selected_element = nullptr;
        Entity* selected_entity = nullptr;
        
        
    };

} // namespace Salix