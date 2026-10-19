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
