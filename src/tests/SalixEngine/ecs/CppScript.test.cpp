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

TEST_SUITE("Salix::ecs::CppScriptBatch3247") {
 TEST_CASE("batch3247 fresh script combines canonical identity and empty runtime name") { Salix::CppScript s; CHECK(std::string(s.get_class_name())=="CppScript"); CHECK(s.get_name()=="CppScript"); CHECK(s.get_script_name().empty()); }
 TEST_CASE("batch3247 UTF8 runtime script name is preserved byte for byte") { Salix::CppScript s; const std::string v=u8"Player_控制_3247"; s.set_script_name(v); CHECK(s.get_script_name()==v); CHECK(s.get_script_name().size()==v.size()); }
 TEST_CASE("batch3247 script name accepts path-like separators without normalization") { Salix::CppScript s; s.set_script_name("Gameplay/Actors\\Player.Controller"); CHECK(s.get_script_name()=="Gameplay/Actors\\Player.Controller"); }
 TEST_CASE("batch3247 script name accepts leading and trailing whitespace exactly") { Salix::CppScript s; s.set_script_name("\t Player3247 \n"); CHECK(s.get_script_name()=="\t Player3247 \n"); }
 TEST_CASE("batch3247 clearing runtime name preserves customized display name") { Salix::CppScript s; s.set_name("Editor Script 3247"); s.set_script_name("Runtime3247"); s.set_script_name(""); CHECK(s.get_script_name().empty()); CHECK(s.get_name()=="Editor Script 3247"); }
 TEST_CASE("batch3247 runtime name replacement preserves element identifier") { Salix::CppScript s; const auto id=s.get_id(); s.set_script_name("A"); s.set_script_name("A much longer runtime script name 3247"); CHECK(s.get_id()==id); }
 TEST_CASE("batch3247 owner changes preserve display and runtime names") { Salix::Entity a,b; Salix::CppScript s; s.set_name("Display3247"); s.set_script_name("Runtime3247"); s.set_owner(&a); s.set_owner(&b); CHECK(s.get_owner()==&b); CHECK(s.get_name()=="Display3247"); CHECK(s.get_script_name()=="Runtime3247"); }
 TEST_CASE("batch3247 visibility changes preserve display and runtime names") { Salix::CppScript s; s.set_name("Display3247"); s.set_script_name("Runtime3247"); s.set_visibility(false); s.set_visibility(true); CHECK(s.is_visible()); CHECK(s.get_name()=="Display3247"); CHECK(s.get_script_name()=="Runtime3247"); }
 TEST_CASE("batch3247 lifecycle sequence preserves all public metadata") { Salix::CppScript s; s.set_name("Lifecycle3247"); s.set_script_name("Runtime3247"); const auto id=s.get_id(); s.initialize(); s.update(0.f); s.update(-3.f); s.shutdown(); CHECK(s.get_id()==id); CHECK(s.get_name()=="Lifecycle3247"); CHECK(s.get_script_name()=="Runtime3247"); }
 TEST_CASE("batch3247 script name backing string reference remains stable across clear and refill") { Salix::CppScript s; const std::string* p=&s.get_script_name(); s.set_script_name("First3247"); s.set_script_name(""); s.set_script_name("Second3247Longer"); CHECK(&s.get_script_name()==p); CHECK(*p=="Second3247Longer"); }
}
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>

TEST_SUITE("Salix::ecs::CppScriptBatch3647") {
    TEST_CASE("batch3647 script setter copies source string rather than aliasing it") {
        Salix::CppScript script;
        std::string source = "Runtime3647";
        script.set_script_name(source);
        source.assign("MutatedSource");
        CHECK(script.get_script_name() == "Runtime3647");
    }

    TEST_CASE("batch3647 entity identifier replacement leaves runtime script name unchanged") {
        Salix::CppScript script;
        script.set_script_name("Identity3647");
        script.set_id(Salix::SimpleGuid::from_value(3647001ULL));
        CHECK(script.get_id().get_value() == 3647001ULL);
        CHECK(script.get_script_name() == "Identity3647");
    }

    TEST_CASE("batch3647 runtime script name changes never alter customized display name") {
        Salix::CppScript script;
        script.set_name("Editor Label 3647");
        script.set_script_name("FirstRuntime");
        script.set_script_name("SecondRuntime");
        CHECK(script.get_name() == "Editor Label 3647");
        CHECK(script.get_script_name() == "SecondRuntime");
    }

    TEST_CASE("batch3647 class identity remains canonical after display and runtime name changes") {
        Salix::CppScript script;
        script.set_name("NotCppScript");
        script.set_script_name("AnotherName");
        CHECK(std::string(script.get_class_name()) == "CppScript");
        CHECK(script.get_name() == "NotCppScript");
        CHECK(script.get_script_name() == "AnotherName");
    }

    TEST_CASE("batch3647 JSON roundtrip preserves script display and runtime names") {
        Salix::CppScript source;
        source.set_name("Json Script 3647");
        source.set_script_name("Gameplay.Json.Controller");
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_name() == source.get_name());
        CHECK(restored.get_script_name() == source.get_script_name());
    }

    TEST_CASE("batch3647 JSON roundtrip preserves explicit element identifier") {
        Salix::CppScript source;
        source.set_id(Salix::SimpleGuid::from_value(3647002ULL));
        source.set_script_name("JsonId3647");
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_id() == source.get_id());
        CHECK(restored.get_script_name() == "JsonId3647");
    }

    TEST_CASE("batch3647 binary roundtrip preserves runtime script name and identifier") {
        Salix::CppScript source;
        source.set_id(Salix::SimpleGuid::from_value(3647003ULL));
        source.set_script_name("BinaryRuntime3647");
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_id() == source.get_id());
        CHECK(restored.get_script_name() == source.get_script_name());
    }

    TEST_CASE("batch3647 JSON load preserves destination runtime visibility state") {
        Salix::CppScript source;
        source.set_visibility(true);
        source.set_script_name("VisibilityJson3647");
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        restored.set_visibility(false);
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK_FALSE(restored.is_visible());
        CHECK(restored.get_script_name() == "VisibilityJson3647");
    }

    TEST_CASE("batch3647 JSON load preserves destination runtime owner pointer") {
        Salix::CppScript source;
        source.set_script_name("OwnerJson3647");
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::Entity owner;
        Salix::CppScript restored;
        restored.set_owner(&owner);
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_owner() == &owner);
        CHECK(restored.get_script_name() == "OwnerJson3647");
    }

    TEST_CASE("batch3647 binary roundtrip preserves UTF8 runtime name byte for byte") {
        Salix::CppScript source;
        const std::string runtime = u8"脚本_café_3647";
        source.set_script_name(runtime);
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_script_name() == runtime);
        CHECK(restored.get_script_name().size() == runtime.size());
    }
}

TEST_SUITE("Salix::ecs::CppScriptBatch4047") {
    TEST_CASE("batch4047 fresh script starts visible ownerless with valid identifier") {
        Salix::CppScript script;
        CHECK(script.get_owner() == nullptr);
        CHECK(script.is_visible());
        CHECK(script.get_id().is_valid());
        CHECK(script.get_script_name().empty());
    }

    TEST_CASE("batch4047 runtime name storage address stays stable across clear and rewrite") {
        Salix::CppScript script;
        const auto* storage = &script.get_script_name();
        script.set_script_name("First4047");
        CHECK(&script.get_script_name() == storage);
        script.set_script_name("");
        CHECK(&script.get_script_name() == storage);
        script.set_script_name("Second4047");
        CHECK(&script.get_script_name() == storage);
        CHECK(*storage == "Second4047");
    }

    TEST_CASE("batch4047 embedded null runtime name remains complete owned string") {
        Salix::CppScript script;
        const std::string value("Batch\0Script4047", 16);
        script.set_script_name(value);
        CHECK(script.get_script_name() == value);
        CHECK(script.get_script_name().size() == value.size());
    }

    TEST_CASE("batch4047 path like runtime name remains unnormalized") {
        Salix::CppScript script;
        script.set_script_name("Scripts/Gameplay\\Player4047.cpp");
        CHECK(script.get_script_name() == "Scripts/Gameplay\\Player4047.cpp");
    }

    TEST_CASE("batch4047 element identifier replacement never changes runtime script name") {
        Salix::CppScript script;
        script.set_script_name("Runtime4047");
        script.set_id(Salix::SimpleGuid::from_value(4047003ULL));
        CHECK(script.get_id().get_value() == 4047003ULL);
        CHECK(script.get_script_name() == "Runtime4047");
    }

    TEST_CASE("batch4047 customized display name remains independent from runtime name") {
        Salix::CppScript script;
        script.set_name("Editor Label 4047");
        script.set_script_name("RuntimeName4047");
        CHECK(script.get_name() == "Editor Label 4047");
        CHECK(script.get_script_name() == "RuntimeName4047");
        CHECK(std::string(script.get_class_name()) == "CppScript");
    }

    TEST_CASE("batch4047 repeated visibility changes never alter runtime name") {
        Salix::CppScript script;
        script.set_script_name("Visible4047");
        script.set_visibility(false);
        script.set_visibility(true);
        script.set_visibility(false);
        CHECK_FALSE(script.is_visible());
        CHECK(script.get_script_name() == "Visible4047");
    }

    TEST_CASE("batch4047 owner attach detach cycle never alters runtime metadata") {
        Salix::Entity owner;
        Salix::CppScript script;
        script.set_script_name("Owner4047");
        script.set_owner(&owner);
        REQUIRE(script.get_owner() == &owner);
        script.set_owner(nullptr);
        CHECK(script.get_owner() == nullptr);
        CHECK(script.get_script_name() == "Owner4047");
    }

    TEST_CASE("batch4047 lifecycle sequence preserves display and runtime identities") {
        Salix::CppScript script;
        script.set_name("Display4047");
        script.set_script_name("Lifecycle4047");
        const auto id = script.get_id();
        script.initialize();
        script.update(0.0f);
        script.update(-4047.0f);
        script.shutdown();
        CHECK(script.get_name() == "Display4047");
        CHECK(script.get_script_name() == "Lifecycle4047");
        CHECK(script.get_id() == id);
    }

    TEST_CASE("batch4047 two scripts keep completely independent runtime names") {
        Salix::CppScript first, second;
        first.set_script_name("First4047");
        second.set_script_name("Second4047");
        first.set_script_name("FirstUpdated4047");
        CHECK(first.get_script_name() == "FirstUpdated4047");
        CHECK(second.get_script_name() == "Second4047");
    }
}

TEST_SUITE("Salix::ecs::CppScriptBatch4347") {
    TEST_CASE("batch4347 runtime name setter copies caller storage") {
        Salix::CppScript script;
        std::string source = "Controller4347";
        script.set_script_name(source);
        source = "MutatedCaller";
        CHECK(script.get_script_name() == "Controller4347");
    }

    TEST_CASE("batch4347 runtime name preserves tabs newlines and spaces exactly") {
        Salix::CppScript script;
        const std::string value = "\tRuntime 4347\nController ";
        script.set_script_name(value);
        CHECK(script.get_script_name() == value);
    }

    TEST_CASE("batch4347 runtime name and display name can both be empty independently") {
        Salix::CppScript script;
        script.set_name("");
        script.set_script_name("");
        CHECK(script.get_name().empty());
        CHECK(script.get_script_name().empty());
        CHECK(std::string(script.get_class_name()) == "CppScript");
    }

    TEST_CASE("batch4347 invalid identifier assignment preserves runtime name") {
        Salix::CppScript script;
        script.set_script_name("Runtime4347");
        script.set_id(Salix::SimpleGuid::invalid());
        CHECK_FALSE(script.get_id().is_valid());
        CHECK(script.get_script_name() == "Runtime4347");
    }

    TEST_CASE("batch4347 binary roundtrip preserves custom display and runtime names") {
        Salix::CppScript source;
        source.set_name("Display Binary 4347");
        source.set_script_name("Runtime.Binary.4347");
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_name() == source.get_name());
        CHECK(restored.get_script_name() == source.get_script_name());
    }

    TEST_CASE("batch4347 JSON roundtrip preserves escaped path and quote characters") {
        Salix::CppScript source;
        const std::string runtime = "Scripts\\Gameplay\\\"Controller4347\"";
        source.set_script_name(runtime);
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_script_name() == runtime);
        CHECK(restored.get_script_name().size() == runtime.size());
    }

    TEST_CASE("batch4347 binary deserialization leaves destination owner pointer local") {
        Salix::CppScript source;
        source.set_script_name("OwnerLocal4347");
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::Entity owner;
        Salix::CppScript restored;
        restored.set_owner(&owner);
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_owner() == &owner);
        CHECK(restored.get_script_name() == "OwnerLocal4347");
    }

    TEST_CASE("batch4347 binary deserialization leaves destination visibility local") {
        Salix::CppScript source;
        source.set_visibility(true);
        source.set_script_name("VisibilityLocal4347");
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        restored.set_visibility(false);
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK_FALSE(restored.is_visible());
        CHECK(restored.get_script_name() == "VisibilityLocal4347");
    }

    TEST_CASE("batch4347 lifecycle calls preserve explicit invalid identifier") {
        Salix::CppScript script;
        script.set_id(Salix::SimpleGuid::invalid());
        script.set_script_name("Lifecycle4347");
        script.initialize();
        script.update(4347.0f);
        script.shutdown();
        CHECK_FALSE(script.get_id().is_valid());
        CHECK(script.get_script_name() == "Lifecycle4347");
    }

    TEST_CASE("batch4347 separate scripts retain independent backing strings after alternating writes") {
        Salix::CppScript first, second;
        first.set_script_name("First4347");
        second.set_script_name("Second4347");
        first.set_script_name("FirstUpdated4347");
        second.set_script_name("SecondUpdated4347");
        CHECK(first.get_script_name() == "FirstUpdated4347");
        CHECK(second.get_script_name() == "SecondUpdated4347");
        CHECK(&first.get_script_name() != &second.get_script_name());
    }
}

TEST_SUITE("Salix::ecs::CppScriptBatch4847") {
    TEST_CASE("batch4847 runtime name preserves UTF8 bytes exactly") {
        Salix::CppScript script;
        const std::string value = std::string("\xE8\x84\x9A\xE6\x9C\xAC") + "_Controller_4847";
        script.set_script_name(value);
        CHECK(script.get_script_name() == value);
        CHECK(script.get_script_name().size() == value.size());
    }

    TEST_CASE("batch4847 runtime name preserves leading and trailing whitespace exactly") {
        Salix::CppScript script;
        const std::string value = "  \tRuntime4847\n ";
        script.set_script_name(value);
        CHECK(script.get_script_name() == value);
    }

    TEST_CASE("batch4847 runtime name reference retains stable address across setter calls") {
        Salix::CppScript script;
        const auto* storage = &script.get_script_name();
        script.set_script_name("First4847");
        CHECK(&script.get_script_name() == storage);
        script.set_script_name("Second4847LongerName");
        CHECK(&script.get_script_name() == storage);
        CHECK(*storage == "Second4847LongerName");
    }

    TEST_CASE("batch4847 binary roundtrip preserves embedded null bytes in runtime name") {
        Salix::CppScript source;
        const std::string value("Run\0Time\0Batch4847", 18);
        source.set_script_name(value);
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_script_name() == value);
        CHECK(restored.get_script_name().size() == value.size());
    }

    TEST_CASE("batch4847 JSON roundtrip preserves explicitly empty runtime name") {
        Salix::CppScript source;
        source.set_script_name("");
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        restored.set_script_name("Before4847");
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_script_name().empty());
    }

    TEST_CASE("batch4847 JSON roundtrip preserves newline tab and backslash escaping") {
        Salix::CppScript source;
        const std::string value = "Scripts\\Batch4847\nController\tA";
        source.set_script_name(value);
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::CppScript restored;
        { cereal::JSONInputArchive in(stream); in(restored); }
        CHECK(restored.get_script_name() == value);
    }

    TEST_CASE("batch4847 runtime name mutation never changes display name identifier or visibility") {
        Salix::CppScript script;
        script.set_name("Display4847");
        script.set_id(Salix::SimpleGuid::from_value(4847001ULL));
        script.set_visibility(false);
        script.set_script_name("Runtime4847");
        script.set_script_name("RuntimeUpdated4847");
        CHECK(script.get_name() == "Display4847");
        CHECK(script.get_id().get_value() == 4847001ULL);
        CHECK_FALSE(script.is_visible());
        CHECK(script.get_script_name() == "RuntimeUpdated4847");
    }

    TEST_CASE("batch4847 binary roundtrip replaces destination runtime name but leaves destination owner local") {
        Salix::CppScript source;
        source.set_script_name("Serialized4847");
        std::stringstream stream;
        { cereal::BinaryOutputArchive out(stream); out(source); }
        Salix::Entity owner;
        Salix::CppScript restored;
        restored.set_owner(&owner);
        restored.set_script_name("DestinationBefore4847");
        { cereal::BinaryInputArchive in(stream); in(restored); }
        CHECK(restored.get_owner() == &owner);
        CHECK(restored.get_script_name() == "Serialized4847");
    }

    TEST_CASE("batch4847 repeated lifecycle calls preserve whitespace rich runtime name") {
        Salix::CppScript script;
        const std::string value = " \tLifecycle 4847\n";
        script.set_script_name(value);
        script.initialize();
        script.update(-1.0f);
        script.update(0.0f);
        script.shutdown();
        CHECK(script.get_script_name() == value);
    }

    TEST_CASE("batch4847 two scripts with equal runtime text still own distinct string objects") {
        Salix::CppScript first, second;
        first.set_script_name("SharedText4847");
        second.set_script_name("SharedText4847");
        CHECK(first.get_script_name() == second.get_script_name());
        CHECK(&first.get_script_name() != &second.get_script_name());
    }
}
