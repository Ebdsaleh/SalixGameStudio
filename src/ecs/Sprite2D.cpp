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
            SDL_Rect dest_rect;
            dest_rect.x = static_cast<int>(transform->position.x);
            dest_rect.y = static_cast<int>(transform->position.y);
            // Apply the transform's scaling to the texture.
            dest_rect.w = static_cast<int>(width * transform->scale.x);
            dest_rect.h = static_cast<int>(height * transform->scale.y);
            // render the texture.
            renderer->draw_texture(texture, dest_rect);
        }
    }
}