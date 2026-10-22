#include <doctest.h>
#include <Salix/scripting/ScriptRegistration.h>
#include <Salix/ecs/CppScript.h>
#include <Salix/ecs/ScriptElement.h>
#include <memory>
#include <string>

class Batch1547ScriptAlpha : public Salix::CppScript {
public:
    Batch1547ScriptAlpha() { set_script_name("Batch1547ScriptAlpha"); }
    const char* get_class_name() const override { return "Batch1547ScriptAlpha"; }
};

class Batch1547ScriptBeta : public Salix::CppScript {
public:
    Batch1547ScriptBeta() { set_script_name("Batch1547ScriptBeta"); }
    const char* get_class_name() const override { return "Batch1547ScriptBeta"; }
};

class Batch1547ScriptGamma : public Salix::CppScript {
public:
    Batch1547ScriptGamma() { set_script_name("Batch1547ScriptGamma"); }
    const char* get_class_name() const override { return "Batch1547ScriptGamma"; }
};

REGISTER_SCRIPT_FACTORY(Batch1547ScriptAlpha)
REGISTER_SCRIPT_FACTORY(Batch1547ScriptBeta)
REGISTER_SCRIPT_FACTORY(Batch1547ScriptGamma)

TEST_SUITE("Salix::scripting::ScriptRegistrationBatch1547") {
    TEST_CASE("script registration macro auto registers alpha before test execution") {
        auto script = Salix::ScriptFactory::get().create_script("Batch1547ScriptAlpha");
        REQUIRE(script != nullptr);
        CHECK(std::string(script->get_class_name()) == "Batch1547ScriptAlpha");
    }

    TEST_CASE("script registration macro auto registers beta independently") {
        auto script = Salix::ScriptFactory::get().create_script("Batch1547ScriptBeta");
        REQUIRE(script != nullptr);
        CHECK(std::string(script->get_class_name()) == "Batch1547ScriptBeta");
    }

    TEST_CASE("script registration macro auto registers gamma independently") {
        auto script = Salix::ScriptFactory::get().create_script("Batch1547ScriptGamma");
        REQUIRE(script != nullptr);
        CHECK(std::string(script->get_class_name()) == "Batch1547ScriptGamma");
    }

    TEST_CASE("script registration macro uses exact case sensitive type spelling as key") {
        CHECK(Salix::ScriptFactory::get().create_script("batch1547scriptalpha") == nullptr);
        CHECK(Salix::ScriptFactory::get().create_script("BATCH1547SCRIPTALPHA") == nullptr);
        CHECK(Salix::ScriptFactory::get().create_script("Batch1547ScriptAlpha") != nullptr);
    }

    TEST_CASE("script registration factory creates a fresh script instance on every request") {
        auto first = Salix::ScriptFactory::get().create_script("Batch1547ScriptAlpha");
        auto second = Salix::ScriptFactory::get().create_script("Batch1547ScriptAlpha");
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
        CHECK(first.get() != second.get());
        CHECK(first->get_id() != second->get_id());
    }

    TEST_CASE("script registration created instance remains polymorphic through ScriptElement") {
        std::unique_ptr<Salix::ScriptElement> script =
            Salix::ScriptFactory::get().create_script("Batch1547ScriptBeta");
        REQUIRE(script != nullptr);
        CHECK(std::string(script->get_class_name()) == "Batch1547ScriptBeta");
        CHECK(script->get_owner() == nullptr);
        CHECK(script->is_visible());
    }

    TEST_CASE("script registration created CppScript retains constructor supplied registered name") {
        auto script = Salix::ScriptFactory::get().create_script("Batch1547ScriptGamma");
        REQUIRE(script != nullptr);
        auto* cpp_script = static_cast<Salix::CppScript*>(script.get());
        CHECK(cpp_script->get_script_name() == "Batch1547ScriptGamma");
    }

    TEST_CASE("script registration created script metadata can change without altering factory key") {
        auto script = Salix::ScriptFactory::get().create_script("Batch1547ScriptAlpha");
        REQUIRE(script != nullptr);
        script->set_name("Runtime Display Name");
        CHECK(script->get_name() == "Runtime Display Name");

        auto fresh = Salix::ScriptFactory::get().create_script("Batch1547ScriptAlpha");
        REQUIRE(fresh != nullptr);
        CHECK(fresh->get_name() == "CppScript");
    }

    TEST_CASE("script registration created script inherited lifecycle is safe in isolation") {
        auto script = Salix::ScriptFactory::get().create_script("Batch1547ScriptBeta");
        REQUIRE(script != nullptr);
        const auto id = script->get_id();
        script->initialize();
        script->update(0.016f);
        script->shutdown();
        CHECK(script->get_id() == id);
        CHECK(std::string(script->get_class_name()) == "Batch1547ScriptBeta");
    }

    TEST_CASE("script registration does not make similar unregistered names resolvable") {
        CHECK(Salix::ScriptFactory::get().create_script("Batch1547Script") == nullptr);
        CHECK(Salix::ScriptFactory::get().create_script("Batch1547ScriptAlphaExtra") == nullptr);
        CHECK(Salix::ScriptFactory::get().create_script("") == nullptr);
    }
}
