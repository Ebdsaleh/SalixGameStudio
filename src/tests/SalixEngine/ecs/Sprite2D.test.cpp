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
        bool expected_visibility = true;
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
            CHECK(expected_visibility == sprite->is_visible());
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

// Sprite property mutability and archive fidelity without real graphics resources.
#include <cereal/archives/binary.hpp>
#include <sstream>
TEST_SUITE("Salix::ecs::Sprite2DAdditional") {
    TEST_CASE("sprite mutable color reference updates state observable through const interface") {
        Salix::Sprite2D sprite;
        sprite.get_color() = Salix::Color(0.1f, 0.2f, 0.3f, 0.4f);
        const auto& read_only = static_cast<const Salix::Sprite2D&>(sprite);
        CHECK(read_only.get_color() == Salix::Color(0.1f, 0.2f, 0.3f, 0.4f));
    }

    TEST_CASE("sprite offset reference and setter retain negative positional offsets") {
        Salix::Sprite2D sprite;
        sprite.set_offset(Salix::Vector2(-7.5f, 20.25f));
        CHECK(sprite.get_offset() == Salix::Vector2(-7.5f, 20.25f));
        sprite.get_offset().x = 9.5f;
        CHECK(static_cast<const Salix::Sprite2D&>(sprite).get_offset().x == 9.5f);
    }

    TEST_CASE("sprite pivot supports all four normalized anchor corners") {
        Salix::Sprite2D sprite;
        const Salix::Vector2 anchors[] = { {0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {1.f, 1.f} };
        for (const auto& pivot : anchors) {
            sprite.set_pivot(pivot);
            CHECK(sprite.get_pivot() == pivot);
        }
    }

    TEST_CASE("sprite horizontal and vertical flip flags can be toggled independently") {
        Salix::Sprite2D sprite;
        sprite.set_flip_h(true);
        CHECK(sprite.get_flip_h());
        CHECK_FALSE(sprite.get_flip_v());
        sprite.set_flip_v(true);
        CHECK(sprite.get_flip_h());
        CHECK(sprite.get_flip_v());
        sprite.set_flip_h(false);
        CHECK_FALSE(sprite.get_flip_h());
        CHECK(sprite.get_flip_v());
    }

    TEST_CASE("sprite sorting layers preserve negative and large relative ordering") {
        Salix::Sprite2D background, foreground;
        background.set_sorting_layer(-200);
        foreground.set_sorting_layer(200);
        CHECK(background.get_sorting_layer() < foreground.get_sorting_layer());
        foreground.get_sorting_layer() = -201;
        CHECK(foreground.get_sorting_layer() < background.get_sorting_layer());
    }

    TEST_CASE("sprite local rotation and entity-rotation flag are separate properties") {
        Salix::Sprite2D sprite;
        sprite.set_local_rotation(-270.f);
        sprite.set_use_entity_rotation(true);
        CHECK(sprite.get_local_rotation() == -270.f);
        CHECK(sprite.get_use_entity_rotation());
        sprite.get_use_entity_rotation() = false;
        CHECK_FALSE(static_cast<const Salix::Sprite2D&>(sprite).get_use_entity_rotation());
        CHECK(sprite.get_local_rotation() == -270.f);
    }

    TEST_CASE("sprite setting a texture path does not load assets until on_load") {
        Salix::Sprite2D sprite;
        sprite.set_texture_path("art/ui/atlas.png");
        CHECK(sprite.get_texture_path() == "art/ui/atlas.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("sprite empty texture path on_load requires no asset manager or renderer") {
        Salix::Sprite2D sprite;
        Salix::InitContext context{};
        sprite.set_texture_path("");
        CHECK_NOTHROW(sprite.on_load(context));
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("sprite null rendering inputs keep draw count zero and never require a texture") {
        Salix::Sprite2D sprite;
        MockIRenderer renderer;
        sprite.render(&renderer);
        sprite.render(nullptr);
        CHECK(renderer.draw_sprite_call_count == 0);
        CHECK(sprite.get_texture() == nullptr);
    }

    TEST_CASE("sprite binary roundtrip restores flipping layering tint rotation and texture path") {
        Salix::Sprite2D original;
        original.set_use_entity_rotation(true);
        original.set_flip_h(true);
        original.set_flip_v(true);
        original.set_sorting_layer(-21);
        original.set_local_rotation(135.f);
        original.set_offset(Salix::Vector2(12.5f, -8.25f));
        original.set_pivot(Salix::Vector2(0.f, 1.f));
        original.set_color(Salix::Color(0.2f, 0.6f, 0.8f, 0.3f));
        original.set_texture_path("textures/menu.png");
        std::stringstream bytes;
        { cereal::BinaryOutputArchive archive(bytes); archive(original); }
        Salix::Sprite2D restored;
        { cereal::BinaryInputArchive archive(bytes); archive(restored); }
        CHECK(restored.get_use_entity_rotation());
        CHECK(restored.get_flip_h());
        CHECK(restored.get_flip_v());
        CHECK(restored.get_sorting_layer() == -21);
        CHECK(restored.get_local_rotation() == 135.f);
        CHECK(restored.get_offset() == original.get_offset());
        CHECK(restored.get_pivot() == original.get_pivot());
        CHECK(restored.get_color() == original.get_color());
        CHECK(restored.get_texture_path() == original.get_texture_path());
    }
}

// Additional behaviour and lifecycle tests; existing cases preserved.
TEST_SUITE("Salix::ecs::Sprite2DBatch647") {
    TEST_CASE("texture loading exposes the mocked texture's dimensions") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets,"art/sprite.png");
        REQUIRE(sprite.get_texture()!=nullptr);
        CHECK(sprite.get_texture_width()==16);
        CHECK(sprite.get_texture_height()==16);
    }

    TEST_CASE("two sprites referencing the same asset reuse the cached texture") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        Salix::Sprite2D one,two;
        one.load_texture(&assets,"art/shared.png");
        two.load_texture(&assets,"art/shared.png");
        REQUIRE(one.get_texture()!=nullptr);
        CHECK(one.get_texture()==two.get_texture());
    }

    TEST_CASE("loading a failed texture leaves no active texture or dimensions") {
        MockIRenderer renderer; renderer.should_texture_load_fail=true;
        Salix::AssetManager assets; assets.initialize(&renderer);
        Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/missing.png");
        CHECK(sprite.get_texture()==nullptr);
        CHECK(sprite.get_texture_width()==0);
        CHECK(sprite.get_texture_height()==0);
        CHECK(sprite.get_texture_path()=="art/missing.png");
    }

    TEST_CASE("retrying a failed texture load recovers when the backend succeeds") {
        MockIRenderer renderer; renderer.should_texture_load_fail=true;
        Salix::AssetManager assets; assets.initialize(&renderer);
        Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/retry.png");
        REQUIRE(sprite.get_texture()==nullptr);
        renderer.should_texture_load_fail=false;
        sprite.load_texture(&assets,"art/retry.png");
        REQUIRE(sprite.get_texture()!=nullptr);
        CHECK(sprite.get_texture_width()==16);
    }

    TEST_CASE("clearing an already loaded texture resets the runtime dimensions") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/first.png");
        REQUIRE(sprite.get_texture()!=nullptr);
        sprite.load_texture(&assets,"");
        CHECK(sprite.get_texture()==nullptr);
        CHECK(sprite.get_texture_width()==0);
        CHECK(sprite.get_texture_height()==0);
    }

    TEST_CASE("on_load uses a stored path to acquire a texture through AssetManager") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        Salix::InitContext context{};context.asset_manager=&assets;
        Salix::Sprite2D sprite;sprite.set_texture_path("assets/sheet.png");
        sprite.on_load(context);
        REQUIRE(sprite.get_texture()!=nullptr);
        CHECK(sprite.get_texture_path()=="assets/sheet.png");
        CHECK(sprite.get_texture_height()==16);
    }

    TEST_CASE("a loaded but unowned sprite does not invoke the draw call") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        Salix::Sprite2D sprite;sprite.load_texture(&assets,"art/no-owner.png");
        REQUIRE(sprite.get_texture()!=nullptr);
        sprite.render(&renderer);
        CHECK(renderer.draw_sprite_call_count==0);
    }

    TEST_CASE("an attached sprite renders exactly once per explicit draw") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        Salix::Entity entity;
        auto* sprite=entity.add_element<Salix::Sprite2D>();REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/owned.png");REQUIRE(sprite->get_texture()!=nullptr);
        sprite->render(&renderer);
        CHECK(renderer.draw_sprite_call_count==1);
        sprite->render(&renderer);
        CHECK(renderer.draw_sprite_call_count==2);
    }

    TEST_CASE("sprite flip combinations select the matching renderer state") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer);
        Salix::Entity entity;auto* sprite=entity.add_element<Salix::Sprite2D>();REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/flips.png");REQUIRE(sprite->get_texture()!=nullptr);
        sprite->set_flip_h(true);sprite->set_flip_v(false);sprite->render(&renderer);
        CHECK(renderer.last_flip_state==Salix::SpriteFlip::Horizontal);
        sprite->set_flip_v(true);sprite->render(&renderer);
        CHECK(renderer.last_flip_state==Salix::SpriteFlip::Both);
        sprite->set_flip_h(false);sprite->render(&renderer);
        CHECK(renderer.last_flip_state==Salix::SpriteFlip::Vertical);
        CHECK(renderer.draw_sprite_call_count==3);
    }

    TEST_CASE("serializing an unloaded sprite does not materialize a texture at load time") {
        Salix::Sprite2D source;source.set_texture_path("assets/portable.png");
        source.set_flip_h(true);source.set_sorting_layer(23);
        std::stringstream stream;
        {cereal::JSONOutputArchive out(stream);out(source);}
        Salix::Sprite2D restored;
        {cereal::JSONInputArchive in(stream);in(restored);}
        CHECK(restored.get_texture_path()=="assets/portable.png");
        CHECK(restored.get_flip_h());
        CHECK(restored.get_sorting_layer()==23);
        CHECK(restored.get_texture()==nullptr);
        CHECK(restored.get_texture_width()==0);
    }
}

#include <cstdint>
TEST_SUITE("Salix::ecs::Sprite2DBatch947") {
    TEST_CASE("mutable sprite property references update every exposed render setting") {
        Salix::Sprite2D sprite;
        sprite.get_use_entity_rotation() = true;
        sprite.get_local_rotation() = -45.0f;
        sprite.get_flip_h() = true;
        sprite.get_flip_v() = true;
        sprite.get_sorting_layer() = -99;
        sprite.get_offset() = Salix::Vector2(3.0f, -7.0f);
        sprite.get_pivot() = Salix::Vector2(0.25f, 0.75f);
        CHECK(sprite.get_use_entity_rotation());
        CHECK(sprite.get_local_rotation() == -45.0f);
        CHECK(sprite.get_flip_h());
        CHECK(sprite.get_flip_v());
        CHECK(sprite.get_sorting_layer() == -99);
        CHECK(sprite.get_offset() == Salix::Vector2(3.0f, -7.0f));
        CHECK(sprite.get_pivot() == Salix::Vector2(0.25f, 0.75f));
    }

    TEST_CASE("changing stored texture path alone does not replace an already loaded runtime texture") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "art/loaded.png");
        auto* loaded = sprite.get_texture();
        REQUIRE(loaded != nullptr);
        sprite.set_texture_path("art/deferred.png");
        CHECK(sprite.get_texture() == loaded);
        CHECK(sprite.get_texture_path() == "art/deferred.png");
    }

    TEST_CASE("reloading identical texture path reuses cached runtime texture") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "art/repeat.png");
        auto* first = sprite.get_texture();
        REQUIRE(first != nullptr);
        sprite.load_texture(&assets, "art/repeat.png");
        CHECK(sprite.get_texture() == first);
        CHECK(sprite.get_texture_width() == 16);
        CHECK(sprite.get_texture_height() == 16);
    }

    TEST_CASE("loading a distinct texture path replaces runtime texture without changing dimensions") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "art/first-unique.png");
        auto* first = sprite.get_texture();
        REQUIRE(first != nullptr);
        sprite.load_texture(&assets, "art/second-unique.png");
        REQUIRE(sprite.get_texture() != nullptr);
        CHECK(sprite.get_texture() != first);
        CHECK(sprite.get_texture_width() == 16);
        CHECK(sprite.get_texture_height() == 16);
        CHECK(sprite.get_texture_path() == "art/second-unique.png");
    }

    TEST_CASE("failed replacement clears prior runtime texture while retaining requested path") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "art/working.png");
        REQUIRE(sprite.get_texture() != nullptr);
        renderer.should_texture_load_fail = true;
        sprite.load_texture(&assets, "art/failing-replacement.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
        CHECK(sprite.get_texture_path() == "art/failing-replacement.png");
    }

    TEST_CASE("on load with empty stored path leaves existing runtime texture untouched") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "art/runtime.png");
        auto* runtime = sprite.get_texture();
        REQUIRE(runtime != nullptr);
        sprite.set_texture_path("");
        Salix::InitContext context{};
        context.asset_manager = &assets;
        sprite.on_load(context);
        CHECK(sprite.get_texture() == runtime);
        CHECK(sprite.get_texture_width() == 16);
        CHECK(sprite.get_texture_path().empty());
    }

    TEST_CASE("rendering through null renderer preserves loaded sprite runtime state") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "art/null-renderer.png");
        auto* texture = sprite.get_texture();
        REQUIRE(texture != nullptr);
        sprite.set_flip_h(true);
        sprite.set_sorting_layer(12);
        sprite.render(nullptr);
        CHECK(sprite.get_texture() == texture);
        CHECK(sprite.get_flip_h());
        CHECK(sprite.get_sorting_layer() == 12);
    }

    TEST_CASE("successful draw does not mutate sprite render properties") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Entity entity;
        auto* sprite = entity.add_element<Salix::Sprite2D>();
        REQUIRE(sprite != nullptr);
        sprite->load_texture(&assets, "art/stable-draw.png");
        sprite->set_color(Salix::Color(0.2f, 0.4f, 0.6f, 0.8f));
        sprite->set_offset(Salix::Vector2(-3.0f, 5.0f));
        sprite->set_pivot(Salix::Vector2(0.1f, 0.9f));
        sprite->set_local_rotation(22.5f);
        sprite->set_sorting_layer(5);
        sprite->render(&renderer);
        CHECK(renderer.draw_sprite_call_count == 1);
        CHECK(sprite->get_color() == Salix::Color(0.2f, 0.4f, 0.6f, 0.8f));
        CHECK(sprite->get_offset() == Salix::Vector2(-3.0f, 5.0f));
        CHECK(sprite->get_pivot() == Salix::Vector2(0.1f, 0.9f));
        CHECK(sprite->get_local_rotation() == 22.5f);
        CHECK(sprite->get_sorting_layer() == 5);
    }

    TEST_CASE("JSON roundtrip restores base identity and sprite state without runtime texture") {
        Salix::Sprite2D original;
        original.set_id(Salix::SimpleGuid::from_value((std::uint64_t{1} << 40) + 17));
        original.set_name("ForegroundSprite");
        original.set_texture_path("art/persisted.png");
        original.set_flip_v(true);
        original.set_sorting_layer(44);
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(original); }
        Salix::Sprite2D restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_id() == original.get_id());
        CHECK(restored.get_name() == "ForegroundSprite");
        CHECK(restored.get_texture_path() == "art/persisted.png");
        CHECK(restored.get_flip_v());
        CHECK(restored.get_sorting_layer() == 44);
        CHECK(restored.get_texture() == nullptr);
    }

    TEST_CASE("empty direct texture load clears runtime resource but preserves previous portable path") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "art/portable-original.png");
        REQUIRE(sprite.get_texture() != nullptr);
        REQUIRE(sprite.get_texture_path() == "art/portable-original.png");
        sprite.load_texture(&assets, "");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
        CHECK(sprite.get_texture_path() == "art/portable-original.png");
    }
}

// ---- Relocated from src/tests/SalixEngine/ecs/RenderableElement.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Entity.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/ecs/MockRenderableElement.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::ecs::RenderableElementBatch1547") {
    TEST_CASE("renderable element defaults visible through abstract base contract") {
        MockRenderableElementContract element;
        Salix::RenderableElement* base = &element;
        CHECK(base->is_visible());
    }

    TEST_CASE("renderable element visibility setter toggles inherited visibility flag") {
        MockRenderableElementContract element;
        Salix::RenderableElement* base = &element;
        base->set_visibility(false);
        CHECK_FALSE(base->is_visible());
        base->set_visibility(true);
        CHECK(base->is_visible());
    }

    TEST_CASE("renderable element render dispatch receives exact renderer pointer") {
        MockRenderableElementContract element;
        MockIRenderer renderer;
        Salix::RenderableElement* base = &element;
        base->render(&renderer);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == &renderer);
    }

    TEST_CASE("renderable element render contract accepts null renderer for isolated call") {
        MockRenderableElementContract element;
        Salix::RenderableElement* base = &element;
        base->render(nullptr);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == nullptr);
    }

    TEST_CASE("renderable element on load receives exact initialization context") {
        MockRenderableElementContract element;
        Salix::InitContext context{};
        Salix::RenderableElement* base = &element;
        base->on_load(context);
        CHECK(element.load_calls == 1);
        CHECK(element.last_context == &context);
    }

    TEST_CASE("renderable element retains inherited element display name independently") {
        MockRenderableElementContract element;
        element.set_name("Renderable Probe");
        CHECK(element.get_name() == "Renderable Probe");
        CHECK(std::string(element.get_class_name()) == "MockRenderableElementContract");
    }

    TEST_CASE("renderable element owner transport remains inherited from Element") {
        MockRenderableElementContract element;
        Salix::Entity owner;
        element.set_owner(&owner);
        CHECK(element.get_owner() == &owner);
        element.set_owner(nullptr);
        CHECK(element.get_owner() == nullptr);
    }

    TEST_CASE("renderable element identifier remains valid across render and load") {
        MockRenderableElementContract element;
        MockIRenderer renderer;
        Salix::InitContext context{};
        const auto id = element.get_id();
        element.render(&renderer);
        element.on_load(context);
        CHECK(element.get_id() == id);
        CHECK(id.is_valid());
    }

    TEST_CASE("separate renderable elements maintain independent visibility and call state") {
        MockRenderableElementContract first;
        MockRenderableElementContract second;
        first.set_visibility(false);
        first.render(nullptr);
        CHECK_FALSE(first.is_visible());
        CHECK(first.render_calls == 1);
        CHECK(second.is_visible());
        CHECK(second.render_calls == 0);
    }

    TEST_CASE("renderable element supports polymorphic destruction through Element base") {
        std::unique_ptr<Salix::Element> element = std::make_unique<MockRenderableElementContract>();
        REQUIRE(element != nullptr);
        CHECK(std::string(element->get_class_name()) == "MockRenderableElementContract");
        CHECK(element->is_visible());
    }
}


// ---- Relocated from src/tests/SalixEngine/ecs/RenderableElement2D.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/ecs/RenderableElement2D.h>
#include <Salix/ecs/Entity.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/ecs/MockRenderableElement.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::ecs::RenderableElement2DBatch1547") {
    TEST_CASE("2D renderable defaults visible through its own base layer") {
        MockRenderableElement2D element;
        Salix::RenderableElement2D* base = &element;
        CHECK(base->is_visible());
    }

    TEST_CASE("2D renderable visibility writes remain observable through RenderableElement base") {
        MockRenderableElement2D element;
        Salix::RenderableElement* parent = &element;
        element.set_visibility(false);
        CHECK_FALSE(parent->is_visible());
        parent->set_visibility(true);
        CHECK(element.is_visible());
    }

    TEST_CASE("2D renderable render dispatch receives exact renderer pointer") {
        MockRenderableElement2D element;
        MockIRenderer renderer;
        Salix::RenderableElement2D* base = &element;
        base->render(&renderer);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == &renderer);
    }

    TEST_CASE("2D renderable on load receives exact initialization context") {
        MockRenderableElement2D element;
        Salix::InitContext context{};
        Salix::RenderableElement2D* base = &element;
        base->on_load(context);
        CHECK(element.load_calls == 1);
        CHECK(element.last_context == &context);
    }

    TEST_CASE("2D renderable identity remains distinct from generic renderable mock") {
        MockRenderableElement2D two_d;
        MockRenderableElementContract generic;
        CHECK(std::string(two_d.get_class_name()) == "MockRenderableElement2D");
        CHECK(std::string(generic.get_class_name()) == "MockRenderableElementContract");
    }

    TEST_CASE("2D renderable inherited name mutation does not alter class identity") {
        MockRenderableElement2D element;
        element.set_name("Sprite Layer");
        CHECK(element.get_name() == "Sprite Layer");
        CHECK(std::string(element.get_class_name()) == "MockRenderableElement2D");
    }

    TEST_CASE("2D renderable owner pointer is inherited and nonowning") {
        MockRenderableElement2D element;
        Salix::Entity owner;
        element.set_owner(&owner);
        CHECK(element.get_owner() == &owner);
        CHECK(owner.get_transform() != nullptr);
    }

    TEST_CASE("2D renderable visibility stays unchanged by render and load calls") {
        MockRenderableElement2D element;
        Salix::InitContext context{};
        element.set_visibility(false);
        element.render(nullptr);
        element.on_load(context);
        CHECK_FALSE(element.is_visible());
        CHECK(element.render_calls == 1);
        CHECK(element.load_calls == 1);
    }

    TEST_CASE("separate 2D renderables keep independent names visibility and counters") {
        MockRenderableElement2D first;
        MockRenderableElement2D second;
        first.set_name("First");
        first.set_visibility(false);
        first.render(nullptr);
        CHECK(first.get_name() == "First");
        CHECK_FALSE(first.is_visible());
        CHECK(second.get_name() == "Element");
        CHECK(second.is_visible());
        CHECK(second.render_calls == 0);
    }

    TEST_CASE("2D renderable supports polymorphic destruction through RenderableElement base") {
        std::unique_ptr<Salix::RenderableElement> element =
            std::make_unique<MockRenderableElement2D>();
        REQUIRE(element != nullptr);
        CHECK(std::string(element->get_class_name()) == "MockRenderableElement2D");
        CHECK(element->is_visible());
    }
}
