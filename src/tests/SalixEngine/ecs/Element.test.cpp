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
