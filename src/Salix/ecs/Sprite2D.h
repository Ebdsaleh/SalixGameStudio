// Salix/ecs/Sprite2D.h
#pragma once

#include <Salix/core/Core.h>
#include <Salix/math/Color.h>
#include <Salix/ecs/RenderableElement2D.h>
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
    class SALIX_API Sprite2D : public RenderableElement2D {
        public:
            Sprite2D();
            virtual ~Sprite2D();
            const char* get_class_name() const override { return "Sprite2D"; } 
            void on_load(const InitContext& new_context) override;
            // A method to load a texture for this sprite using the AssetManager
            void load_texture(class AssetManager* asset_manager, const std::string& file_path);

            // A method to return the path of the assigned texture.
            

            // The implementation of the render method from RenderableElement Interface.
            void render(IRenderer* renderer) override;


            // --- Properties ---
            bool use_entity_rotation = false;  // For use with hybrid rendering pipeline, set to true when camera is in 'perspective' mode.
            float local_rotation = 0.0f;  // For 2D rotation in degrees.
            Color color;                // Color property for color tinting.
            Vector2 offset;             // A local offset from the Transform's position
            Vector2 pivot;              // Normalized pivot point (0,0 = top-left, 1,1 = bottom-right).
            bool flip_h = false;        // Flip horizontally?
            bool flip_v = false;        // Flip veritcally?
            int sorting_layer = 0;      // For future use in the Scene.
            std::string texture_path;   // The path to the image used as the texture.

            // Getters for MODIFICATION (non-const, return by reference)
            bool& get_use_entity_rotation() { return use_entity_rotation; }
            Color& get_color() { return color; }
            Vector2& get_offset() { return offset; }
            Vector2& get_pivot() { return pivot; }
            float& get_local_rotation() { return local_rotation; }
            bool& get_flip_h() { return flip_h; }
            bool& get_flip_v() { return flip_v; }
            int& get_sorting_layer() { return sorting_layer; }

            // Getters for READING (const, return by value or const reference)
            bool get_use_entity_rotation() const { return use_entity_rotation; }
            const Color& get_color() const { return color; }
            const Vector2& get_offset() const { return offset; }
            const Vector2& get_pivot() const { return pivot; }
            float get_local_rotation() const { return local_rotation; }
            bool get_flip_h() const { return flip_h; }
            bool get_flip_v() const { return flip_v; }
            int get_sorting_layer() const { return sorting_layer; }
            
            // Getters and Setters for use with our ByteMirror reflection system
            void set_use_entity_rotation(bool should_use_entity_rotation) {
                use_entity_rotation = should_use_entity_rotation; 
            }
            void set_local_rotation(const float& new_rotation) { local_rotation = new_rotation; }

           
            void set_color(const Color& new_color) { color = new_color; }

            
            void set_offset(const Vector2& new_offset) { offset = new_offset; }

            void set_pivot(const Vector2& new_pivot) { pivot = new_pivot; }

            
            void set_flip_h(bool should_flip) { flip_h = should_flip; }

            
            void set_flip_v(bool should_flip) { flip_v = should_flip; }

            
            void set_sorting_layer(const int& new_sorting_layer) { sorting_layer = new_sorting_layer; }

            const std::string& get_texture_path() const;
            void set_texture_path(const std::string& new_texture_path) { texture_path = new_texture_path; }

            int get_texture_width() const;
            int get_texture_height() const;

            ITexture* get_texture() const;

        private:
            struct Pimpl;
            std::unique_ptr<Pimpl>pimpl;
            friend class cereal::access;
            template<class Archive>
            void serialize(Archive& archive);

    };
} // namespace Salix