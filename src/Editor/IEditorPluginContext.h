// src/Editor/IEditorPluginContext.h
#pragma once
#include <Salix/core/Core.h> // For EDITOR_API (since it's part of the Editor module's public API)

// Forward declarations for services that editor plugins might need
namespace Salix {
    // These interfaces (IGui, IThemeManager, IFontManager, etc.)
    // are part of the SALIX_API (core engine).
    class IGui;
    class IThemeManager;
    class IFontManager;
    class IAssetManager; // Assuming this is a SALIX_API interface
    class ISceneManager; // Assuming this is a SALIX_API interface
    // ...
}

namespace Salix {

    // This interface provides access to editor-specific services for editor plugins.
    // It is part of the EDITOR_API so that user-defined editor plugin DLLs can link to it.
    class EDITOR_API IEditorPluginContext { // Using EDITOR_API here
    public:
        virtual ~IEditorPluginContext() = default;

        // --- Core Editor Services ---
        virtual IGui* get_gui_system() const = 0;
        virtual IThemeManager* get_theme_manager() const = 0;
        virtual IFontManager* get_font_manager() const = 0;

        // --- Other potentially useful services for editor plugins (from core engine) ---
        virtual IAssetManager* get_asset_manager() const = 0;
        virtual ISceneManager* get_scene_manager() const = 0;
        // Add more getters for other engine services that editor plugins might need.
    };

} // namespace Salix