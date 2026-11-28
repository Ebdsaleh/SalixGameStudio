// Editor/GridSettings.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Salix/math/Color.h>
#include <Editor/RealmSettings.h>
#include <algorithm>
#include <limits>

namespace Salix {
    struct EDITOR_API GridSettings {
        float size;
        float major_division;
        float minor_division;
        bool snap_enabled;
        float snap_size;
        Color color;

    private:
        // A non-finite major interval cannot be a useful grid spacing.
        static constexpr float normalize_major(float value) {
            if (!(value <= std::numeric_limits<float>::max()) ||
                value < -std::numeric_limits<float>::max()) return 1.0f;
            return std::max(0.1f, value);
        }

    public:
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
            // Normalize the major spacing before using it as a bound.
            // Retain the constructor's half-major fallback for oversized minor spacing.
            size = std::clamp(sz == sz ? sz : 20.0f, 1.0f, 1000.0f);
            major_division = normalize_major(major);
            const float minor_value = minor == minor ? minor : 0.25f;
            minor_division = (minor_value < 0.01f) ? 0.01f :
                (minor_value > major_division * 0.9f) ? major_division * 0.5f : minor_value;
            snap_size = std::clamp(snap_sz == snap_sz ? snap_sz : 0.25f, 0.01f, major_division);
        }

        // Runtime validation for when settings change
        void validate() {
            size = std::clamp(size == size ? size : 20.0f, 1.0f, 1000.0f);
            major_division = normalize_major(major_division);

            const float max_minor = major_division * 0.9f;
            minor_division = std::clamp(minor_division == minor_division ? minor_division : 0.25f,
                                        0.01f, max_minor);
            snap_size = std::clamp(snap_size == snap_size ? snap_size : 0.25f,
                                   0.01f, major_division);
        }

        float get_effective_size(const RealmSettings& realm) const {
            return realm.use_realm_bounds ? size : std::numeric_limits<float>::max();
        }
        // Simplified static default - now just returns default-constructed object
        static constexpr GridSettings Default() {
            return GridSettings(); // Uses the constructor's defaults
        }
    };
}