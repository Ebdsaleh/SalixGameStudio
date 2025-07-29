// Salix/math/RayCasting.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <glm/glm.hpp>

// Forward declare ImGui's 2D vector type to avoid including imgui.h
struct ImVec2;

namespace Salix {
    
    // Forward declare the camera interface
    class ICamera;

    // A simple struct to represent a 3D ray
    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    // A static utility class for raycasting and picking operations
    class SALIX_API Raycast {
    public:
        // Creates a world-space ray from a camera and screen coordinates.
        static Ray CreateRayFromScreen(
            ICamera* camera,
            const ImVec2& mouse_pos,      // Mouse position in absolute screen coordinates
            const ImVec2& viewport_pos,   // Top-left corner of the viewport panel
            const ImVec2& viewport_size   // Width and height of the viewport panel
        );

        static Ray CreateRayFromScreen(
            ICamera* camera,
            const Vector2& mouse_pos,
            const Vector2& viewport_pos,
            const Vector2& viewport_size
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
    };

} // namespace Salix