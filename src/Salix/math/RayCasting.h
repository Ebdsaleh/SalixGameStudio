// Salix/math/RayCasting.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/rendering/ICamera.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Forward declare ImGui's 2D vector type to avoid including imgui.h
struct ImVec2;

namespace Salix {
    

    // A simple struct to represent a 3D ray
    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    // A static utility class for raycasting and picking operations
    class SALIX_API Raycast {
    public:
        // --- CORE ENGINE METHOD ---
        // Creates a world-space ray. This is the primary, engine-facing function.
        static Ray CreateRayFromScreen(
            ICamera* camera,
            const Vector2& mouse_pos,
            const Vector2& viewport_pos,
            const Vector2& viewport_size
        );

        // --- EDITOR CONVENIENCE OVERLOAD ---
        // An overload for use with ImGui that wraps the core engine function.
        // This keeps the ImGui dependency out of the main engine API.
        static Ray CreateRayFromScreen(
            ICamera* camera,
            const ImVec2& mouse_pos,
            const ImVec2& viewport_pos,
            const ImVec2& viewport_size
        );


        // Checks for intersection between a ray and an Axis-Aligned Bounding Box (AABB).
        // Returns true if there is an intersection and outputs the distance to the hit.
        static bool IntersectsAABB(

            const Ray& ray,
            const glm::vec3& box_min,
            const glm::vec3& box_max,
            float& distance
        );


        static bool IntersectsAABB(
            const Ray& ray,
            const Vector3& box_min,
            const Vector3& box_max,
            float& distance
        );

        static bool IntersectsOBB(
            const Ray& ray,
            const glm::mat4& model_matrix,
            const glm::vec3& half_extents,
            float& out_distance
        );

        
    };
    

} // namespace Salix