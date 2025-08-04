// Editor/GridSettings.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Salix/math/Color.h>
#include <Editor/SceneSettings.h>
#include <algorithm>

namespace Salix {
    struct EDITOR_API GridSettings {
        float size;
        float major_division;
        float minor_division;
        bool snap_enabled;
        float snap_size;
        Color color;

        // constexpr constructor - now properly constexpr compatible
        constexpr GridSettings(float sz = 20.0f, 
                             float major = 1.0f,
                             float minor = 0.25f,
                             bool snap = true,
                             float snap_sz = 0.25f,
                             Color clr = {0.3f, 0.3f, 0.3f, 0.4f}) :
            size(sz),
            major_division(major),
            minor_division(minor),
            snap_enabled(snap),
            snap_size(snap_sz),
            color(clr) 
        {
            // Simple validation that can be constexpr
            size = (sz < 1.0f) ? 1.0f : (sz > 1000.0f) ? 1000.0f : sz;
            major_division = (major < 0.1f) ? 0.1f : major;
            minor_division = (minor < 0.01f) ? 0.01f : 
                           (minor > (major * 0.9f)) ? (major * 0.5f) : minor;
            snap_size = (snap_sz < 0.01f) ? 0.01f : 
                       (snap_sz > major) ? major : snap_sz;
        }

        // Runtime validation for when settings change
        void validate() {
            // Use runtime versions of clamping
            size = std::clamp(size, 1.0f, 1000.0f);
            major_division = std::max(0.1f, major_division);
            
            const float max_minor = major_division * 0.9f;
            minor_division = std::clamp(minor_division, 0.01f, max_minor);
            
            snap_size = std::clamp(snap_size, 0.01f, major_division);
            
            if (minor_division >= major_division) {
                minor_division = major_division * 0.5f;
            }
        }

        float get_effective_size(const SceneSettings& scene) const {
            return scene.use_scene_bounds ? size : std::numeric_limits<float>::max();
        }
        // Simplified static default - now just returns default-constructed object
        static constexpr GridSettings Default() {
            return GridSettings(); // Uses the constructor's defaults
        }
    };
}