// Salix/ecs/Sprite2D.cpp
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Entity.h> // We need this to get the owner's Transform.
#include <Salix/ecs/Transform.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/rendering/IRenderer.h>  // We need this for the render method parameter.
#include <SDL.h>
#include <iostream>

namespace Salix {

    Sprite2D::Sprite2D() : 
        texture(nullptr), 
        width(0),
        height(0),
        pivot({ 0.5f, 0.5f }) {}

    Sprite2D::~Sprite2D() {}

    void Sprite2D::load_texture(AssetManager* asset_manager, const std::string& file_path) {
        texture = asset_manager->get_texture(file_path);
    }

    void Sprite2D::render(IRenderer* renderer) {
        if (texture && owner) {
            // Get the owner's Transform to know where to draw
            Transform* transform = owner->get_transform();

            if (transform) {
                // Get the final calculated world-space transform values.
                Vector3 world_pos = transform->get_world_position();
                Vector3 world_rot = transform->get_world_rotation();
                Vector3 world_scale = transform->get_world_scale();
                
                Rect dest_rect;
                dest_rect.x = static_cast<int>(world_pos.x + offset.x);
                dest_rect.y = static_cast<int>(world_pos.y + offset.y);
                
                // Apply the transform's scaling to the texture.
                dest_rect.w = static_cast<int>(texture->get_width() * world_scale.x);
                dest_rect.h = static_cast<int>(texture->get_height() * world_scale.y);
                
                // Apply the transform's rotation to the texture.
                double angle = static_cast<double>(world_rot.z);
                
                // Calculate the rotation and pivot in pixels
                Point pivot_point;
                pivot_point.x = static_cast<int>(dest_rect.w * pivot.x);
                pivot_point.y = static_cast<int>(dest_rect.h * pivot.y);

                // Determine the final flip state.
                SpriteFlip flip_state = SpriteFlip::None;
                if (flip_h && flip_v) {
                    flip_state = SpriteFlip::Both;
                } else if (flip_h) {
                    flip_state = SpriteFlip::Horizontal;
                } else if (flip_v) {
                    flip_state = SpriteFlip::Vertical;
                }


                // render the texture ignoring rotation (might be useful for GUI Elements).
                // renderer->draw_texture(texture, dest_rect);

                // render the texture using rotation.
                renderer->draw_sprite(texture, dest_rect, angle, &pivot_point, color, flip_state);
            }
        }
    }
} // namespace Salix