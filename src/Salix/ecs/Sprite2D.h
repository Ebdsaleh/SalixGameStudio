// Salix/ecs/Sprite2D.h
#pragma once

#include <Salix/core/Core.h>
#include <Salix/math/Color.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/math/Vector2.h>  // For the pivot and offset.
#include <string>
#include <memory>
#include <cereal/access.hpp>


namespace Salix {

    // Forward declarations
    class ITexture;
    class IRenderer;
    class AssetManager;
    struct InitContext;
    class SALIX_API Sprite2D : public RenderableElement {
        public:
            Sprite2D();
            virtual ~Sprite2D();
            
            void on_load(const InitContext& new_context) override;
            // A method to load a texture for this sprite using the AssetManager
            void load_texture(class AssetManager* asset_manager, const std::string& file_path);

            // A method to return the path of the assigned texture.
            const std::string& get_texture_path() const;

            // The implementation of the render method from RenderableElement Interface.
            void render(IRenderer* renderer) override;


            // --- Properties ---
            
            Color color;                // Color property for color tinting.
            Vector2 offset;             // A local offset from the Transform's position
            Vector2 pivot;              // Normalized pivot point (0,0 = top-left, 1,1 = bottom-right).
            bool flip_h = false;        // Flip horizontally?
            bool flip_v = false;        // Flip veritcally?
            int sorting_layer = 0;      // For future use in the Scene.
            std::string texture_path;   // The path to the image used as the texture.
            
        private:
            struct Pimpl;
            std::unique_ptr<Pimpl>pimpl;
            friend class cereal::access;
            template<class Archive>
            void serialize(Archive& archive);

    };
} // namespace Salix