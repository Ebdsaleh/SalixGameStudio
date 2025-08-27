// Salix/ecs/Sprite2D.cpp
#include <Salix/core/InitContext.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/BoxCollider.h>
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

    struct Sprite2D::Pimpl {
        ITexture* texture = nullptr;
            int width = 0;
            int height = 0;
            InitContext context;
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
        color = White;
        set_name(get_class_name());
    }
        

    Sprite2D::~Sprite2D() = default;

    void Sprite2D::on_load(const InitContext& new_context) {
        pimpl->context = new_context;
        if (texture_path.empty()) { return; }
        load_texture(pimpl->context.asset_manager, texture_path);
    }
    
    int Sprite2D::get_texture_width() const { return pimpl->width; }

    int Sprite2D::get_texture_height() const { return pimpl->height; }

    ITexture* Sprite2D::get_texture() const {
        return pimpl->texture;
    }

    void Sprite2D::load_texture(AssetManager* asset_manager, const std::string& relative_file_path) {
        if (relative_file_path.empty()) {
            std::cerr << "Warning: Sprite2D::load_texture called with an empty path." << std::endl;
            pimpl->texture = nullptr; // Explicitly nullify
            pimpl->width = 0;         // Reset dimensions
            pimpl->height = 0;        // Reset dimensions
            return;
        }

        // 1. Store the portable, project-relative path.
        this->texture_path = relative_file_path;

        // 2. Ask the AssetManager to load it using the relative path.
        //    The AssetManager is now responsible for converting it to an absolute path.
        pimpl->texture = asset_manager->get_texture(this->texture_path);

        // 3. Update dimensions if the texture was loaded successfully.
        if (pimpl->texture) {
            pimpl->width = pimpl->texture->get_width();
            pimpl->height = pimpl->texture->get_height();
        } else {
            pimpl->width = 0;
            pimpl->height = 0;
            std::cerr << "Warning: Failed to load texture for Sprite2D using relative path: " << this->texture_path << std::endl;
        }
    }

    void Sprite2D::render(IRenderer* renderer) {
        // Check that we have everything we need to draw
        if (pimpl->texture && owner && renderer) {
            // Get the owner's Transform, which contains all positional data
            Transform* transform = owner->get_transform();

            if (transform) {
                // Determine the flip state from this component's public properties
                SpriteFlip flip_state = SpriteFlip::None;
                if (flip_h && flip_v) {
                    flip_state = SpriteFlip::Both;
                } else if (flip_h) {
                    flip_state = SpriteFlip::Horizontal;
                } else if (flip_v) {
                    flip_state = SpriteFlip::Vertical;
                }
                
                // One simple call to the renderer. The renderer will read the
                // transform and do all the complex work.
                renderer->draw_sprite(pimpl->texture, transform, color, flip_state);
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
        archive(
            cereal::make_nvp("use_entity_rotation", use_entity_rotation),
            cereal::make_nvp("local_rotation", local_rotation),
            cereal::make_nvp("color", color),
            cereal::make_nvp("offset", offset),
            cereal::make_nvp("pivot", pivot),
            cereal::make_nvp("flip_h", flip_h),
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