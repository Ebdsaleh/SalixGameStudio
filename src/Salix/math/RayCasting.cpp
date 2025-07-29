// Salix/math/RayCasting.cpp
#include <Salix/math/RayCasting.h>
#include <Salix/rendering/ICamera.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <glm/glm.hpp>
#include <imgui.h> 

namespace Salix {

    Ray Raycast::CreateRayFromScreen(
        ICamera* camera,
        const ImVec2& mouse_pos,
        const ImVec2& viewport_pos,
        const ImVec2& viewport_size)
    {
        // --- 1. Convert mouse position to Normalized Device Coordinates (NDC) ---
        // The mouse position is relative to the viewport's top-left corner.
        ImVec2 mouse_relative = { mouse_pos.x - viewport_pos.x, mouse_pos.y - viewport_pos.y };
        
        float ndc_x = (2.0f * mouse_relative.x) / viewport_size.x - 1.0f;
        float ndc_y = 1.0f - (2.0f * mouse_relative.y) / viewport_size.y; // Y is flipped for OpenGL

        // --- 2. Unproject from Screen Space to World Space ---
        const glm::mat4& proj_matrix = camera->get_projection_matrix();
        const glm::mat4& view_matrix = camera->get_view_matrix();

        glm::mat4 inv_proj = glm::inverse(proj_matrix);
        glm::mat4 inv_view = glm::inverse(view_matrix);

        // Start with the point on the near clip plane
        glm::vec4 ray_clip = glm::vec4(ndc_x, ndc_y, -1.0, 1.0);
        
        // Transform to eye/view space
        glm::vec4 ray_eye = inv_proj * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        // Transform to world space to get the final direction
        glm::vec4 ray_world_4d = inv_view * ray_eye;
        glm::vec3 ray_dir = glm::normalize(glm::vec3(ray_world_4d));

        // The ray's origin is the camera's position
        // NOTE: You'll need to add a get_transform() method to your ICamera interface
        // or pass the camera's position in as a parameter.
        // For now, we assume the camera is at the origin of its view matrix.
        glm::vec3 ray_origin = glm::vec3(inv_view[3]);

        return { ray_origin, ray_dir };
    }


    Ray Raycast::CreateRayFromScreen(
        ICamera* camera,
        const Vector2& mouse_pos,
        const Vector2& viewport_pos,
        const Vector2& viewport_size)
    {
        // Convert your types to the types the core function needs and call it
        return CreateRayFromScreen(
            camera,
            ImVec2(mouse_pos.x, mouse_pos.y),
            ImVec2(viewport_pos.x, viewport_pos.y),
            ImVec2(viewport_size.x, viewport_size.y)
        );
    }




    bool Raycast::IntersectsAABB(
        const Ray& ray,
        const glm::vec3& box_min,
        const glm::vec3& box_max,
        float& distance)
    {
        glm::vec3 inv_dir = 1.0f / ray.direction;
        glm::vec3 tMin = (box_min - ray.origin) * inv_dir;
        glm::vec3 tMax = (box_max - ray.origin) * inv_dir;

        glm::vec3 t1 = glm::min(tMin, tMax);
        glm::vec3 t2 = glm::max(tMin, tMax);

        float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
        float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

        if (tNear > tFar || tFar < 0.0f) {
            return false;
        }

        distance = tNear > 0.0f ? tNear : tFar;
        return true;
    }


    bool Raycast::IntersectsAABB(
        const Ray& ray,
        const Vector3& box_min,
        const Vector3& box_max,
        float& distance)
    {
        // Convert your types to the types the core function needs and call it
        // (This assumes your Vector3 has a .to_glm() helper method)
        return IntersectsAABB(
            ray,
            box_min.to_glm(),
            box_max.to_glm(),
            distance
        );
    }

} // namespace Salix
