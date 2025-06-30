// Salix/ecs/Sprite2D.cpp
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Entity.h> // We need this to get the owner's Transform.
#include <Salix/ecs/Transform.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/rendering/IRenderer.h>  // We need this for the render method parameter.
#include <Salix/core/SerializationRegistrations.h>
#include <SDL.h>
#include <iostream>
#include <cereal/types/string.hpp> // Needed to serialize std::string
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <filesystem>

namespace Salix {
    extern std::filesystem::path g_project_root_path;
    struct Sprite2D::Pimpl {
        ITexture* texture = nullptr;
            int width = 0;
            int height = 0;

            Pimpl() = default;

            template <class Archive>
            void serialize(Archive& archive) {
                std::cout << "Sprite2D: Pimpl: serilialze entered."
                archive(
                    /* Nothing to serialize here as these members will be initailzed at runtime.*/
                );
            }
    };
    
    Sprite2D::Sprite2D() : pimpl(std::make_unique<Pimpl>()){
        pivot = { 0.5f, 0.5f };
        texture_path = "";
        color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    }
        

    Sprite2D::~Sprite2D() = default;

    void Sprite2D::on_load(AssetManager* asset_manager) {
        if(texture_path.empty()) { return; }
        load_texture(asset_manager, texture_path);
    }

    void Sprite2D::load_texture(AssetManager* asset_manager, const std::string& file_path) {
        if (file_path.empty()) {
            std::cerr << "Warning: Provided texture path is empty.\n";
            return;
        }
         // ---- ADD THIS DEBUG BLOCK ----
        std::cout << "\n--- SPRITE2D DEBUG ---" << std::endl;
        std::cout << "1. Project Root Path = '" << g_project_root_path.string() << "'" << std::endl;
        std::cout << "2. Incoming file_path  = '" << file_path << "'" << std::endl;
        // --- END DEBUG BLOCK ---
        
        // 1. Create a path object from the project-relative string
        std::filesystem::path relative_path(file_path);

        // 2. Combine the project's root path with the relative asset path.
        //    The '/' operator is the correct, platform-agnostic way to join paths.
        std::filesystem::path absolute_path = g_project_root_path / relative_path;

        // 3. Normalize the path. This resolves any ".." or "." segments and
        //    converts separators to the OS-preferred format (e.g., '\' on Windows).
        absolute_path = absolute_path.lexically_normal();

        // 4. Use the final, absolute path string to load the resource.
        texture_path = absolute_path.string();
        // ---- ADD THIS FINAL DEBUG BLOCK ----
        std::cout << "\n--- FINAL POINTER CHECK ---" << std::endl;
        std::cout << "1. asset_manager pointer is: " << (asset_manager ? "VALID" : "!!! NULL !!!") << std::endl;
        std::cout << "2. pimpl pointer is:         " << (pimpl ? "VALID" : "!!! NULL !!!") << std::endl;
        std::cout << "--- ABOUT TO CALL get_texture ---\n" << std::endl;
        // ------------------------------------
        std::cout << "Sprite2D: load_texture - texture_path is set to '" << texture_path << "'" << std::endl;
        pimpl->texture = asset_manager->get_texture(texture_path);  // this is where it crashes
        if (pimpl->texture) {
            pimpl->width = pimpl->texture->get_width();
            pimpl->height = pimpl->texture->get_height();
        } else {
            pimpl->width = 0;
            pimpl->height = 0;
            std::cerr << "Warning: Failed to load texture for Sprite2D: " << texture_path << std::endl;
        }
    }

    void Sprite2D::render(IRenderer* renderer) {
        if (pimpl->texture && owner) {
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
                dest_rect.w = static_cast<int>(pimpl->texture->get_width() * world_scale.x);
                dest_rect.h = static_cast<int>(pimpl->texture->get_height() * world_scale.y);
                
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
                renderer->draw_sprite(pimpl->texture, dest_rect, angle, &pivot_point, color, flip_state);
            }
        }
    }

    const std::string& Sprite2D::get_texture_path() const {

        return texture_path;
    }

    // --- CEREAL IMPLEMENTATION ---
    template<class Archive>
    void Sprite2D::serialize(Archive& archive){
        archive(cereal::base_class<RenderableElement>(this));
        // The serialize function now accesses the public member directly.
        archive (
            cereal::make_nvp("color", color), cereal::make_nvp("offset", offset),
            cereal::make_nvp("pivot", pivot), cereal::make_nvp("flip_h", flip_h),
            cereal::make_nvp("flip_v", flip_v),
            cereal::make_nvp("sorting_layer", sorting_layer),
            cereal::make_nvp("texture_path", texture_path)
        );
    }

    template void Sprite2D::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& );
    template void Sprite2D::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& );
    template void Sprite2D::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void Sprite2D::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &);


} // namespace Salix