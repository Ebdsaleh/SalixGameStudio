// =================================================================================
// Filename:    Editor/EditorContext.h
// Author:      SalixGameStudio
// Description: Defines the shared context for the editor. This struct holds
//              pointers to all relevant engine and editor systems that a
//              panel or editor tool might need to access.
// =================================================================================
#pragma once
#include <yaml-cpp/yaml.h> 
#include <Editor/EditorAPI.h>
#include <Editor/GridSettings.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/reflection/EditorDataMode.h>

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
    struct SceneSettings;
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
        GridSettings grid_settings; 
        SceneSettings scene_settings;
        SimpleGuid selected_entity_id = SimpleGuid::invalid();
        SimpleGuid selected_element_id = SimpleGuid::invalid();
        EditorDataMode data_mode = EditorDataMode::Live;
        YAML::Node active_yaml_scene;
        EditorContext() : grid_settings(20.0f, 1.0f, 4, true, 0.25f, {0.3f, 0.3f, 0.3f, 0.4f}){}
    };
    

} // namespace Salix