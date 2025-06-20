// Salix/ecs/Sprite2D.h
#pragma once

#include <Salix/math/Color.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/math/Vector2.h>  // For the pivot and offset.
#include <string>

namespace Salix {

    // Forward declarations
    class ITexture;
    class IRenderer;

    class Sprite2D : public RenderableElement {
        public:
            Sprite2D();
            virtual ~Sprite2D();

            // A method to load a texture for this sprite using the AssetManager
            void load_texture(class AssetManager* asset_manager, const std::string& file_path);

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
            ITexture* texture;
            int width;
            int height;

    };
} // namespace Salix