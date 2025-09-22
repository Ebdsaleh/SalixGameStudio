// =================================================================================
// Filename:    src/Tests/SalixEngine/ecs/Sprite2D.test.cpp
// Description: Contains unit tests for the Sprite2D class.
// ================================================================================= 

#include <doctest.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Entity.h>
#include <Salix/core/InitContext.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/math/Color.h>
#include <Salix/math/Vector2.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/rendering/ITexture.h>
#include <Tests/SalixEngine/mocking/rendering/MockITexture.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
// #include <Salix/core/SerializationRegistrations.h>
#include <memory>
#include <cereal/archives/json.hpp>

TEST_SUITE("Salix::ecs::Sprite2D") {
    TEST_CASE("initializes with default values correctly") {
        // ARRANGE
        Salix::SimpleGuid::reset_counter_for_testing();
        auto sprite = std::make_unique<Salix::Sprite2D>();
        // ACT
        bool expected_use_entity_rotation = false;
        float expected_local_rotation = 0.0f;
        int expected_id = 1;
        Salix::Color expected_color = Salix::White;
        Salix::Vector2 expected_offset = Salix::Vector2(0.0f, 0.0f);
        Salix::Vector2 expected_pivot = Salix::Vector2(0.5f, 0.5f);
        const std::string expected_class_name = "Sprite2D";
        const std::string expected_name = "Sprite2D";
        bool expected_flip_h = false;
        bool expected_flip_v = false;
        int expected_sorting_layer = 0;
        std::string expected_texture_path = "";
        int expected_texture_width = 0;
        int expected_texture_height = 0;
        Salix::ITexture* expected_texture = nullptr;
        SUBCASE("with methods") {
            // ASSERT
            CHECK(expected_use_entity_rotation == sprite->get_use_entity_rotation());
            CHECK(expected_local_rotation == sprite->get_local_rotation());
            CHECK(expected_id == sprite->get_id().get_value());
            CHECK(sprite->get_id_as_ptr() != nullptr);
            CHECK(expected_color == Salix::White);
            CHECK(expected_class_name == sprite->get_class_name());
            CHECK(expected_offset == sprite->get_offset());
            CHECK(expected_pivot == sprite->get_pivot());
            CHECK(expected_name == sprite->get_name());
            CHECK(expected_sorting_layer == sprite->get_sorting_layer());
            CHECK(expected_texture_path == sprite->get_texture_path());
            CHECK(expected_flip_h == sprite->get_flip_h());
            CHECK(expected_flip_v == sprite->get_flip_v());
            CHECK(expected_texture_width == sprite->get_texture_width());
            CHECK(expected_texture_height == sprite->get_texture_height());
            CHECK(expected_texture == sprite->get_texture());
        }
        SUBCASE("with public members") {
            // ASSERT
            CHECK(expected_use_entity_rotation == sprite->use_entity_rotation);
            CHECK(expected_local_rotation == sprite->local_rotation);
            CHECK(expected_color == Salix::White);
            CHECK(expected_offset == sprite->offset);
            CHECK(expected_pivot == sprite->pivot);
            CHECK(expected_name == sprite->name);
            CHECK(expected_sorting_layer == sprite->sorting_layer);
            CHECK(expected_texture_path == sprite->texture_path);
            CHECK(expected_flip_h == sprite->flip_h);
            CHECK(expected_flip_v == sprite->flip_v);
            
        }
    }

    TEST_CASE("const getters work correctly on a const object") {
        // ARRANGE: Create a sprite and then create a const reference to it.
        Salix::Sprite2D sprite;
        sprite.set_color(Salix::Red);
        sprite.set_offset({1, 2});

        const Salix::Sprite2D& const_sprite = sprite;

        // ACT & ASSERT: Calling getters on the const reference will execute
        // the const-qualified versions of the methods, covering them.
        CHECK(const_sprite.get_color() == Salix::Red);
        CHECK(const_sprite.get_offset() == Salix::Vector2(1, 2));
        CHECK(const_sprite.get_pivot() == Salix::Vector2(0.5f, 0.5f));
        CHECK(const_sprite.get_local_rotation() == 0.0f);
        CHECK(const_sprite.get_flip_h() == false);
        CHECK(const_sprite.get_flip_v() == false);
        CHECK(const_sprite.get_sorting_layer() == 0);
        CHECK(const_sprite.get_use_entity_rotation() == false);
    }

    TEST_CASE("load_texture method handles empty path directly") {
        // This test specifically covers the branch inside load_texture,
        // bypassing the check in on_load.
        
        // ARRANGE
        Salix::Sprite2D sprite;
        Salix::AssetManager asset_manager;
        MockIRenderer mock_renderer;
        asset_manager.initialize(&mock_renderer);

        // Pre-condition: give the sprite a valid texture first
        sprite.load_texture(&asset_manager, "assets/test.png");
        REQUIRE(sprite.get_texture() != nullptr);
        REQUIRE(sprite.get_texture_width() != 0);

        // ACT: Call load_texture directly with an empty path
        sprite.load_texture(&asset_manager, "");

        // ASSERT: The texture and dimensions should now be cleared
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("setters correctly modify properties") {
        // ARRANGE
        Salix::Sprite2D sprite;

        // ACT & ASSERT
        sprite.set_color({0.1f, 0.2f, 0.3f, 0.4f});
        CHECK(sprite.get_color() == Salix::Color(0.1f, 0.2f, 0.3f, 0.4f));

        sprite.set_offset({10.0f, 20.0f});
        CHECK(sprite.get_offset() == Salix::Vector2(10.0f, 20.0f));

        sprite.set_pivot({0.0f, 1.0f});
        CHECK(sprite.get_pivot() == Salix::Vector2(0.0f, 1.0f));

        sprite.set_local_rotation(45.0f);
        CHECK(sprite.get_local_rotation() == 45.0f);

        sprite.set_flip_h(true);
        CHECK(sprite.get_flip_h() == true);

        sprite.set_flip_v(true);
        CHECK(sprite.get_flip_v() == true);

        sprite.set_sorting_layer(5);
        CHECK(sprite.get_sorting_layer() == 5);

        sprite.set_texture_path("new/path.png");
        CHECK(sprite.get_texture_path() == "new/path.png");

        sprite.set_use_entity_rotation(true);
        CHECK(sprite.get_use_entity_rotation() == true);
    }

    TEST_CASE("on_load and texture loading") {
        // ARRANGE
        Salix::Sprite2D sprite;
        Salix::InitContext context;
        MockIRenderer mock_renderer;
        Salix::AssetManager asset_manager;
        asset_manager.initialize(&mock_renderer);
        context.asset_manager = &asset_manager;

        SUBCASE("loads texture and sets dimensions correctly") {
            // ACT
            sprite.texture_path = "assets/test.png";
            sprite.on_load(context);

            // ASSERT
            REQUIRE(sprite.get_texture() != nullptr);
            CHECK(sprite.get_texture_width() == 16); // From MockITexture
            CHECK(sprite.get_texture_height() == 16); // From MockITexture
        }

        SUBCASE("handles empty texture path gracefully") {
            // ACT
            sprite.texture_path = "";
            sprite.on_load(context);

            // ASSERT
            CHECK(sprite.get_texture() == nullptr);
            CHECK(sprite.get_texture_width() == 0);
            CHECK(sprite.get_texture_height() == 0);
        }

        SUBCASE("handles failed texture load") {
            // ARRANGE: Configure the mock to fail the texture load
            mock_renderer.should_texture_load_fail = true;

            // ACT
            sprite.texture_path = "assets/fail.png";
            sprite.on_load(context);

            // ASSERT
            CHECK(sprite.get_texture() == nullptr);
            CHECK(sprite.get_texture_width() == 0);
            CHECK(sprite.get_texture_height() == 0);
        }
    }

    TEST_CASE("render logic") {
        // ARRANGE
        // A unique_ptr is used here to match the change you made
        auto sprite = std::make_unique<Salix::Sprite2D>(); 
        MockIRenderer mock_renderer;
        Salix::Entity owner;
        sprite->set_owner(&owner);
        
        SUBCASE("does not render if texture is null") {
            // ACT
            sprite->render(&mock_renderer);

            // ASSERT
            CHECK(mock_renderer.draw_sprite_call_count == 0);
        }

        SUBCASE("does not render if owner is null") {
            // ARRANGE
            // --- THIS IS THE FIX ---
            Salix::AssetManager asset_manager;
            asset_manager.initialize(&mock_renderer);
            // ---------------------

            sprite->set_owner(nullptr);
            // Simulate texture being loaded correctly
            sprite->load_texture(&asset_manager, "fake_path.png"); // Pass the valid manager

            // ACT
            sprite->render(&mock_renderer);

            // ASSERT
            CHECK(mock_renderer.draw_sprite_call_count == 0);
        }

        SUBCASE("renders correctly when all conditions are met") {
            // ARRANGE
            Salix::AssetManager asset_manager;
            asset_manager.initialize(&mock_renderer);
            sprite->load_texture(&asset_manager, "assets/test.png");

            // ACT
            sprite->render(&mock_renderer);

            // ASSERT
            CHECK(mock_renderer.draw_sprite_call_count == 1);
        }
        
        SUBCASE("passes correct flip state to renderer") {
            // ARRANGE
            Salix::AssetManager asset_manager;
            asset_manager.initialize(&mock_renderer);
            sprite->load_texture(&asset_manager, "assets/test.png");

            // ACT & ASSERT for Horizontal
            sprite->flip_h = true;
            sprite->flip_v = false;
            sprite->render(&mock_renderer);
            CHECK(mock_renderer.last_flip_state == Salix::SpriteFlip::Horizontal);
            
            // ACT & ASSERT for Vertical
            sprite->flip_h = false;
            sprite->flip_v = true;
            sprite->render(&mock_renderer);
            CHECK(mock_renderer.last_flip_state == Salix::SpriteFlip::Vertical);

            // ACT & ASSERT for Both
            sprite->flip_h = true;
            sprite->flip_v = true;
            sprite->render(&mock_renderer);
            CHECK(mock_renderer.last_flip_state == Salix::SpriteFlip::Both);

            // ACT & ASSERT for None
            sprite->flip_h = false;
            sprite->flip_v = false;
            sprite->render(&mock_renderer);
            CHECK(mock_renderer.last_flip_state == Salix::SpriteFlip::None);
        }
    }
    TEST_CASE("can be serialized and deserialized correctly") {
        // ARRANGE
        auto original_sprite = std::make_unique<Salix::Sprite2D>();
        original_sprite->set_name("TestSprite");
        original_sprite->set_color({0.1f, 0.2f, 0.3f, 0.4f});
        original_sprite->set_offset({10, 20});
        original_sprite->set_pivot({0.1f, 0.9f});
        original_sprite->set_local_rotation(45.0f);
        original_sprite->set_flip_h(true);
        original_sprite->set_sorting_layer(10);
        original_sprite->set_texture_path("path/to/my/texture.png");

        std::stringstream ss;

        // ACT (Serialization)
        {
            cereal::JSONOutputArchive output_archive(ss);
            output_archive(*original_sprite);
        }

        // ACT (Deserialization)
        auto loaded_sprite = std::make_unique<Salix::Sprite2D>();
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(*loaded_sprite);
        }

        // ASSERT
        REQUIRE(loaded_sprite != nullptr);
        CHECK(loaded_sprite->get_name() == "TestSprite");
        CHECK(loaded_sprite->get_color() == Salix::Color(0.1f, 0.2f, 0.3f, 0.4f));
        CHECK(loaded_sprite->get_offset() == Salix::Vector2(10, 20));
        CHECK(loaded_sprite->get_pivot() == Salix::Vector2(0.1f, 0.9f));
        CHECK(loaded_sprite->get_local_rotation() == doctest::Approx(45.0f));
        CHECK(loaded_sprite->get_flip_h() == true);
        CHECK(loaded_sprite->get_flip_v() == false); // Should remain default
        CHECK(loaded_sprite->get_sorting_layer() == 10);
        CHECK(loaded_sprite->get_texture_path() == "path/to/my/texture.png");
    }
}