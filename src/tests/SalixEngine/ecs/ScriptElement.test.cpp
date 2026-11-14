#include <doctest.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/ecs/Entity.h>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>
#include <string>

TEST_SUITE("Salix::ecs::ScriptElementBatch1247") {
    TEST_CASE("script element exposes canonical class identity without rewriting base display name") {
        Salix::ScriptElement script;
        CHECK(std::string(script.get_class_name()) == "ScriptElement");
        CHECK(script.get_name() == "Element");
    }

    TEST_CASE("script element construction generates a valid persistent identifier") {
        Salix::ScriptElement first;
        Salix::ScriptElement second;
        CHECK(first.get_id().is_valid());
        CHECK(second.get_id().is_valid());
        CHECK(first.get_id() != second.get_id());
    }

    TEST_CASE("script element initialize is an intentional no-op for persistent metadata") {
        Salix::ScriptElement script;
        script.set_name("Script Probe");
        const auto id = script.get_id();
        script.initialize();
        CHECK(script.get_name() == "Script Probe");
        CHECK(script.get_id() == id);
    }

    TEST_CASE("script element update accepts positive and zero delta without mutating metadata") {
        Salix::ScriptElement script;
        script.set_name("Update Probe");
        const auto id = script.get_id();
        script.update(0.016f);
        script.update(0.0f);
        CHECK(script.get_name() == "Update Probe");
        CHECK(script.get_id() == id);
    }

    TEST_CASE("script element update accepts negative delta as a no-op") {
        Salix::ScriptElement script;
        script.set_visibility(false);
        const auto id = script.get_id();
        script.update(-10.0f);
        CHECK_FALSE(script.is_visible());
        CHECK(script.get_id() == id);
    }

    TEST_CASE("script element shutdown leaves inherited state unchanged") {
        Salix::ScriptElement script;
        script.set_name("Shutdown Probe");
        script.set_visibility(false);
        const auto id = script.get_id();
        script.shutdown();
        CHECK(script.get_name() == "Shutdown Probe");
        CHECK_FALSE(script.is_visible());
        CHECK(script.get_id() == id);
    }

    TEST_CASE("script element inherited visibility remains independently mutable") {
        Salix::ScriptElement script;
        CHECK(script.is_visible());
        script.set_visibility(false);
        CHECK_FALSE(script.is_visible());
        script.set_visibility(true);
        CHECK(script.is_visible());
    }

    TEST_CASE("script element runtime owner survives its no-op lifecycle calls") {
        Salix::Entity owner;
        Salix::ScriptElement script;
        script.set_owner(&owner);
        script.initialize();
        script.update(1.0f);
        script.shutdown();
        CHECK(script.get_owner() == &owner);
    }

    TEST_CASE("script element JSON roundtrip preserves base element name and identity") {
        Salix::ScriptElement source;
        source.set_name("Json Script Element");

        std::stringstream stream;
        {
            cereal::JSONOutputArchive archive(stream);
            archive(source);
        }

        Salix::ScriptElement restored;
        {
            cereal::JSONInputArchive archive(stream);
            archive(restored);
        }

        CHECK(restored.get_name() == "Json Script Element");
        CHECK(restored.get_id() == source.get_id());
    }

    TEST_CASE("script element binary roundtrip preserves persistent state but not runtime owner") {
        Salix::Entity owner;
        Salix::ScriptElement source;
        source.set_name("Binary Script Element");
        source.set_owner(&owner);

        std::stringstream stream;
        {
            cereal::BinaryOutputArchive archive(stream);
            archive(source);
        }

        Salix::ScriptElement restored;
        {
            cereal::BinaryInputArchive archive(stream);
            archive(restored);
        }

        CHECK(restored.get_name() == "Binary Script Element");
        CHECK(restored.get_id() == source.get_id());
        CHECK(restored.get_owner() == nullptr);
    }
}

TEST_SUITE("Salix::ecs::ScriptElementBatch6747") {
    TEST_CASE("batch6747 empty display name remains independent from class identity") {
        Salix::ScriptElement script;
        script.set_name("");
        CHECK(script.get_name().empty());
        CHECK(std::string(script.get_class_name()) == "ScriptElement");
    }

    TEST_CASE("batch6747 explicit invalid identifier survives no op initialize") {
        Salix::ScriptElement script;
        script.set_id(Salix::SimpleGuid::invalid());
        script.initialize();
        CHECK_FALSE(script.get_id().is_valid());
    }

    TEST_CASE("batch6747 explicit identifier survives signed update sequence") {
        Salix::ScriptElement script;
        script.set_id(Salix::SimpleGuid::from_value(6747005ULL));
        script.update(-67.47f);
        script.update(0.0f);
        script.update(67.47f);
        CHECK(script.get_id().get_value() == 6747005ULL);
    }

    TEST_CASE("batch6747 owner can be replaced without changing persistent identity") {
        Salix::Entity first;
        Salix::Entity second;
        Salix::ScriptElement script;
        const auto id = script.get_id();
        script.set_owner(&first);
        script.set_owner(&second);
        CHECK(script.get_owner() == &second);
        CHECK(script.get_id() == id);
    }

    TEST_CASE("batch6747 lifecycle sequence preserves hidden visibility") {
        Salix::ScriptElement script;
        script.set_visibility(false);
        script.initialize();
        script.update(6.747f);
        script.shutdown();
        CHECK_FALSE(script.is_visible());
    }

    TEST_CASE("batch6747 lifecycle sequence preserves display name") {
        Salix::ScriptElement script;
        script.set_name("Lifecycle6747");
        script.initialize();
        script.update(6.747f);
        script.shutdown();
        CHECK(script.get_name() == "Lifecycle6747");
    }

    TEST_CASE("batch6747 JSON roundtrip preserves invalid identifier") {
        Salix::ScriptElement source;
        source.set_id(Salix::SimpleGuid::invalid());
        source.set_name("JsonInvalid6747");
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::ScriptElement restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK_FALSE(restored.get_id().is_valid());
        CHECK(restored.get_name() == "JsonInvalid6747");
    }

    TEST_CASE("batch6747 binary roundtrip preserves empty display name") {
        Salix::ScriptElement source;
        source.set_name("");
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::ScriptElement restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_name().empty());
        CHECK(restored.get_id() == source.get_id());
    }

    TEST_CASE("batch6747 serialization never restores runtime owner") {
        Salix::Entity owner;
        Salix::ScriptElement source;
        source.set_owner(&owner);
        source.set_name("RuntimeOwner6747");
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::ScriptElement restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_owner() == nullptr);
        CHECK(restored.get_name() == "RuntimeOwner6747");
    }

    TEST_CASE("batch6747 separate scripts retain independent inherited state") {
        Salix::ScriptElement first;
        Salix::ScriptElement second;
        first.set_name("First6747");
        first.set_visibility(false);
        CHECK(second.get_name() == "Element");
        CHECK(second.is_visible());
        CHECK(first.get_id() != second.get_id());
    }
}
