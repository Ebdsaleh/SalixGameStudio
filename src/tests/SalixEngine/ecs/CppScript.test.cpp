#include <doctest.h>
#include <Salix/ecs/CppScript.h>
#include <Salix/ecs/Entity.h>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>
#include <string>

TEST_SUITE("Salix::ecs::CppScriptBatch1247") {
    TEST_CASE("cpp script constructor exposes canonical class and display names") {
        Salix::CppScript script;
        CHECK(std::string(script.get_class_name()) == "CppScript");
        CHECK(script.get_name() == "CppScript");
    }

    TEST_CASE("cpp script starts with an empty registered script name") {
        Salix::CppScript script;
        CHECK(script.get_script_name().empty());
    }

    TEST_CASE("cpp script registered name setter replaces the complete value") {
        Salix::CppScript script;
        script.set_script_name("PlayerController");
        CHECK(script.get_script_name() == "PlayerController");
        script.set_script_name("EnemyController");
        CHECK(script.get_script_name() == "EnemyController");
    }

    TEST_CASE("cpp script accepts an explicitly empty registered name") {
        Salix::CppScript script;
        script.set_script_name("Temporary");
        script.set_script_name("");
        CHECK(script.get_script_name().empty());
    }

    TEST_CASE("cpp script preserves embedded null bytes in registered names") {
        Salix::CppScript script;
        const std::string value("AI\0Controller", 13);
        script.set_script_name(value);
        CHECK(script.get_script_name().size() == value.size());
        CHECK(script.get_script_name() == value);
    }

    TEST_CASE("cpp script inherited lifecycle methods do not mutate script identity") {
        Salix::CppScript script;
        script.set_script_name("LifecycleProbe");
        const auto id = script.get_id().get_value();
        script.initialize();
        script.update(0.016f);
        script.update(-1.0f);
        script.shutdown();
        CHECK(script.get_script_name() == "LifecycleProbe");
        CHECK(script.get_id().get_value() == id);
        CHECK(script.get_name() == "CppScript");
    }

    TEST_CASE("cpp script JSON roundtrip preserves script name element name and id") {
        Salix::CppScript source;
        source.set_name("Script Component");
        source.set_script_name("JsonScript");
        const auto source_id = source.get_id().get_value();

        std::stringstream stream;
        {
            cereal::JSONOutputArchive archive(stream);
            archive(source);
        }

        Salix::CppScript restored;
        {
            cereal::JSONInputArchive archive(stream);
            archive(restored);
        }

        CHECK(restored.get_script_name() == "JsonScript");
        CHECK(restored.get_name() == "Script Component");
        CHECK(restored.get_id().get_value() == source_id);
    }

    TEST_CASE("cpp script binary roundtrip preserves script metadata") {
        Salix::CppScript source;
        source.set_name("Binary Script");
        source.set_script_name("BinaryController");

        std::stringstream stream;
        {
            cereal::BinaryOutputArchive archive(stream);
            archive(source);
        }

        Salix::CppScript restored;
        {
            cereal::BinaryInputArchive archive(stream);
            archive(restored);
        }

        CHECK(restored.get_script_name() == "BinaryController");
        CHECK(restored.get_name() == "Binary Script");
        CHECK(restored.get_id() == source.get_id());
    }

    TEST_CASE("cpp script serialization never transfers runtime owner pointer") {
        Salix::Entity owner;
        Salix::CppScript source;
        source.set_owner(&owner);
        source.set_script_name("OwnerlessAfterLoad");

        std::stringstream stream;
        {
            cereal::BinaryOutputArchive archive(stream);
            archive(source);
        }

        Salix::CppScript restored;
        {
            cereal::BinaryInputArchive archive(stream);
            archive(restored);
        }

        CHECK(source.get_owner() == &owner);
        CHECK(restored.get_owner() == nullptr);
        CHECK(restored.get_script_name() == "OwnerlessAfterLoad");
    }

    TEST_CASE("cpp script visibility remains independent of registered script name") {
        Salix::CppScript script;
        script.set_script_name("VisibleScript");
        script.set_visibility(false);
        CHECK_FALSE(script.is_visible());
        CHECK(script.get_script_name() == "VisibleScript");
        script.set_visibility(true);
        CHECK(script.is_visible());
        CHECK(script.get_script_name() == "VisibleScript");
    }
}
