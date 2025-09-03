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
#include <Salix/ecs/Scene.h>
#include <Editor/panels/WorldTreeNode.h>
#include <Salix/reflection/EditorDataMode.h>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <functional>
#include <Editor/Archetypes.h>
#include <Editor/management/EditorRealmManager.h>
#include <Editor/management/RealmSnapshot.h>

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
    struct EntityArchetype;
    
    // Game World Data
    class Project;
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
        Scene* active_scene = nullptr; // used for RealmPortalPanel for future play in editor functionality.
        std::unique_ptr<Scene> preview_scene;
        Camera* main_camera = nullptr;
        Element* selected_element = nullptr;
        Entity* selected_entity = nullptr;
        GridSettings grid_settings; 
        SceneSettings scene_settings;
        SimpleGuid selected_entity_id = SimpleGuid::invalid();
        SimpleGuid selected_element_id = SimpleGuid::invalid();
        EditorDataMode data_mode = EditorDataMode::Yaml;
        std::unique_ptr<EditorRealmManager> editor_realm_manager;
        //A queue for commands to be run at the end of the frame.
        std::vector<std::function<void()>> deferred_commands;
        std::vector<std::function<void()>> sync_queue;
        bool realm_is_dirty = true; 
        bool is_editing_property = false;
        EditorContext() : grid_settings(20.0f, 1.0f, 4, true, 0.25f, {0.3f, 0.3f, 0.3f, 0.4f}){}
        // This is the centralized way to add deferred commands.
        void add_deferred_command(std::function<void()> command) {
            deferred_commands.push_back(std::move(command));
        }
        void clear_deferred_commands() {
            deferred_commands.clear();
        }
        bool is_deferred_commands_empty() {
            return deferred_commands.empty();
        }
    };
    

} // namespace Salix