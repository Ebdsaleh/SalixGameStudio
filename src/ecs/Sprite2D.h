// Sprite2D.h
#pragma once

#include "../math/Color.h"
#include "RenderableElement.h"
#include <string>

// Forward declarations
struct SDL_Texture;
class AssetManager;

class Sprite2D : public RenderableElement {
    public:
        Sprite2D();
        virtual ~Sprite2D();

        // A method to load a texture for this sprite using the AssetManager
        void load_texture(AssetManager* asset_manager, std::string& file_path);

        // The implementation of the render method from RenderableElement Interface.
        void render(class IRenderer* renderer) override;

        // Color property for color tinting.
        Color color;
    private:
        SDL_Texture* texture;
        int width;
        int height;

};