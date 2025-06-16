// Sprite2D.cpp
#include "Sprite2D.h"
#include "Entity.h" // We need this to get the owner's Transform.
#include "Transform.h"
#include "../assets/AssetManager.h"
#include "../rendering/IRenderer.h"  // We need this for the render method parameter.
#include <SDL.h>
#include <iostream>

Sprite2D::Sprite2D() : texture(nullptr), width(0), height(0) {}
Sprite2D::~Sprite2D() {}

void Sprite2D::load_texture(AssetManager* asset_manager, std::string& file_path) {
    texture = asset_manager->get_texture(file_path);

    // If the texture loaded successfully, query its dimensions.
    if (texture) {
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    }
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
            
            SDL_Rect dest_rect;
            dest_rect.x = static_cast<int>(world_pos.x);
            dest_rect.y = static_cast<int>(world_pos.y);
            // Apply the transform's scaling to the texture.
            dest_rect.w = static_cast<int>(width * world_scale.x);
            dest_rect.h = static_cast<int>(height * world_scale.y);
            // Apply the transform's rotation to the texture.
            double angle = static_cast<double>(world_rot.z);

            // Determine the final flip state.
            SpriteFlip flip_state = SpriteFlip::None;
            if (flip_h && flip_v) {
                flip_state = SpriteFlip::Both;
            } else if (flip_h) {
                flip_state = SpriteFlip::Horizontal;
            } else if (flip_v) {
                flip_state = SpriteFlip::Vertical;
            }


            // render the texture ignoring rotation.
            // renderer->draw_texture(texture, dest_rect);

            // render the texture using rotation.
            renderer->draw_sprite(texture, dest_rect, angle, color, flip_state);
        }
    }
}