// =================================================================================
// Filename:    src/Tests/SalixEngine/ecs/Entity.test.cpp
// Description: Contains unit tests for the Entity class.
// ================================================================================= 

#include <doctest.h>
#include <Salix/core/InitContext.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/BoxCollider.h>
#include <Salix/core/SimpleGuid.h>
#include <Salix/ecs/Sprite2D.h>
#include <cereal/archives/json.hpp>
#include <Salix/rendering/IRenderer.h>
#include <Salix/assets/AssetManager.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <sstream>
#include <memory>


class MockRenderableElement : public Salix::RenderableElement {
public:
    int render_call_count = 0;
    void render(Salix::IRenderer* renderer) override { (void)renderer; render_call_count++; }
    void on_load(const Salix::InitContext& context) override { (void)context; }
    const char* get_class_name() const override { return "MockRenderableElement"; }
};

TEST_SUITE("Salix::ecs::Entity") {
    
    TEST_CASE("default constructor creates a valid entity with default components") {
        // ARRANGE: Reset the GUID counter for a predictable ID.
        Salix::SimpleGuid::reset_counter_for_testing();
        std::unique_ptr<Salix::Entity> entity;
        
        // ACT: Create a new, default entity.
        entity = std::make_unique<Salix::Entity>();

        // ASSERT: Check that the entity is in the correct initial state.
        CHECK(entity->get_name().empty()); // Default name should be empty 
        CHECK(entity->is_visible() == true); // Should be visible by default 
        CHECK(entity->is_purged() == false); // Should not be purged initially 
        CHECK(entity->get_id().is_valid()); // Should have a valid ID 

        // ASSERT: Check for the existence of mandatory components.
        // The constructor automatically adds a Transform and a BoxCollider. 
        CHECK(entity->get_transform() != nullptr);
        CHECK(entity->has_element<Salix::Transform>() == true);
        CHECK(entity->has_element<Salix::BoxCollider>() == true);
        
        // ASSERT: Check that the component count is correct.
        CHECK(entity->get_all_elements().size() == 2);
    }

    TEST_CASE("adding and retrieving elements") {
        Salix::Entity entity;

        // The constructor adds Transform and BoxCollider, let's retrieve them
        Salix::Transform* transform = entity.get_element<Salix::Transform>();
        Salix::BoxCollider* collider = entity.get_element<Salix::BoxCollider>();

        // ASSERT: Check that we can retrieve existing components
        CHECK(transform != nullptr);
        CHECK(collider != nullptr);

        // ASSERT: Check that getting a non-existent component returns nullptr
        // This covers the failure path of get_element()
        Salix::Sprite2D* sprite = entity.get_element<Salix::Sprite2D>();
        CHECK(sprite == nullptr);
    }

    TEST_CASE("parent-child relationships") {
        Salix::Entity parent_entity;
        Salix::Entity child_entity;

        SUBCASE("an entity is a root by default") {
            CHECK(parent_entity.is_root() == true);
            CHECK(parent_entity.get_parent() == nullptr);
        }

        SUBCASE("setting and getting parent") {
            // ACT
            child_entity.set_parent(&parent_entity);

            // ASSERT
            CHECK(child_entity.get_parent() == &parent_entity);
            CHECK(child_entity.is_root() == false);
            CHECK(parent_entity.get_children().size() == 1);
            CHECK(parent_entity.get_children()[0] == &child_entity);
            CHECK(child_entity.is_child_of(&parent_entity) == true);
        }

        SUBCASE("releasing from parent") {
            // ARRANGE
            child_entity.set_parent(&parent_entity);
            // Pre-condition check
            REQUIRE(child_entity.get_parent() == &parent_entity);

            // ACT
            child_entity.release_from_parent();

            // ASSERT
            CHECK(child_entity.get_parent() == nullptr);
            CHECK(child_entity.is_root() == true);
            CHECK(parent_entity.get_children().empty() == true);
        }
    }

    TEST_CASE("destructor correctly orphans children") {
        Salix::Entity child_entity;
        
        // This block creates a new scope.
        {
            // The parent entity is owned by a unique_ptr.
            auto parent_entity = std::make_unique<Salix::Entity>();
            child_entity.set_parent(parent_entity.get());

            // Pre-condition check: ensure the relationship is set up.
            REQUIRE(child_entity.get_parent() == parent_entity.get());
        
        } // <-- parent_entity's destructor is called here as it goes out of scope.

        // ASSERT: The child should now be an orphan with no parent.
        // This covers the child-orphaning logic in the Entity destructor.
        CHECK(child_entity.get_parent() == nullptr);
    }

    TEST_CASE("purge correctly marks an entity for removal") {
        // ARRANGE
        Salix::Entity entity;

        // ACT
        entity.purge();

        // ASSERT
        CHECK(entity.is_purged() == true);
    }

    TEST_CASE("retrieving elements by name, type, and ID") {
        // ARRANGE
        Salix::Entity entity;
        Salix::Transform* transform = entity.get_element<Salix::Transform>();
        Salix::BoxCollider* collider = entity.get_element<Salix::BoxCollider>();
        
        // Give one of the elements a unique name
        collider->set_name("MyCollider");

        // ACT & ASSERT
        // by name
        CHECK(entity.get_element_by_name("MyCollider") == collider);
        CHECK(entity.get_element_by_name("NonExistentName") == nullptr);

        // by type name (string)
        CHECK(entity.get_element_by_type_name("Transform") == transform);
        CHECK(entity.get_element_by_type_name("Sprite2D") == nullptr);

        // by ID
        CHECK(entity.get_element_by_id(transform->get_id()) == transform);
        CHECK(entity.get_element_by_id(Salix::SimpleGuid::invalid()) == nullptr);
    }

    TEST_CASE("advanced element management and lifecycle") {
        Salix::Entity entity;

        SUBCASE("adding a renderable element") {
            // This test covers the `if (renderable)` branch inside add_element_internal
            // and the get_elements_by_type_name method.
            
            // ACT
            entity.add_element<Salix::Sprite2D>();

            // ASSERT
            CHECK(entity.has_element<Salix::Sprite2D>() == true);
            
            // get_elements_by_type_name should now find one Sprite2D
            std::vector<Salix::Element*> sprites = entity.get_elements_by_type_name("Sprite2D");
            CHECK(sprites.size() == 1);

            // Check for a type that doesn't exist to cover the empty return path
            std::vector<Salix::Element*> cameras = entity.get_elements_by_type_name("Camera");
            CHECK(cameras.empty() == true);
        }

         SUBCASE("purging a parent orphans its immediate children") {
            // ARRANGE
            Salix::Entity parent;
            Salix::Entity child;
            child.set_parent(&parent);
            REQUIRE(child.get_parent() == &parent);

            // ACT
            parent.purge();

            // ASSERT
            CHECK(parent.is_purged() == true);
            
            // The child should now be a root object with no parent.
            CHECK(child.get_parent() == nullptr);
            
            // Crucially, the child itself should NOT be flagged for purging.
            CHECK(child.is_purged() == false);
        }

        SUBCASE("adding an element via raw pointer") {
            // This test covers the add_element(Element*) overload.
            
            // ACT
            // Note: Using 'new' here is okay because the Entity takes ownership.
            entity.add_element(new Salix::Sprite2D());

            // ASSERT
            CHECK(entity.has_element<Salix::Sprite2D>() == true);
        }
        
        SUBCASE("setting name and visibility") {
            // This covers the simple setters and getters.

            // ACT
            entity.set_name("Player");
            entity.set_visible(false);

            // ASSERT
            CHECK(entity.get_name() == "Player");
            CHECK(entity.is_visible() == false);
        }
    }

     TEST_CASE("simple_purge only flags the entity itself for removal") {
        // ARRANGE
        Salix::Entity parent;
        Salix::Entity child;
        child.set_parent(&parent);
        REQUIRE(child.get_parent() == &parent);

        // ACT
        parent.simple_purge();

        // ASSERT
        // The parent should be flagged for purging.
        CHECK(parent.is_purged() == true);

        // The child should NOT be flagged for purging.
        CHECK(child.is_purged() == false);
        
        // Crucially, the child should still be attached to its parent.
        // The relationship is only broken later when the Scene's maintain() method
        // destroys the parent and the parent's destructor runs.
        CHECK(child.get_parent() == &parent);
    }
    
    TEST_CASE("hierarchy safety checks and edge cases") {
        SUBCASE("cannot set an entity as its own parent") {
            Salix::Entity entity;
            entity.set_parent(&entity);
            CHECK(entity.get_parent() == nullptr);
        }

        SUBCASE("cannot create a circular dependency") {
            Salix::Entity grandparent;
            Salix::Entity parent;
            Salix::Entity child;

            parent.set_parent(&grandparent);
            child.set_parent(&parent);

            // ACT: Try to set the grandparent's parent to the child
            grandparent.set_parent(&child);

            // ASSERT: The operation should have failed.
            CHECK(grandparent.get_parent() == nullptr);
        }

        SUBCASE("is_child_of works for grandchildren") {
            Salix::Entity grandparent;
            Salix::Entity parent;
            Salix::Entity child;
            parent.set_parent(&grandparent);
            child.set_parent(&parent);

            CHECK(child.is_child_of(&grandparent) == true);
        }

        SUBCASE("const get_element returns nullptr for non-existent types") {
            // This test is specifically to cover the return path on the CONST overload
            const Salix::Entity entity; // Create a const entity
            // Calling get_element on a const object will call the const overload
            const Salix::Sprite2D* sprite = entity.get_element<Salix::Sprite2D>();
            CHECK(sprite == nullptr);
        }
    }

    TEST_CASE("lifecycle methods and overloads") {
        Salix::Entity entity;

        // ACT: Call the lifecycle methods. Since they are simple loops,
        // we just need to call them to get coverage.
        entity.update(0.016f);
        entity.render(nullptr); // Pass nullptr as we don't need a real renderer

        // ACT: Test the unique_ptr overload for add_element
        auto sprite_ptr = std::make_unique<Salix::Sprite2D>();
        // Get the raw pointer before we move ownership away
        Salix::Sprite2D* raw_sprite_ptr = sprite_ptr.get(); 
        
        entity.add_element(std::move(sprite_ptr));

        // ASSERT
        CHECK(entity.has_element<Salix::Sprite2D>() == true);
        CHECK(entity.get_element<Salix::Sprite2D>() == raw_sprite_ptr);
        CHECK(sprite_ptr == nullptr); // Check that ownership was correctly moved
    }

    TEST_CASE("final edge cases and helper methods") {
    
        SUBCASE("render calls render on renderable elements") {
            // ARRANGE
            Salix::Entity entity;
            MockRenderableElement* renderable = entity.add_element<MockRenderableElement>();
            
            // ACT
            entity.render(nullptr); // Pass a nullptr, the mock doesn't use it.

            // ASSERT
            CHECK(renderable->render_call_count == 1);
        }

        SUBCASE("get_element_by_id returns nullptr for non-existent ID") {
            Salix::Entity entity;
            Salix::SimpleGuid non_existent_id = Salix::SimpleGuid::generate();

            CHECK(entity.get_element_by_id(non_existent_id) == nullptr);
        }

        SUBCASE("add_element handles nullptr gracefully") {
            Salix::Entity entity;
            size_t initial_count = entity.get_all_elements().size();

            // Test the raw pointer overload
            entity.add_element(static_cast<Salix::Element*>(nullptr));
            CHECK(entity.get_all_elements().size() == initial_count);

            // Test the unique_ptr overload
            entity.add_element(std::unique_ptr<Salix::Sprite2D>(nullptr));
            CHECK(entity.get_all_elements().size() == initial_count);
        }

        SUBCASE("report_ids runs without crashing") {
            // This test's main purpose is to get coverage and ensure no crashes.
            Salix::Entity parent;
            Salix::Entity child;
            child.set_parent(&parent);
            
            // ACT: Just call the function. A passing test means it didn't crash.
            parent.report_ids();
            child.report_ids();
        }

        SUBCASE("set_id updates the entity's ID") {
            Salix::Entity entity;
            Salix::SimpleGuid original_id = entity.get_id();
            Salix::SimpleGuid new_id = Salix::SimpleGuid::generate();

            // ACT
            entity.set_id(new_id);

            // ASSERT
            CHECK(entity.get_id() == new_id);
            CHECK(entity.get_id() != original_id);
        }
    }

    TEST_CASE("can be serialized and deserialized with Cereal") {
        // ARRANGE 1: Create a valid InitContext with a working AssetManager.
        Salix::InitContext mock_context;
        MockIRenderer mock_renderer;
        Salix::AssetManager asset_manager;
        asset_manager.initialize(&mock_renderer);
        mock_context.asset_manager = &asset_manager; // Put the valid manager into the context

        // ARRANGE 2: Create the original entity to be serialized.
        auto original_entity = std::make_unique<Salix::Entity>();
        original_entity->set_name("Player");
        original_entity->get_transform()->set_position(10.f, 20.f, 30.f);
        
        auto* original_sprite = original_entity->add_element<Salix::Sprite2D>();
        // Use a real path that exists for the mock to "load"
        original_sprite->set_texture_path("assets/textures/test.png"); 

        std::stringstream ss;

        // ACT 1: Serialize the entity.
        {
            cereal::JSONOutputArchive output_archive(ss);
            output_archive(original_entity);
        }

        // ACT 2: Deserialize into a new entity.
        std::unique_ptr<Salix::Entity> loaded_entity;
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(loaded_entity);
        }

        // ACT 3: Call on_load with the fully initialized context.
        loaded_entity->on_load(mock_context);

        // ASSERT: Check that the loaded entity matches the original.
        CHECK(loaded_entity->get_name() == original_entity->get_name());
        
        Salix::Transform* loaded_transform = loaded_entity->get_transform();
        CHECK(loaded_transform->get_position().x == doctest::Approx(10.0f));

        Salix::Sprite2D* loaded_sprite = loaded_entity->get_element<Salix::Sprite2D>();
        CHECK(loaded_sprite != nullptr);
        CHECK(loaded_sprite->get_texture_path() == "assets/textures/test.png");
        
        // This is the most important check: on_load should have successfully loaded the texture.
        CHECK(loaded_sprite->get_texture() != nullptr);
    }

    TEST_CASE("private helper get_element_internal finds elements by typeid") {
        // ARRANGE
        Salix::Entity entity;
        const Salix::Entity& const_entity = entity; // Create a const reference

        // ACT & ASSERT for non-const version
        Salix::Element* found_transform = entity.get_element_internal(typeid(Salix::Transform));
        Salix::Element* not_found_sprite = entity.get_element_internal(typeid(Salix::Sprite2D));

        CHECK(found_transform != nullptr);
        CHECK(not_found_sprite == nullptr);

        // ACT & ASSERT for const version
        const Salix::Element* const_found_transform = const_entity.get_element_internal(typeid(Salix::Transform));
        const Salix::Element* const_not_found_sprite = const_entity.get_element_internal(typeid(Salix::Sprite2D));

        CHECK(const_found_transform != nullptr);
        CHECK(const_not_found_sprite == nullptr);
    }
}