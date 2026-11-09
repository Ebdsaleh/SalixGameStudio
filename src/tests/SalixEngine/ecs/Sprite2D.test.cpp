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

TEST_SUITE("Salix::ecs::Sprite2DBatch2347") {
    TEST_CASE("batch2347 empty on_load path is safe without an asset manager") {
        Salix::Sprite2D sprite; Salix::InitContext context{}; CHECK(sprite.get_texture_path().empty()); CHECK_NOTHROW(sprite.on_load(context)); CHECK(sprite.get_texture()==nullptr);
    }
    TEST_CASE("batch2347 successful load stores portable path and runtime dimensions") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite;
        sprite.load_texture(&assets,"art/batch2347.png"); REQUIRE(sprite.get_texture()!=nullptr); CHECK(sprite.get_texture_path()=="art/batch2347.png"); CHECK(sprite.get_texture_width()>0); CHECK(sprite.get_texture_height()>0);
    }
    TEST_CASE("batch2347 failed replacement clears runtime resource but keeps attempted portable path") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite;
        sprite.load_texture(&assets,"art/good2347.png"); REQUIRE(sprite.get_texture()!=nullptr); renderer.should_texture_load_fail=true; sprite.load_texture(&assets,"art/missing2347.png");
        CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_width()==0); CHECK(sprite.get_texture_height()==0); CHECK(sprite.get_texture_path()=="art/missing2347.png");
    }
    TEST_CASE("batch2347 render without loaded texture never reaches renderer") {
        MockIRenderer renderer; Salix::Entity entity; auto* sprite=entity.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr); sprite->render(&renderer); CHECK(renderer.draw_sprite_call_count==0);
    }
    TEST_CASE("batch2347 render without owner never reaches renderer even with loaded texture") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/no-owner2347.png"); REQUIRE(sprite.get_texture()!=nullptr);
        sprite.render(&renderer); CHECK(renderer.draw_sprite_call_count==0);
    }
    TEST_CASE("batch2347 horizontal flip branch still emits one draw") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity entity; auto* sprite=entity.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/h2347.png"); sprite->set_flip_h(true); sprite->set_flip_v(false); sprite->render(&renderer); CHECK(renderer.draw_sprite_call_count==1);
    }
    TEST_CASE("batch2347 vertical flip branch still emits one draw") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity entity; auto* sprite=entity.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/v2347.png"); sprite->set_flip_h(false); sprite->set_flip_v(true); sprite->render(&renderer); CHECK(renderer.draw_sprite_call_count==1);
    }
    TEST_CASE("batch2347 both-flips branch still emits one draw") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity entity; auto* sprite=entity.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/both2347.png"); sprite->set_flip_h(true); sprite->set_flip_v(true); sprite->render(&renderer); CHECK(renderer.draw_sprite_call_count==1);
    }
    TEST_CASE("batch2347 unflipped branch emits one draw and preserves flags") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity entity; auto* sprite=entity.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/none2347.png"); sprite->set_flip_h(false); sprite->set_flip_v(false); sprite->render(&renderer); CHECK(renderer.draw_sprite_call_count==1); CHECK_FALSE(sprite->get_flip_h()); CHECK_FALSE(sprite->get_flip_v());
    }
    TEST_CASE("batch2347 null renderer leaves loaded runtime texture and dimensions untouched") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity entity; auto* sprite=entity.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/null-renderer2347.png"); auto* texture=sprite->get_texture(); const int width=sprite->get_texture_width(),height=sprite->get_texture_height(); sprite->render(nullptr);
        CHECK(sprite->get_texture()==texture); CHECK(sprite->get_texture_width()==width); CHECK(sprite->get_texture_height()==height);
    }
}

TEST_SUITE("Salix::ecs::Sprite2DBatch2647") {
    TEST_CASE("batch2647 empty direct load on fresh sprite keeps runtime texture empty") {
        Salix::Sprite2D sprite;
        sprite.load_texture(nullptr,"");
        CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_width()==0); CHECK(sprite.get_texture_height()==0); CHECK(sprite.get_texture_path().empty());
    }
    TEST_CASE("batch2647 successful direct load records renderer path and dimensions") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite;
        sprite.load_texture(&assets,"art/batch2647-direct.png");
        REQUIRE(sprite.get_texture()!=nullptr); CHECK(sprite.get_texture_width()==16); CHECK(sprite.get_texture_height()==16);
        CHECK(sprite.get_texture_path()=="art/batch2647-direct.png"); CHECK(renderer.last_texture_path.find("batch2647-direct.png")!=std::string::npos);
    }
    TEST_CASE("batch2647 repeated same-path load reuses asset cache and texture pointer") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite;
        sprite.load_texture(&assets,"art/cache2647.png"); auto* first=sprite.get_texture(); REQUIRE(first!=nullptr);
        sprite.load_texture(&assets,"art/cache2647.png"); CHECK(sprite.get_texture()==first); CHECK(renderer.load_texture_calls==1);
    }
    TEST_CASE("batch2647 different texture path replaces runtime texture through asset manager") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite;
        sprite.load_texture(&assets,"art/a2647.png"); auto* first=sprite.get_texture(); REQUIRE(first!=nullptr);
        sprite.load_texture(&assets,"art/b2647.png"); REQUIRE(sprite.get_texture()!=nullptr); CHECK(sprite.get_texture()!=first); CHECK(renderer.load_texture_calls==2);
    }
    TEST_CASE("batch2647 on load resolves preconfigured portable path through supplied asset manager") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.set_texture_path("art/onload2647.png");
        Salix::InitContext context{}; context.asset_manager=&assets; sprite.on_load(context);
        REQUIRE(sprite.get_texture()!=nullptr); CHECK(sprite.get_texture_width()==16); CHECK(sprite.get_texture_height()==16); CHECK(renderer.load_texture_calls==1);
    }
    TEST_CASE("batch2647 empty portable path on load leaves an already loaded runtime texture untouched") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/persist2647.png");
        auto* texture=sprite.get_texture(); REQUIRE(texture!=nullptr); sprite.set_texture_path(""); Salix::InitContext context{}; context.asset_manager=&assets; sprite.on_load(context);
        CHECK(sprite.get_texture()==texture); CHECK(sprite.get_texture_width()==16); CHECK(sprite.get_texture_height()==16); CHECK(renderer.load_texture_calls==1);
    }
    TEST_CASE("batch2647 unflipped render forwards exact texture transform and color") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity owner; auto* sprite=owner.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->set_color(Salix::Color(0.1f,0.2f,0.3f,0.4f)); sprite->load_texture(&assets,"art/render2647.png"); auto* texture=sprite->get_texture(); REQUIRE(texture!=nullptr);
        sprite->render(&renderer); CHECK(renderer.draw_sprite_call_count==1); CHECK(renderer.last_texture==texture); CHECK(renderer.last_transform==owner.get_transform()); CHECK(renderer.last_color==sprite->get_color()); CHECK(renderer.last_flip_state==Salix::SpriteFlip::None);
    }
    TEST_CASE("batch2647 horizontal flip render forwards horizontal state") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity owner; auto* sprite=owner.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/h2647.png"); sprite->set_flip_h(true); sprite->set_flip_v(false); sprite->render(&renderer);
        CHECK(renderer.draw_sprite_call_count==1); CHECK(renderer.last_flip_state==Salix::SpriteFlip::Horizontal);
    }
    TEST_CASE("batch2647 vertical flip render forwards vertical state") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity owner; auto* sprite=owner.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/v2647.png"); sprite->set_flip_h(false); sprite->set_flip_v(true); sprite->render(&renderer);
        CHECK(renderer.draw_sprite_call_count==1); CHECK(renderer.last_flip_state==Salix::SpriteFlip::Vertical);
    }
    TEST_CASE("batch2647 dual flip render forwards both state") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity owner; auto* sprite=owner.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/both2647.png"); sprite->set_flip_h(true); sprite->set_flip_v(true); sprite->render(&renderer);
        CHECK(renderer.draw_sprite_call_count==1); CHECK(renderer.last_flip_state==Salix::SpriteFlip::Both);
    }
}

TEST_SUITE("Salix::ecs::Sprite2DBatch2947") {
    TEST_CASE("batch2947 mutable property references update underlying sprite state") {
        Salix::Sprite2D sprite; sprite.get_local_rotation()=12.5f; sprite.get_flip_h()=true; sprite.get_flip_v()=true; sprite.get_sorting_layer()=-7;
        CHECK(sprite.get_local_rotation()==doctest::Approx(12.5f)); CHECK(sprite.get_flip_h()); CHECK(sprite.get_flip_v()); CHECK(sprite.get_sorting_layer()==-7);
    }
    TEST_CASE("batch2947 mutable vector and color references update underlying sprite state") {
        Salix::Sprite2D sprite; sprite.get_offset()=Salix::Vector2(-3.f,4.f); sprite.get_pivot()=Salix::Vector2(0.25f,0.75f); sprite.get_color()=Salix::Color(0.1f,0.2f,0.3f,0.4f);
        CHECK(sprite.get_offset()==Salix::Vector2(-3.f,4.f)); CHECK(sprite.get_pivot()==Salix::Vector2(0.25f,0.75f)); CHECK(sprite.get_color()==Salix::Color(0.1f,0.2f,0.3f,0.4f));
    }
    TEST_CASE("batch2947 texture path setter alone never synthesizes runtime texture state") {
        Salix::Sprite2D sprite; sprite.set_texture_path("art/not-loaded2947.png");
        CHECK(sprite.get_texture_path()=="art/not-loaded2947.png"); CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_width()==0); CHECK(sprite.get_texture_height()==0);
    }
    TEST_CASE("batch2947 failed texture load preserves requested portable path while clearing runtime dimensions") {
        MockIRenderer renderer; renderer.should_texture_load_fail=true; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite;
        sprite.load_texture(&assets,"art/fail2947.png"); CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_width()==0); CHECK(sprite.get_texture_height()==0); CHECK(sprite.get_texture_path()=="art/fail2947.png");
    }
    TEST_CASE("batch2947 successful load after failed load recovers runtime texture and dimensions") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite;
        renderer.should_texture_load_fail=true; sprite.load_texture(&assets,"art/recover2947.png"); REQUIRE(sprite.get_texture()==nullptr);
        renderer.should_texture_load_fail=false; sprite.load_texture(&assets,"art/recover2947.png"); REQUIRE(sprite.get_texture()!=nullptr); CHECK(sprite.get_texture_width()==16); CHECK(sprite.get_texture_height()==16);
    }
    TEST_CASE("batch2947 empty direct load clears runtime texture without erasing prior portable path") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/keep-path2947.png"); REQUIRE(sprite.get_texture()!=nullptr);
        const auto path_before=sprite.get_texture_path(); sprite.load_texture(&assets,""); CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_width()==0); CHECK(sprite.get_texture_height()==0); CHECK(sprite.get_texture_path()==path_before);
    }
    TEST_CASE("batch2947 render without owner performs no draw despite valid texture") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/ownerless2947.png"); REQUIRE(sprite.get_texture()!=nullptr);
        sprite.render(&renderer); CHECK(renderer.draw_sprite_call_count==0);
    }
    TEST_CASE("batch2947 loaded sprite with owner and null renderer performs no draw side effects") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Entity owner; auto* sprite=owner.add_element<Salix::Sprite2D>(); REQUIRE(sprite!=nullptr);
        sprite->load_texture(&assets,"art/null2947.png"); REQUIRE(sprite->get_texture()!=nullptr); sprite->render(nullptr); CHECK(renderer.draw_sprite_call_count==0);
    }
    TEST_CASE("batch2947 sorting and transform-independent properties survive successful texture load") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.set_sorting_layer(2947); sprite.set_local_rotation(-33.f); sprite.set_use_entity_rotation(true);
        sprite.load_texture(&assets,"art/state2947.png"); REQUIRE(sprite.get_texture()!=nullptr); CHECK(sprite.get_sorting_layer()==2947); CHECK(sprite.get_local_rotation()==doctest::Approx(-33.f)); CHECK(sprite.get_use_entity_rotation());
    }
    TEST_CASE("batch2947 on_load with empty path does not call renderer or change default dimensions") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; Salix::InitContext context{}; context.asset_manager=&assets;
        sprite.on_load(context); CHECK(renderer.load_texture_calls==0); CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_width()==0); CHECK(sprite.get_texture_height()==0);
    }
}

TEST_SUITE("Salix::ecs::Sprite2DBatch3347") {
    TEST_CASE("batch3347 mutable scalar references write directly into sprite state") {
        Salix::Sprite2D sprite; sprite.get_use_entity_rotation()=true; sprite.get_local_rotation()=33.47f; sprite.get_flip_h()=true; sprite.get_flip_v()=true; sprite.get_sorting_layer()=-3347;
        CHECK(sprite.get_use_entity_rotation()); CHECK(sprite.get_local_rotation()==doctest::Approx(33.47f)); CHECK(sprite.get_flip_h()); CHECK(sprite.get_flip_v()); CHECK(sprite.get_sorting_layer()==-3347);
    }
    TEST_CASE("batch3347 mutable structured references write directly into sprite state") {
        Salix::Sprite2D sprite; sprite.get_color()=Salix::Color(.1f,.2f,.3f,.4f); sprite.get_offset()=Salix::Vector2(-3.f,4.f); sprite.get_pivot()=Salix::Vector2(.25f,.75f);
        CHECK(sprite.get_color()==Salix::Color(.1f,.2f,.3f,.4f)); CHECK(sprite.get_offset()==Salix::Vector2(-3.f,4.f)); CHECK(sprite.get_pivot()==Salix::Vector2(.25f,.75f));
    }
    TEST_CASE("batch3347 const getters observe all latest public property mutations") {
        Salix::Sprite2D sprite; sprite.set_use_entity_rotation(true); sprite.set_local_rotation(-90.f); sprite.set_flip_h(true); sprite.set_sorting_layer(42); const Salix::Sprite2D& view=sprite;
        CHECK(view.get_use_entity_rotation()); CHECK(view.get_local_rotation()==doctest::Approx(-90.f)); CHECK(view.get_flip_h()); CHECK(view.get_sorting_layer()==42);
    }
    TEST_CASE("batch3347 texture path metadata alone never creates runtime texture state") {
        Salix::Sprite2D sprite; sprite.set_texture_path("art/metadata3347.png"); CHECK(sprite.get_texture_path()=="art/metadata3347.png"); CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_width()==0); CHECK(sprite.get_texture_height()==0);
    }
    TEST_CASE("batch3347 successful direct load stores path texture and mock dimensions") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/success3347.png");
        REQUIRE(sprite.get_texture()!=nullptr); CHECK(sprite.get_texture_path()=="art/success3347.png"); CHECK(sprite.get_texture_width()==16); CHECK(sprite.get_texture_height()==16);
    }
    TEST_CASE("batch3347 lexical aliases reuse the same runtime texture through asset cache") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/ui/icon3347.png"); auto* first=sprite.get_texture(); REQUIRE(first);
        sprite.load_texture(&assets,"art/ui/./icon3347.png"); CHECK(sprite.get_texture()==first); CHECK(sprite.get_texture_width()==16); CHECK(sprite.get_texture_height()==16);
    }
    TEST_CASE("batch3347 empty direct load clears runtime resource but preserves prior portable path") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/prior3347.png"); REQUIRE(sprite.get_texture()!=nullptr);
        sprite.load_texture(&assets,""); CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_width()==0); CHECK(sprite.get_texture_height()==0); CHECK(sprite.get_texture_path()=="art/prior3347.png");
    }
    TEST_CASE("batch3347 empty on load is safe with completely empty initialization context") {
        Salix::Sprite2D sprite; Salix::InitContext context{}; CHECK_NOTHROW(sprite.on_load(context)); CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_path().empty());
    }
    TEST_CASE("batch3347 configured path on load resolves through supplied asset manager") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::InitContext context{}; context.asset_manager=&assets; Salix::Sprite2D sprite; sprite.set_texture_path("art/onload3347.png");
        sprite.on_load(context); REQUIRE(sprite.get_texture()!=nullptr); CHECK(sprite.get_texture_width()==16); CHECK(sprite.get_texture_height()==16); CHECK(sprite.get_texture_path()=="art/onload3347.png");
    }
    TEST_CASE("batch3347 failed replacement records attempted path while clearing runtime dimensions") {
        MockIRenderer renderer; Salix::AssetManager assets; assets.initialize(&renderer); Salix::Sprite2D sprite; sprite.load_texture(&assets,"art/good3347.png"); REQUIRE(sprite.get_texture()!=nullptr);
        renderer.should_texture_load_fail=true; sprite.load_texture(&assets,"art/missing3347.png"); CHECK(sprite.get_texture()==nullptr); CHECK(sprite.get_texture_width()==0); CHECK(sprite.get_texture_height()==0); CHECK(sprite.get_texture_path()=="art/missing3347.png");
    }
}
TEST_SUITE("Salix::ecs::Sprite2DBatch3747") {
    TEST_CASE("batch3747 configured path on load is safe with empty context and clears runtime dimensions") {
        Salix::Sprite2D sprite;
        sprite.set_texture_path("assets/missing-context3747.png");
        Salix::InitContext context{};
        CHECK_NOTHROW(sprite.on_load(context));
        CHECK(sprite.get_texture_path() == "assets/missing-context3747.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch3747 direct nonempty load without asset manager is a recoverable null result") {
        Salix::Sprite2D sprite;
        CHECK_NOTHROW(sprite.load_texture(nullptr, "assets/null-manager3747.png"));
        CHECK(sprite.get_texture_path() == "assets/null-manager3747.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch3747 null manager replacement clears prior runtime texture and records requested path") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/loaded3747.png");
        REQUIRE(sprite.get_texture() != nullptr);
        REQUIRE(sprite.get_texture_width() > 0);
        sprite.load_texture(nullptr, "assets/unresolved3747.png");
        CHECK(sprite.get_texture_path() == "assets/unresolved3747.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch3747 valid manager recovers after null manager failure on same path") {
        Salix::Sprite2D sprite;
        const std::string path = "assets/recover3747.png";
        sprite.load_texture(nullptr, path);
        REQUIRE(sprite.get_texture() == nullptr);
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        sprite.load_texture(&assets, path);
        REQUIRE(sprite.get_texture() != nullptr);
        CHECK(sprite.get_texture_width() == 16);
        CHECK(sprite.get_texture_height() == 16);
        CHECK(renderer.load_texture_calls == 1);
    }

    TEST_CASE("batch3747 null manager load preserves all nontexture sprite properties") {
        Salix::Sprite2D sprite;
        sprite.set_color(Salix::Color(-1.0f, 2.0f, 3.0f, 0.25f));
        sprite.set_offset(Salix::Vector2(-3.0f, 4.0f));
        sprite.set_pivot(Salix::Vector2(0.25f, 0.75f));
        sprite.set_local_rotation(-37.47f);
        sprite.set_flip_h(true);
        sprite.set_flip_v(true);
        sprite.set_sorting_layer(-3747);
        sprite.set_use_entity_rotation(true);
        sprite.load_texture(nullptr, "assets/properties3747.png");
        CHECK(sprite.get_color() == Salix::Color(-1.0f, 2.0f, 3.0f, 0.25f));
        CHECK(sprite.get_offset() == Salix::Vector2(-3.0f, 4.0f));
        CHECK(sprite.get_pivot() == Salix::Vector2(0.25f, 0.75f));
        CHECK(sprite.get_local_rotation() == doctest::Approx(-37.47f));
        CHECK(sprite.get_flip_h());
        CHECK(sprite.get_flip_v());
        CHECK(sprite.get_sorting_layer() == -3747);
        CHECK(sprite.get_use_entity_rotation());
    }

    TEST_CASE("batch3747 owner and renderer cannot draw after null manager clears runtime resource") {
        MockIRenderer asset_renderer;
        Salix::AssetManager assets;
        assets.initialize(&asset_renderer);
        Salix::Entity owner;
        auto* sprite = owner.add_element<Salix::Sprite2D>();
        REQUIRE(sprite != nullptr);
        sprite->load_texture(&assets, "assets/drawable3747.png");
        REQUIRE(sprite->get_texture() != nullptr);
        sprite->load_texture(nullptr, "assets/not-drawable3747.png");
        MockIRenderer draw_renderer;
        sprite->render(&draw_renderer);
        CHECK(draw_renderer.draw_sprite_call_count == 0);
    }

    TEST_CASE("batch3747 embedded null bytes in unresolved texture path are preserved exactly") {
        Salix::Sprite2D sprite;
        const std::string path("asset\0hidden3747.png", 20);
        sprite.load_texture(nullptr, path);
        CHECK(sprite.get_texture_path() == path);
        CHECK(sprite.get_texture_path().size() == path.size());
        CHECK(sprite.get_texture() == nullptr);
    }

    TEST_CASE("batch3747 whitespace-only unresolved texture path is preserved as nonempty metadata") {
        Salix::Sprite2D sprite;
        const std::string path = "   \t  ";
        sprite.load_texture(nullptr, path);
        CHECK(sprite.get_texture_path() == path);
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
    }

    TEST_CASE("batch3747 empty path with null manager clears runtime resource but preserves prior portable path") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/keep-path3747.png");
        REQUIRE(sprite.get_texture() != nullptr);
        const std::string stored_path = sprite.get_texture_path();
        sprite.load_texture(nullptr, "");
        CHECK(sprite.get_texture_path() == stored_path);
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch3747 on load with null manager after successful load clears stale runtime texture") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/reload-null3747.png");
        REQUIRE(sprite.get_texture() != nullptr);
        Salix::InitContext empty_context{};
        sprite.on_load(empty_context);
        CHECK(sprite.get_texture_path() == "assets/reload-null3747.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }
}

TEST_SUITE("Salix::ecs::Sprite2DBatch4147") {
    TEST_CASE("batch4147 fresh sprite combines canonical identity with empty runtime texture state") {
        Salix::Sprite2D sprite;
        CHECK(std::string(sprite.get_class_name()) == "Sprite2D");
        CHECK(sprite.get_name() == "Sprite2D");
        CHECK(sprite.get_texture_path().empty());
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch4147 public render metadata setters remain mutually independent") {
        Salix::Sprite2D sprite;
        sprite.set_color(Salix::Color(0.2f, 0.4f, 0.6f, 0.8f));
        sprite.set_offset(Salix::Vector2(-4.0f, 7.0f));
        sprite.set_pivot(Salix::Vector2(0.1f, 0.9f));
        sprite.set_local_rotation(41.47f);
        sprite.set_flip_h(true);
        sprite.set_flip_v(false);
        sprite.set_sorting_layer(-4147);
        sprite.set_use_entity_rotation(true);
        CHECK(sprite.get_color() == Salix::Color(0.2f, 0.4f, 0.6f, 0.8f));
        CHECK(sprite.get_offset() == Salix::Vector2(-4.0f, 7.0f));
        CHECK(sprite.get_pivot() == Salix::Vector2(0.1f, 0.9f));
        CHECK(sprite.get_local_rotation() == doctest::Approx(41.47f));
        CHECK(sprite.get_flip_h());
        CHECK_FALSE(sprite.get_flip_v());
        CHECK(sprite.get_sorting_layer() == -4147);
        CHECK(sprite.get_use_entity_rotation());
    }

    TEST_CASE("batch4147 const getters observe latest mutable sprite metadata") {
        Salix::Sprite2D sprite;
        sprite.set_local_rotation(-12.5f);
        sprite.set_flip_h(true);
        sprite.set_sorting_layer(47);
        const Salix::Sprite2D& view = sprite;
        CHECK(view.get_local_rotation() == doctest::Approx(-12.5f));
        CHECK(view.get_flip_h());
        CHECK(view.get_sorting_layer() == 47);
    }

    TEST_CASE("batch4147 texture path metadata writes never synthesize runtime resource state") {
        Salix::Sprite2D sprite;
        sprite.set_texture_path("assets/first4147.png");
        sprite.set_texture_path("assets/second4147.png");
        CHECK(sprite.get_texture_path() == "assets/second4147.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch4147 nonempty load without asset manager records path and clears runtime state") {
        Salix::Sprite2D sprite;
        sprite.load_texture(nullptr, "assets/null4147.png");
        CHECK(sprite.get_texture_path() == "assets/null4147.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch4147 null manager replacement clears a previously loaded runtime texture") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/good4147.png");
        REQUIRE(sprite.get_texture() != nullptr);
        sprite.load_texture(nullptr, "assets/missing4147.png");
        CHECK(sprite.get_texture_path() == "assets/missing4147.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch4147 valid manager recovers after prior null manager failure") {
        Salix::Sprite2D sprite;
        const std::string path = "assets/recover4147.png";
        sprite.load_texture(nullptr, path);
        REQUIRE(sprite.get_texture() == nullptr);
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        sprite.load_texture(&assets, path);
        REQUIRE(sprite.get_texture() != nullptr);
        CHECK(sprite.get_texture_width() == 16);
        CHECK(sprite.get_texture_height() == 16);
    }

    TEST_CASE("batch4147 empty direct load clears resource while preserving prior portable path") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/preserved4147.png");
        REQUIRE(sprite.get_texture() != nullptr);
        sprite.load_texture(&assets, "");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
        CHECK(sprite.get_texture_path() == "assets/preserved4147.png");
    }

    TEST_CASE("batch4147 loaded ownerless sprite never submits a draw") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/ownerless4147.png");
        REQUIRE(sprite.get_texture() != nullptr);
        sprite.render(&renderer);
        CHECK(renderer.draw_sprite_call_count == 0);
    }

    TEST_CASE("batch4147 loaded owned sprite submits one draw through provided renderer") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Entity owner;
        auto* sprite = owner.add_element<Salix::Sprite2D>();
        REQUIRE(sprite != nullptr);
        sprite->load_texture(&assets, "assets/owned4147.png");
        REQUIRE(sprite->get_texture() != nullptr);
        sprite->render(&renderer);
        CHECK(renderer.draw_sprite_call_count == 1);
    }
}

TEST_SUITE("Salix::ecs::Sprite2DBatch4547") {
    TEST_CASE("batch4547 mutable reference getters update all primary render metadata in place") {
        Salix::Sprite2D sprite;
        sprite.get_use_entity_rotation() = true;
        sprite.get_local_rotation() = -45.47f;
        sprite.get_color() = Salix::Color(0.1f, 0.2f, 0.3f, 0.4f);
        sprite.get_offset() = Salix::Vector2(-4.0f, 5.0f);
        sprite.get_pivot() = Salix::Vector2(0.25f, 0.75f);
        CHECK(sprite.get_use_entity_rotation());
        CHECK(sprite.get_local_rotation() == doctest::Approx(-45.47f));
        CHECK(sprite.get_color() == Salix::Color(0.1f, 0.2f, 0.3f, 0.4f));
        CHECK(sprite.get_offset() == Salix::Vector2(-4.0f, 5.0f));
        CHECK(sprite.get_pivot() == Salix::Vector2(0.25f, 0.75f));
    }

    TEST_CASE("batch4547 mutable flip and sorting references update stored values") {
        Salix::Sprite2D sprite;
        sprite.get_flip_h() = true;
        sprite.get_flip_v() = true;
        sprite.get_sorting_layer() = 4547;
        CHECK(sprite.get_flip_h());
        CHECK(sprite.get_flip_v());
        CHECK(sprite.get_sorting_layer() == 4547);
    }

    TEST_CASE("batch4547 texture path setter copies caller storage without creating runtime texture") {
        Salix::Sprite2D sprite;
        std::string path = "assets/copy4547.png";
        sprite.set_texture_path(path);
        path = "mutated.png";
        CHECK(sprite.get_texture_path() == "assets/copy4547.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch4547 metadata path replacement after successful load preserves existing runtime resource") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/runtime4547.png");
        auto* texture = sprite.get_texture();
        REQUIRE(texture != nullptr);
        const int width = sprite.get_texture_width();
        const int height = sprite.get_texture_height();
        sprite.set_texture_path("assets/metadata-only4547.png");
        CHECK(sprite.get_texture_path() == "assets/metadata-only4547.png");
        CHECK(sprite.get_texture() == texture);
        CHECK(sprite.get_texture_width() == width);
        CHECK(sprite.get_texture_height() == height);
    }

    TEST_CASE("batch4547 repeated load of identical path reuses AssetManager cache") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/cache4547.png");
        REQUIRE(sprite.get_texture() != nullptr);
        auto* first = sprite.get_texture();
        REQUIRE(renderer.load_texture_calls == 1);
        sprite.load_texture(&assets, "assets/cache4547.png");
        CHECK(sprite.get_texture() == first);
        CHECK(renderer.load_texture_calls == 1);
    }

    TEST_CASE("batch4547 loading a second path updates portable path and requests another texture") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/first4547.png");
        REQUIRE(sprite.get_texture() != nullptr);
        sprite.load_texture(&assets, "assets/second4547.png");
        REQUIRE(sprite.get_texture() != nullptr);
        CHECK(sprite.get_texture_path() == "assets/second4547.png");
        CHECK(renderer.load_texture_calls == 2);
    }

    TEST_CASE("batch4547 renderer load failure records requested path and clears runtime dimensions") {
        MockIRenderer renderer;
        renderer.should_texture_load_fail = true;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/fail4547.png");
        CHECK(sprite.get_texture_path() == "assets/fail4547.png");
        CHECK(sprite.get_texture() == nullptr);
        CHECK(sprite.get_texture_width() == 0);
        CHECK(sprite.get_texture_height() == 0);
    }

    TEST_CASE("batch4547 failed texture request can recover on same path after renderer resumes") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        renderer.should_texture_load_fail = true;
        sprite.load_texture(&assets, "assets/retry4547.png");
        REQUIRE(sprite.get_texture() == nullptr);
        renderer.should_texture_load_fail = false;
        sprite.load_texture(&assets, "assets/retry4547.png");
        REQUIRE(sprite.get_texture() != nullptr);
        CHECK(sprite.get_texture_width() == 16);
        CHECK(sprite.get_texture_height() == 16);
        CHECK(renderer.load_texture_calls == 2);
    }

    TEST_CASE("batch4547 loaded owned sprite does not draw when renderer argument is null") {
        MockIRenderer asset_renderer;
        Salix::AssetManager assets;
        assets.initialize(&asset_renderer);
        Salix::Entity owner;
        auto* sprite = owner.add_element<Salix::Sprite2D>();
        REQUIRE(sprite != nullptr);
        sprite->load_texture(&assets, "assets/null-renderer4547.png");
        REQUIRE(sprite->get_texture() != nullptr);
        CHECK_NOTHROW(sprite->render(nullptr));
        CHECK(asset_renderer.draw_sprite_call_count == 0);
    }

    TEST_CASE("batch4547 render metadata mutations after load preserve runtime texture identity") {
        MockIRenderer renderer;
        Salix::AssetManager assets;
        assets.initialize(&renderer);
        Salix::Sprite2D sprite;
        sprite.load_texture(&assets, "assets/stable4547.png");
        auto* texture = sprite.get_texture();
        REQUIRE(texture != nullptr);
        sprite.set_color(Salix::Red);
        sprite.set_flip_h(true);
        sprite.set_flip_v(true);
        sprite.set_sorting_layer(-4547);
        sprite.set_local_rotation(90.0f);
        CHECK(sprite.get_texture() == texture);
        CHECK(sprite.get_texture_width() == 16);
        CHECK(sprite.get_texture_height() == 16);
    }
}
