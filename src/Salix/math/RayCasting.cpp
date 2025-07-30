// Salix/math/RayCasting.cpp
#include <Salix/math/RayCasting.h>
#include <Salix/rendering/ICamera.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/ecs/Transform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h> 

namespace Salix {

    Ray Raycast::CreateRayFromScreen(
        ICamera* camera,
        const ImVec2& mouse_pos,
        const ImVec2& viewport_pos,
        const ImVec2& viewport_size) {
        // --- 1. Convert to NDC ---
        ImVec2 mouse_relative = { mouse_pos.x - viewport_pos.x, mouse_pos.y - viewport_pos.y };
        float ndc_x = (2.0f * mouse_relative.x) / viewport_size.x - 1.0f;
        float ndc_y = 1.0f - (2.0f * mouse_relative.y) / viewport_size.y;

        // --- 2. Unproject near and far points from clip space to world space ---
        const glm::mat4& proj = camera->get_projection_matrix();
        const glm::mat4& view = camera->get_view_matrix();
        
        glm::mat4 inv_view_proj = glm::inverse(proj * view);

        glm::vec4 near_point_clip = { ndc_x, ndc_y, -1.0, 1.0 };
        glm::vec4 far_point_clip  = { ndc_x, ndc_y,  1.0, 1.0 };

        glm::vec4 near_point_world = inv_view_proj * near_point_clip;
        glm::vec4 far_point_world  = inv_view_proj * far_point_clip;

        // Perform perspective divide
        near_point_world /= near_point_world.w;
        far_point_world  /= far_point_world.w;

        // --- 3. Create the final ray ---
        // CORRECTED: The direction must point from the near plane TOWARDS the far plane.
        glm::vec3 ray_dir = glm::normalize(glm::vec3(far_point_world) - glm::vec3(near_point_world));
        
        // The origin is the camera's position, derived from the inverse view matrix.
        glm::vec3 ray_origin = glm::vec3(glm::inverse(view)[3]);

        return { ray_origin, ray_dir };
    }

    Ray Raycast::CreateRayFromScreen(
        ICamera* camera,
        const Vector2& mouse_pos,
        const Vector2& viewport_pos,
        const Vector2& viewport_size)
    {
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
        float& distance) {

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
        return IntersectsAABB(
            ray,
            box_min.to_glm(),
            box_max.to_glm(),
            distance
        );
    }

    bool Raycast::IntersectsOBB(
        const Ray& ray,
        const glm::mat4& model_matrix,
        const glm::vec3& half_extents,
        float& out_distance) {
        
        // const float EPSILON = 1e-6f;
        glm::vec3 obb_position_world = glm::vec3(model_matrix[3]);
        glm::vec3 delta = obb_position_world - ray.origin;

        // Transform the ray direction to the OBB's local space
        glm::mat4 model_inverse = glm::inverse(model_matrix);
        glm::vec3 ray_origin_local = glm::vec3(model_inverse * glm::vec4(ray.origin, 1.0f));
        glm::vec3 ray_direction_local = glm::normalize(glm::vec3(model_inverse * glm::vec4(ray.direction, 0.0f)));

        Ray local_ray = { ray_origin_local, ray_direction_local };
        glm::vec3 box_min = -half_extents;
        glm::vec3 box_max = half_extents;

        return IntersectsAABB(local_ray, box_min, box_max, out_distance);
    }

} // namespace Salix
