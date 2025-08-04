// Editor/SceneSettings.h
#pragma once
#include <Editor/EditorAPI.h>
#include <algorithm>

namespace Salix {

    struct EDITOR_API SceneSettings {
        bool use_scene_bounds = true;  // Default to bounded
        float scene_size = 1000.0f;    // Default boundary size
        
        // constexpr constructor
        constexpr SceneSettings(bool use_bounds = true, float size = 1000.0f) :
            use_scene_bounds(use_bounds),
            scene_size((size < 1.0f) ? 1.0f : size) // Basic constexpr validation
        {}

        // Runtime validation for when settings change
        void validate() {
            scene_size = std::max(1.0f, scene_size);
        }

        // Simplified static default - returns default-constructed object
        static constexpr SceneSettings Default() {
            return SceneSettings(); // Uses constructor defaults
        }

        // Optional: Add comparison operators if needed
        constexpr bool operator==(const SceneSettings& other) const {
            return use_scene_bounds == other.use_scene_bounds &&
                   nearly_equal(scene_size, other.scene_size);
        }
        
        constexpr bool operator!=(const SceneSettings& other) const {
            return !(*this == other);
        }

    private:
        // Helper for constexpr float comparison
        static constexpr bool nearly_equal(float a, float b, float epsilon = 0.0001f) {
            return (a > b ? a - b : b - a) <= epsilon;
        }
    };

} // namespace Salix