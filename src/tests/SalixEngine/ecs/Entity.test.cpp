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

// Additional cases preserved from the separate regression suite.
// Entity ownership, dynamic component lookup and hierarchy integrity.
#include <doctest.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/BoxCollider.h>
#include <Salix/core/SimpleGuid.h>
#include <algorithm>
#include <memory>

TEST_SUITE("Salix::ecs::EntityRegression") {
    TEST_CASE("entity fresh entity owns mandatory transform and collider and is visible") {
        Salix::Entity entity;
        CHECK(entity.is_visible());
        CHECK_FALSE(entity.is_purged());
        REQUIRE(entity.get_transform() != nullptr);
        CHECK(entity.get_element<Salix::BoxCollider>() != nullptr);
        CHECK(entity.get_parent() == nullptr);
    }

    TEST_CASE("entity entity component enumeration contains distinct owned components") {
        Salix::Entity entity;
        const auto elements = entity.get_all_elements();
        REQUIRE(elements.size() >= 2);
        REQUIRE(entity.get_transform() != nullptr);
        CHECK(std::find(elements.begin(), elements.end(), entity.get_transform()) != elements.end());
        CHECK(std::find(elements.begin(), elements.end(), entity.get_element<Salix::BoxCollider>()) != elements.end());
    }

    TEST_CASE("entity adding a null raw element does not alter component ownership") {
        Salix::Entity entity;
        const auto before = entity.get_all_elements().size();
        entity.add_element(static_cast<Salix::Element*>(nullptr));
        CHECK(entity.get_all_elements().size() == before);
        CHECK(entity.get_transform() != nullptr);
    }

    TEST_CASE("entity raw element adoption sets owner and allows typed and name lookup") {
        Salix::Entity entity;
        auto* collider = new Salix::BoxCollider();
        collider->set_name("ColliderExtra");
        entity.add_element(collider);
        CHECK(collider->get_owner() == &entity);
        CHECK(entity.get_element_by_name("ColliderExtra") == collider);
        CHECK(entity.get_element_by_id(collider->get_id()) == collider);
    }

    TEST_CASE("entity unique pointer element addition preserves entity owner relationship") {
        Salix::Entity entity;
        auto added = std::make_unique<Salix::BoxCollider>();
        Salix::BoxCollider* raw = added.get();
        entity.add_element(std::move(added));
        CHECK(raw->get_owner() == &entity);
        CHECK(entity.get_element_by_id(raw->get_id()) == raw);
        CHECK(entity.get_elements_by_type_name("BoxCollider").size() == 2);
    }

    TEST_CASE("entity component queries reject empty unknown and invalid identifiers") {
        Salix::Entity entity;
        CHECK(entity.get_element_by_name("") == nullptr);
        CHECK(entity.get_element_by_name("NoSuchElement") == nullptr);
        CHECK(entity.get_element_by_type_name("") == nullptr);
        CHECK(entity.get_element_by_type_name("Unregistered") == nullptr);
        CHECK(entity.get_element_by_id(Salix::SimpleGuid::invalid()) == nullptr);
        CHECK(entity.get_elements_by_type_name("Missing").empty());
    }

    TEST_CASE("entity const enumeration exposes the same components without granting ownership") {
        Salix::Entity entity;
        const Salix::Entity& read_only = entity;
        const auto mutable_elements = entity.get_all_elements();
        const auto const_elements = read_only.get_all_elements();
        REQUIRE(const_elements.size() == mutable_elements.size());
        for (std::size_t i = 0; i < const_elements.size(); ++i) {
            CHECK(const_elements[i] == mutable_elements[i]);
        }
    }

    TEST_CASE("entity reparenting updates both entity and transform hierarchy") {
        Salix::Entity parent;
        Salix::Entity child;
        child.set_parent(&parent);
        CHECK(child.get_parent() == &parent);
        CHECK(child.is_child_of(&parent));
        CHECK(parent.get_children().size() == 1);
        CHECK(parent.get_children().front() == &child);
        CHECK(child.get_transform()->get_parent() == parent.get_transform());
    }

    TEST_CASE("entity switching parents drops former list membership and keeps one new edge") {
        Salix::Entity old_parent;
        Salix::Entity new_parent;
        Salix::Entity child;
        child.set_parent(&old_parent);
        child.set_parent(&new_parent);
        CHECK(old_parent.get_children().empty());
        REQUIRE(new_parent.get_children().size() == 1);
        CHECK(new_parent.get_children().front() == &child);
        CHECK(child.get_parent() == &new_parent);
        CHECK(child.get_transform()->get_parent() == new_parent.get_transform());
    }

    TEST_CASE("entity self cycle and ancestor cycle are rejected without hierarchy mutation") {
        Salix::Entity root;
        Salix::Entity child;
        Salix::Entity grandchild;
        child.set_parent(&root);
        grandchild.set_parent(&child);
        root.set_parent(&root);
        root.set_parent(&grandchild);
        CHECK(root.get_parent() == nullptr);
        CHECK(child.get_parent() == &root);
        CHECK(grandchild.get_parent() == &child);
        CHECK(grandchild.is_child_of(&root));
        CHECK_FALSE(root.is_child_of(&grandchild));
    }
}

// More entity identity, hierarchy, component type, and reload invariants.
#include <Salix/core/InitContext.h>
namespace {
    class AdditionalEntityProbe final : public Salix::Element {
    public:
        int initialized = 0;
        int loads = 0;
        int updates = 0;
        float last_delta = 0.f;
        const char* get_class_name() const override { return "AdditionalEntityProbe"; }
        void initialize() override { ++initialized; }
        void on_load(const Salix::InitContext&) override { ++loads; }
        void update(float delta) override { ++updates; last_delta = delta; }
    };
}
TEST_SUITE("Salix::ecs::EntityAdditional") {
    TEST_CASE("entity additional renaming an entity does not regenerate its persistent GUID") {
        Salix::Entity entity;
        const auto id = entity.get_id();
        entity.set_name("Player / Localized Name");
        CHECK(entity.get_name() == "Player / Localized Name");
        CHECK(entity.get_id() == id);
        CHECK(entity.get_transform() != nullptr);
    }

    TEST_CASE("entity additional reassigning entity ID does not modify owned component GUIDs") {
        Salix::Entity entity;
        REQUIRE(entity.get_transform() != nullptr);
        const auto transform_id = entity.get_transform()->get_id();
        const auto collider_id = entity.get_element<Salix::BoxCollider>()->get_id();
        entity.set_id(Salix::SimpleGuid::from_value(44332211));
        CHECK(entity.get_id().get_value() == 44332211ULL);
        CHECK(entity.get_transform()->get_id() == transform_id);
        CHECK(entity.get_element<Salix::BoxCollider>()->get_id() == collider_id);
    }

    TEST_CASE("entity additional entity visibility toggling preserves ownership and parent relationships") {
        Salix::Entity parent, child;
        child.set_parent(&parent);
        const auto count = child.get_all_elements().size();
        child.set_visible(false);
        CHECK_FALSE(child.is_visible());
        CHECK(child.get_parent() == &parent);
        CHECK(child.get_all_elements().size() == count);
        child.set_visible(true);
        CHECK(child.is_visible());
    }

    TEST_CASE("entity additional type query lists both colliders in insertion order after adding a second") {
        Salix::Entity entity;
        const auto existing = entity.get_elements_by_type_name("BoxCollider");
        REQUIRE(existing.size() == 1);
        auto* second = entity.add_element<Salix::BoxCollider>();
        REQUIRE(second != nullptr);
        const auto colliders = entity.get_elements_by_type_name("BoxCollider");
        REQUIRE(colliders.size() == 2);
        CHECK(colliders[0] == existing[0]);
        CHECK(colliders[1] == second);
    }

    TEST_CASE("entity additional renaming a component changes name lookup but preserves type lookup") {
        Salix::Entity entity;
        auto* transform = entity.get_transform();
        REQUIRE(transform != nullptr);
        transform->set_name("RootNodeTransform");
        CHECK(entity.get_element_by_name("RootNodeTransform") == transform);
        CHECK(entity.get_element_by_type_name("Transform") == transform);
        CHECK(entity.get_element_by_name("Transform") == nullptr);
    }

    TEST_CASE("entity additional element ID lookup follows explicitly restored component identity") {
        Salix::Entity entity;
        auto* collider = entity.get_element<Salix::BoxCollider>();
        REQUIRE(collider != nullptr);
        const auto old = collider->get_id();
        const auto assigned = Salix::SimpleGuid::from_value(77112233ULL);
        collider->set_id(assigned);
        CHECK(entity.get_element_by_id(assigned) == collider);
        CHECK(entity.get_element_by_id(old) == nullptr);
    }

    TEST_CASE("entity additional simple purge hides element lookups without immediately destroying ownership") {
        Salix::Entity entity;
        auto* transform = entity.get_transform();
        REQUIRE(transform != nullptr);
        const auto id = transform->get_id();
        const auto count = entity.get_all_elements().size();
        entity.simple_purge();
        CHECK(entity.is_purged());
        CHECK(entity.get_element_by_id(id) == nullptr);
        CHECK(entity.get_element_by_type_name("Transform") == nullptr);
        CHECK(entity.get_elements_by_type_name("BoxCollider").empty());
        CHECK(entity.get_all_elements().size() == count);
    }

    TEST_CASE("entity additional purging parent orphans children without flagging descendants") {
        Salix::Entity root, child, sibling;
        child.set_parent(&root);
        sibling.set_parent(&root);
        root.purge();
        CHECK(root.is_purged());
        CHECK(root.get_children().empty());
        CHECK(child.get_parent() == nullptr);
        CHECK(sibling.get_parent() == nullptr);
        CHECK_FALSE(child.is_purged());
        CHECK_FALSE(sibling.is_purged());
        CHECK(child.get_transform()->get_parent() == nullptr);
    }

    TEST_CASE("entity additional purging only a leaf never flags its still-alive parent") {
        Salix::Entity root, leaf;
        leaf.set_parent(&root);
        leaf.purge();
        CHECK(leaf.is_purged());
        CHECK_FALSE(root.is_purged());
        CHECK(root.get_children().size() == 1);
        CHECK(root.get_children()[0] == &leaf);
    }

    TEST_CASE("entity additional deserialization load restores element owners and reinitializes live components") {
        Salix::Entity entity;
        auto* observer = entity.add_element<AdditionalEntityProbe>();
        REQUIRE(observer != nullptr);
        REQUIRE(entity.get_transform() != nullptr);
        CHECK(observer->initialized == 1);
        Salix::InitContext context{};
        entity.on_load(context);
        CHECK(observer->initialized == 2);
        CHECK(observer->loads == 1);
        CHECK(observer->get_owner() == &entity);
        CHECK(entity.get_transform()->get_owner() == &entity);
        entity.update(0.25f);
        CHECK(observer->updates == 1);
        CHECK(observer->last_delta == doctest::Approx(0.25f));
    }
}


// Runtime-facing isolated contracts for the 847+ coverage batch.
TEST_SUITE("Salix::ecs::EntityBatch847") {
    TEST_CASE("entity empty name assignment preserves persistent identity and mandatory components") {
        Salix::Entity e; const auto id=e.get_id(); e.set_name(""); CHECK(e.get_name().empty()); CHECK(e.get_id()==id); CHECK(e.get_transform()!=nullptr); CHECK(e.get_element<Salix::BoxCollider>()!=nullptr);
    }

    TEST_CASE("entity release from parent is idempotent for a root entity") {
        Salix::Entity e; REQUIRE(e.get_parent()==nullptr); CHECK_NOTHROW(e.release_from_parent()); CHECK_NOTHROW(e.release_from_parent()); CHECK(e.get_parent()==nullptr); CHECK(e.is_root());
    }

    TEST_CASE("entity self parenting is rejected without mutating root state") {
        Salix::Entity e; e.set_parent(&e); CHECK(e.get_parent()==nullptr); CHECK(e.get_children().empty()); CHECK(e.get_transform()->get_parent()==nullptr);
    }

    TEST_CASE("entity grandchild recognizes every ancestor but not an unrelated sibling") {
        Salix::Entity root,parent,child,sibling; parent.set_parent(&root); child.set_parent(&parent); sibling.set_parent(&root); CHECK(child.is_child_of(&parent)); CHECK(child.is_child_of(&root)); CHECK_FALSE(child.is_child_of(&sibling)); CHECK_FALSE(root.is_child_of(&child));
    }

    TEST_CASE("entity assigning the same parent twice keeps one child edge") {
        Salix::Entity parent,child; child.set_parent(&parent); child.set_parent(&parent); REQUIRE(parent.get_children().size()==1); CHECK(parent.get_children().front()==&child); CHECK(child.get_parent()==&parent);
    }

    TEST_CASE("entity parent child insertion order is stable for multiple children") {
        Salix::Entity parent,a,b,c; a.set_parent(&parent); b.set_parent(&parent); c.set_parent(&parent); const auto& children=parent.get_children(); REQUIRE(children.size()==3); CHECK(children[0]==&a); CHECK(children[1]==&b); CHECK(children[2]==&c);
    }

    TEST_CASE("entity invalid element identifier lookup always returns null") {
        Salix::Entity e; CHECK(e.get_element_by_id(Salix::SimpleGuid::invalid())==nullptr);
    }

    TEST_CASE("entity empty element name and type queries never match mandatory components") {
        Salix::Entity e; CHECK(e.get_element_by_name("")==nullptr); CHECK(e.get_element_by_type_name("")==nullptr); CHECK(e.get_elements_by_type_name("").empty());
    }

    TEST_CASE("entity const element enumeration mirrors mutable enumeration order") {
        Salix::Entity e; const auto mutable_elements=e.get_all_elements(); const Salix::Entity& ce=e; const auto const_elements=ce.get_all_elements(); REQUIRE(mutable_elements.size()==const_elements.size()); REQUIRE(mutable_elements.size()>=2); for(std::size_t i=0;i<mutable_elements.size();++i) CHECK(mutable_elements[i]==const_elements[i]);
    }

    TEST_CASE("entity simple purge preserves hierarchy links until maintenance or explicit detach") {
        Salix::Entity parent,child; child.set_parent(&parent); child.simple_purge(); CHECK(child.is_purged()); CHECK(child.get_parent()==&parent); REQUIRE(parent.get_children().size()==1); CHECK(parent.get_children().front()==&child); CHECK(child.get_element_by_type_name("Transform")==nullptr);
    }
}

// ---- Relocated from src/tests/SalixEngine/ecs/Element.test.cpp; canonical tests mirror production .cpp files ----
// =================================================================================
// Filename:    src/Tests/SalixEngine/ecs/Element.test.cpp
// Description: Contains unit tests for the base Element class.
// ================================================================================= 

#include <doctest.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Entity.h> // Needed for owner tests
#include <Salix/core/SimpleGuid.h>
#include <memory>
#include <cereal/archives/json.hpp>
#include <sstream>

// A minimal, concrete class that inherits from Element for testing purposes.
class MockElement : public Salix::Element {
public:
    const char* get_class_name() const override { return "MockElement"; }
};

TEST_SUITE("Salix::ecs::Element") {

    TEST_CASE("initializes with correct default values") {
        // ARRANGE
        Salix::SimpleGuid::reset_counter_for_testing();
        MockElement element;

        // ASSERT
        CHECK(element.get_name() == "Element");
        CHECK(element.is_visible() == true);
        CHECK(element.get_owner() == nullptr);
        CHECK(element.get_id().get_value() == 1);
        CHECK(element.get_id().is_valid() == true);
    }

    TEST_CASE("getters and setters modify properties correctly") {
        // ARRANGE
        MockElement element;
        Salix::Entity mock_owner;

        // ACT & ASSERT for name
        element.set_name("New Name");
        CHECK(element.get_name() == "New Name");

        // ACT & ASSERT for visibility
        element.set_visibility(false);
        CHECK(element.is_visible() == false);

        // ACT & ASSERT for owner
        element.set_owner(&mock_owner);
        CHECK(element.get_owner() == &mock_owner);

        // ACT & ASSERT for ID
        Salix::SimpleGuid new_id = Salix::SimpleGuid::generate();
        element.set_id(new_id);
        CHECK(element.get_id() == new_id);
    }

    TEST_CASE("lifecycle methods run without crashing") {
        // ARRANGE
        MockElement element;
        Salix::InitContext context; // Default context is fine

        // ACT & ASSERT: The test passes if these virtual methods can be
        // called on the base class without throwing an exception.
        CHECK_NOTHROW(element.initialize());
        CHECK_NOTHROW(element.on_load(context));
        CHECK_NOTHROW(element.update(0.016f));
        CHECK_NOTHROW(element.shutdown());
    }

    TEST_CASE("can be serialized and deserialized correctly") {
        // ARRANGE
        auto original_element = std::make_unique<MockElement>();
        original_element->set_name("SerializedElement");
        // We'll use the ID it was generated with for the check.
        Salix::SimpleGuid original_id = original_element->get_id();

        std::stringstream ss;

        // ACT (Serialization)
        {
            cereal::JSONOutputArchive output_archive(ss);
            output_archive(*original_element);
        }

        // ACT (Deserialization)
        auto loaded_element = std::make_unique<MockElement>();
        {
            cereal::JSONInputArchive input_archive(ss);
            input_archive(*loaded_element);
        }

        // ASSERT
        REQUIRE(loaded_element != nullptr);
        CHECK(loaded_element->get_name() == "SerializedElement");
        // The ID should also be restored from the serialized data.
        CHECK(loaded_element->get_id() == original_id);
    }
}

// Component identity, runtime ownership and serialization boundaries.
#include <cereal/archives/binary.hpp>
#include <cstdint>
TEST_SUITE("Salix::ecs::ElementExtended") {
    TEST_CASE("element class type and user-facing name remain separate identities") {
        MockElement element;
        CHECK(std::string(element.get_class_name()) == "MockElement");
        CHECK(element.get_name() == "Element");
        element.set_name("Changed for Editor");
        CHECK(element.get_name() == "Changed for Editor");
        CHECK(std::string(element.get_class_name()) == "MockElement");
    }

    TEST_CASE("element visibility can cycle off on off without modifying identity") {
        MockElement element;
        const auto id = element.get_id();
        element.set_visibility(false);
        CHECK_FALSE(element.is_visible());
        element.set_visibility(true);
        CHECK(element.is_visible());
        element.set_visibility(false);
        CHECK_FALSE(element.is_visible());
        CHECK(element.get_id() == id);
    }

    TEST_CASE("element empty component names can be assigned without losing class information") {
        MockElement element;
        element.set_name("");
        CHECK(element.get_name().empty());
        CHECK(std::string(element.get_class_name()) == "MockElement");
    }

    TEST_CASE("element non-owning owner reference can be replaced and cleared") {
        Salix::Entity first, second;
        MockElement component;
        component.set_owner(&first);
        CHECK(component.get_owner() == &first);
        component.set_owner(&second);
        CHECK(component.get_owner() == &second);
        component.set_owner(nullptr);
        CHECK(component.get_owner() == nullptr);
    }

    TEST_CASE("element raw ID pointer views same storage as public GUID value") {
        MockElement component;
        const uint64_t* raw = component.get_id_as_ptr();
        REQUIRE(raw != nullptr);
        CHECK(*raw == component.get_id().get_value());
        component.set_id(Salix::SimpleGuid::from_value(900003ULL));
        CHECK(raw == component.get_id_as_ptr());
        CHECK(*raw == 900003ULL);
    }

    TEST_CASE("element setting invalid ID makes component unidentifiable but retains metadata") {
        MockElement component;
        component.set_name("InspectorLabel");
        component.set_id(Salix::SimpleGuid::invalid());
        CHECK_FALSE(component.get_id().is_valid());
        CHECK(component.get_id_as_ptr() != nullptr);
        CHECK(*component.get_id_as_ptr() == 0ULL);
        CHECK(component.get_name() == "InspectorLabel");
    }

    TEST_CASE("element virtual lifecycle methods are callable via base element pointer") {
        MockElement component;
        Salix::Element* polymorphic = &component;
        Salix::InitContext context{};
        CHECK_NOTHROW(polymorphic->initialize());
        CHECK_NOTHROW(polymorphic->on_load(context));
        CHECK_NOTHROW(polymorphic->update(-0.25f));
        CHECK_NOTHROW(polymorphic->shutdown());
        CHECK(polymorphic->get_owner() == nullptr);
    }

    TEST_CASE("element JSON archive restores name and GUID but retains destination visibility") {
        MockElement original;
        original.set_name("Saved");
        original.set_id(Salix::SimpleGuid::from_value(99113ULL));
        std::stringstream buffer;
        { cereal::JSONOutputArchive out(buffer); out(original); }
        MockElement loaded;
        loaded.set_visibility(false);
        { cereal::JSONInputArchive in(buffer); in(loaded); }
        CHECK(loaded.get_name() == "Saved");
        CHECK(loaded.get_id() == original.get_id());
        CHECK_FALSE(loaded.is_visible());
    }

    TEST_CASE("element binary archive restores GUID and component label") {
        MockElement original;
        original.set_name("BinaryComponent");
        original.set_id(Salix::SimpleGuid::from_value(8190ULL));
        std::stringstream buffer;
        { cereal::BinaryOutputArchive out(buffer); out(original); }
        MockElement loaded;
        { cereal::BinaryInputArchive in(buffer); in(loaded); }
        CHECK(loaded.get_name() == original.get_name());
        CHECK(loaded.get_id() == original.get_id());
    }

    TEST_CASE("element serializing a component does not serialize its runtime owner pointer") {
        Salix::Entity parent;
        MockElement original;
        original.set_owner(&parent);
        std::stringstream buffer;
        { cereal::BinaryOutputArchive out(buffer); out(original); }
        MockElement loaded;
        { cereal::BinaryInputArchive in(buffer); in(loaded); }
        CHECK(loaded.get_owner() == nullptr);
        CHECK(original.get_owner() == &parent);
    }
}

// Value and ownership contracts for the existing canonical suite.
#include <cstdint>
namespace { struct LifecycleElement647 : MockElement {
    int initialized=0,loaded=0,updated=0,shut_down=0;
    float delta=0.f;
    void initialize() override {++initialized;}
    void on_load(const Salix::InitContext&) override {++loaded;}
    void update(float dt) override {++updated;delta=dt;}
    void shutdown() override {++shut_down;}
}; }
TEST_SUITE("Salix::ecs::ElementBatch647") {
    TEST_CASE("component renaming leaves its generated GUID unchanged") {
        MockElement component;
        const auto identity=component.get_id();
        component.set_name("Renamed");
        CHECK(component.get_id()==identity);
        CHECK(component.get_name()=="Renamed");
    }

    TEST_CASE("two freshly created components have distinct identifiers and owners") {
        MockElement a,b;
        CHECK(a.get_id()!=b.get_id());
        CHECK(a.get_owner()==nullptr);
        CHECK(b.get_owner()==nullptr);
    }

    TEST_CASE("assigning an explicit identity updates the raw ID view") {
        MockElement element;
        auto* raw=element.get_id_as_ptr();REQUIRE(raw!=nullptr);
        element.set_id(Salix::SimpleGuid::from_value(45321098ULL));
        CHECK(raw==element.get_id_as_ptr());
        CHECK(*raw==45321098ULL);
    }

    TEST_CASE("resetting an element identity to invalid preserves its name and visibility") {
        MockElement element;
        element.set_name("CameraCollision");
        element.set_visibility(false);
        element.set_id(Salix::SimpleGuid::invalid());
        CHECK_FALSE(element.get_id().is_valid());
        CHECK(element.get_name()=="CameraCollision");
        CHECK_FALSE(element.is_visible());
    }

    TEST_CASE("reassigning an owner pointer does not modify the owner's GUID") {
        Salix::Entity parent,other;
        const auto first_id=parent.get_id(),second_id=other.get_id();
        MockElement element;
        element.set_owner(&parent);element.set_owner(&other);
        CHECK(element.get_owner()==&other);
        CHECK(parent.get_id()==first_id);
        CHECK(other.get_id()==second_id);
    }

    TEST_CASE("JSON persistence restores an explicit GUID beyond 32 bits") {
        MockElement original;
        original.set_id(Salix::SimpleGuid::from_value((std::uint64_t{1}<<40)+5));
        std::stringstream stream;{cereal::JSONOutputArchive out(stream);out(original);}
        MockElement restored;{cereal::JSONInputArchive in(stream);in(restored);}
        CHECK(restored.get_id()==original.get_id());
    }

    TEST_CASE("binary persistence does not adopt the original component owner") {
        Salix::Entity owner;
        MockElement original;original.set_owner(&owner);original.set_name("OwnerlessPersist");
        std::stringstream stream;{cereal::BinaryOutputArchive out(stream);out(original);}
        MockElement restored;{cereal::BinaryInputArchive in(stream);in(restored);}
        CHECK(restored.get_owner()==nullptr);
        CHECK(restored.get_name()=="OwnerlessPersist");
    }

    TEST_CASE("JSON loading changes identifier and label but not runtime visibility") {
        MockElement original;original.set_name("Persisted");
        original.set_id(Salix::SimpleGuid::from_value(23003344ULL));
        std::stringstream stream;{cereal::JSONOutputArchive out(stream);out(original);}
        MockElement restored;restored.set_visibility(false);
        {cereal::JSONInputArchive in(stream);in(restored);}
        CHECK(restored.get_id()==original.get_id());
        CHECK(restored.get_name()=="Persisted");
        CHECK_FALSE(restored.is_visible());
    }

    TEST_CASE("lifecycle calls dispatch through base pointer into a derived probe") {
        LifecycleElement647 probe;
        Salix::Element* base=&probe;
        Salix::InitContext context{};
        base->initialize();base->on_load(context);base->update(0.25f);base->shutdown();
        CHECK(probe.initialized==1);
        CHECK(probe.loaded==1);
        CHECK(probe.updated==1);
        CHECK(probe.shut_down==1);
        CHECK(probe.delta==doctest::Approx(0.25f));
    }

    TEST_CASE("a second lifecycle sequence never regenerates the component identifier") {
        LifecycleElement647 probe;
        const auto id=probe.get_id();
        Salix::InitContext context{};
        probe.initialize();probe.update(0.f);probe.on_load(context);probe.shutdown();
        probe.initialize();probe.update(-0.5f);
        CHECK(probe.initialized==2);
        CHECK(probe.updated==2);
        CHECK(probe.get_id()==id);
    }
}

TEST_SUITE("Salix::ecs::ElementBatch947") {
    TEST_CASE("display name changes never alter concrete element class name") {
        MockElement element;
        element.set_name("");
        CHECK(std::string(element.get_class_name()) == "MockElement");
        element.set_name("Editor Label");
        CHECK(std::string(element.get_class_name()) == "MockElement");
        CHECK(element.get_name() == "Editor Label");
    }

    TEST_CASE("visibility changes preserve owner name and persistent identity") {
        Salix::Entity owner;
        MockElement element;
        element.set_owner(&owner);
        element.set_name("Renderable");
        const auto id = element.get_id();
        element.set_visibility(false);
        element.set_visibility(true);
        CHECK(element.get_owner() == &owner);
        CHECK(element.get_name() == "Renderable");
        CHECK(element.get_id() == id);
    }

    TEST_CASE("const GUID reference remains attached to element storage after replacement") {
        MockElement element;
        const Salix::SimpleGuid& observed = element.get_id();
        element.set_id(Salix::SimpleGuid::from_value(424242ULL));
        CHECK(&observed == &element.get_id());
        CHECK(observed.get_value() == 424242ULL);
    }

    TEST_CASE("raw GUID pointer stays stable across invalid and valid assignments") {
        MockElement element;
        const std::uint64_t* raw = element.get_id_as_ptr();
        element.set_id(Salix::SimpleGuid::invalid());
        CHECK(raw == element.get_id_as_ptr());
        CHECK(*raw == 0ULL);
        element.set_id(Salix::SimpleGuid::from_value(77ULL));
        CHECK(raw == element.get_id_as_ptr());
        CHECK(*raw == 77ULL);
    }

    TEST_CASE("JSON persistence preserves multiline component labels") {
        MockElement original;
        original.set_name("Line One\nLine Two\tTabbed");
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(original); }
        MockElement restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_name() == original.get_name());
    }

    TEST_CASE("binary persistence preserves embedded NUL bytes in component labels") {
        MockElement original;
        const std::string label("A\0B\0C", 5);
        original.set_name(label);
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(original); }
        MockElement restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        REQUIRE(restored.get_name().size() == 5);
        CHECK(restored.get_name() == label);
    }

    TEST_CASE("JSON persistence preserves a long editor-facing component label") {
        MockElement original;
        original.set_name(std::string(4096, 'x'));
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(original); }
        MockElement restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_name().size() == 4096);
        CHECK(restored.get_name() == original.get_name());
    }

    TEST_CASE("default lifecycle methods do not mutate element metadata") {
        MockElement element;
        element.set_name("Stable");
        element.set_visibility(false);
        const auto id = element.get_id();
        Salix::InitContext context{};
        element.initialize();
        element.on_load(context);
        element.update(123.0f);
        element.shutdown();
        CHECK(element.get_name() == "Stable");
        CHECK_FALSE(element.is_visible());
        CHECK(element.get_id() == id);
    }

    TEST_CASE("serialization roundtrip preserves invalid GUID sentinel") {
        MockElement original;
        original.set_id(Salix::SimpleGuid::invalid());
        original.set_name("No Identity");
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(original); }
        MockElement restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK_FALSE(restored.get_id().is_valid());
        CHECK(restored.get_name() == "No Identity");
    }

    TEST_CASE("runtime owner can be cleared without changing persistent element state") {
        Salix::Entity owner;
        MockElement element;
        element.set_name("Transient Owner");
        element.set_visibility(false);
        const auto id = element.get_id();
        element.set_owner(&owner);
        REQUIRE(element.get_owner() == &owner);
        element.set_owner(nullptr);
        CHECK(element.get_owner() == nullptr);
        CHECK(element.get_name() == "Transient Owner");
        CHECK_FALSE(element.is_visible());
        CHECK(element.get_id() == id);
    }
}
