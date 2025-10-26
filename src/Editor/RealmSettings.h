// Editor/RealmSettings.h
#pragma once
#include <Editor/EditorAPI.h>
#include <algorithm>

namespace Salix {

    struct EDITOR_API RealmSettings {
        bool use_realm_bounds = true;  // Default to bounded
        float realm_size = 1000.0f;    // Default boundary size
        
        // constexpr constructor
        constexpr RealmSettings(bool use_bounds = true, float size = 1000.0f) :
            use_realm_bounds(use_bounds),
            realm_size((size < 1.0f) ? 1.0f : size) // Basic constexpr validation
        {}

        // Runtime validation for when settings change
        void validate() {
            realm_size = std::max(1.0f, realm_size);
        }

        // Simplified static default - returns default-constructed object
        static constexpr RealmSettings Default() {
            return RealmSettings(); // Uses constructor defaults
        }

        // Optional: Add comparison operators if needed
        constexpr bool operator==(const RealmSettings& other) const {
            return use_realm_bounds == other.use_realm_bounds &&
                   nearly_equal(realm_size, other.realm_size);
        }
        
        constexpr bool operator!=(const RealmSettings& other) const {
            return !(*this == other);
        }

    private:
        // Helper for constexpr float comparison
        static constexpr bool nearly_equal(float a, float b, float epsilon = 0.0001f) {
            return (a > b ? a - b : b - a) <= epsilon;
        }
    };

} // namespace Salix