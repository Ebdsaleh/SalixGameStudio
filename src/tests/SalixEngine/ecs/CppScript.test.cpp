#include <doctest.h>
#include <Salix/ecs/CppScript.h>
#include <Salix/ecs/Entity.h>
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

    TEST_CASE("cpp script display name remains independent of registered script name") {
        Salix::CppScript script;
        script.set_name("Editor Label");
        script.set_script_name("RuntimeController");
        CHECK(script.get_name() == "Editor Label");
        CHECK(script.get_script_name() == "RuntimeController");
        CHECK(std::string(script.get_class_name()) == "CppScript");
    }

    TEST_CASE("cpp script owner assignment does not alter registered script metadata") {
        Salix::Entity owner;
        Salix::CppScript script;
        script.set_script_name("OwnedController");
        script.set_owner(&owner);
        CHECK(script.get_owner() == &owner);
        CHECK(script.get_script_name() == "OwnedController");
        script.set_owner(nullptr);
        CHECK(script.get_owner() == nullptr);
        CHECK(script.get_script_name() == "OwnedController");
    }

    TEST_CASE("cpp script getter returns stable string storage across replacements") {
        Salix::CppScript script;
        const std::string* storage = &script.get_script_name();
        script.set_script_name("First");
        CHECK(&script.get_script_name() == storage);
        CHECK(*storage == "First");
        script.set_script_name("SecondNameWithDifferentLength");
        CHECK(&script.get_script_name() == storage);
        CHECK(*storage == "SecondNameWithDifferentLength");
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


TEST_SUITE("Salix::ecs::CppScriptBatch2247") {
    TEST_CASE("batch2247 script name supports punctuation-rich runtime identifiers") {
        Salix::CppScript script; script.set_script_name("Gameplay.Player/Controller-v2");
        CHECK(script.get_script_name()=="Gameplay.Player/Controller-v2");
    }
    TEST_CASE("batch2247 script name supports whitespace without trimming") {
        Salix::CppScript script; script.set_script_name("  Player Controller  ");
        CHECK(script.get_script_name()=="  Player Controller  ");
    }
    TEST_CASE("batch2247 script name replacement leaves element identifier unchanged") {
        Salix::CppScript script; const auto id=script.get_id(); script.set_script_name("One"); script.set_script_name("Two");
        CHECK(script.get_id()==id); CHECK(script.get_script_name()=="Two");
    }
    TEST_CASE("batch2247 visibility toggles never alter runtime script name") {
        Salix::CppScript script; script.set_script_name("VisibilityProbe"); script.set_visibility(false); script.set_visibility(true);
        CHECK(script.get_script_name()=="VisibilityProbe"); CHECK(script.is_visible());
    }
    TEST_CASE("batch2247 owner replacement never alters runtime script name") {
        Salix::Entity first,second; Salix::CppScript script; script.set_script_name("OwnerProbe");
        script.set_owner(&first); script.set_owner(&second);
        CHECK(script.get_owner()==&second); CHECK(script.get_script_name()=="OwnerProbe");
    }
    TEST_CASE("batch2247 repeated identical script-name assignment preserves exact stored value") {
        Salix::CppScript script; script.set_script_name("Repeat2247"); const auto* storage=&script.get_script_name();
        script.set_script_name("Repeat2247");
        CHECK(script.get_script_name()=="Repeat2247"); CHECK(&script.get_script_name()==storage);
    }
    TEST_CASE("batch2247 multiline runtime script name is stored without normalization") {
        Salix::CppScript script; const std::string value="LineOne\nLineTwo\tController"; script.set_script_name(value);
        CHECK(script.get_script_name()==value); CHECK(script.get_script_name().find('\n')!=std::string::npos); CHECK(script.get_script_name().find('\t')!=std::string::npos);
    }
    TEST_CASE("batch2247 long runtime script name remains intact through getter access") {
        Salix::CppScript script; const std::string value(4096,'S'); script.set_script_name(value);
        CHECK(script.get_script_name().size()==4096); CHECK(script.get_script_name()==value);
    }
    TEST_CASE("batch2247 clearing script name does not alter display name visibility owner or identity") {
        Salix::Entity owner; Salix::CppScript script; script.set_name("Editor2247"); script.set_visibility(false); script.set_owner(&owner); const auto id=script.get_id();
        script.set_script_name("Runtime2247"); script.set_script_name("");
        CHECK(script.get_script_name().empty()); CHECK(script.get_name()=="Editor2247"); CHECK_FALSE(script.is_visible()); CHECK(script.get_owner()==&owner); CHECK(script.get_id()==id);
    }
    TEST_CASE("batch2247 lifecycle calls preserve public runtime metadata without serialization") {
        Salix::Entity owner; Salix::CppScript script; script.set_name("Lifecycle2247"); script.set_script_name("RuntimeLifecycle2247"); script.set_owner(&owner); script.set_visibility(false); const auto id=script.get_id();
        script.initialize(); script.update(-5.0f); script.update(0.0f); script.update(5.0f); script.shutdown();
        CHECK(script.get_name()=="Lifecycle2247"); CHECK(script.get_script_name()=="RuntimeLifecycle2247"); CHECK(script.get_owner()==&owner); CHECK_FALSE(script.is_visible()); CHECK(script.get_id()==id);
    }
}

