// Sprite2D.h
#pragma once

#include "../math/Color.h"
#include "RenderableElement.h"
#include "../math/Vector2.h"  // For the pivot and offset.
#include <string>

// Forward declarations
struct SDL_Texture;
class AssetManager;
class IRenderer;

class Sprite2D : public RenderableElement {
    public:
        Sprite2D();
        virtual ~Sprite2D();

        // A method to load a texture for this sprite using the AssetManager
        void load_texture(AssetManager* asset_manager, std::string& file_path);

        // The implementation of the render method from RenderableElement Interface.
        void render(IRenderer* renderer) override;

        // --- Properties ---
        
        Color color;            // Color property for color tinting.
        Vector2 offset;         // A local offset from the Transform's position
        Vector2 pivot;          // Normalized pivot point (0,0 = top-left, 1,1 = bottom-right).
        bool flip_h = false;    // Flip horizontally?
        bool flip_v = false;    // Flip veritcally?
        int sorting_layer = 0;  // For future use in the Scene.


    private:
        SDL_Texture* texture;
        int width;
        int height;

};