#include <doctest.h>
#include <Salix/scripting/ScriptFactory.h>
#include <Salix/ecs/ScriptElement.h>
#include <memory>
#include <string>

namespace {
class Batch1047ScriptA : public Salix::ScriptElement {
public:
    const char* get_class_name() const override { return "Batch1047ScriptA"; }
};

class Batch1047ScriptB : public Salix::ScriptElement {
public:
    const char* get_class_name() const override { return "Batch1047ScriptB"; }
};
}

TEST_SUITE("Salix::scripting::ScriptFactoryBatch1047") {
    TEST_CASE("get returns the same process-wide factory instance") {
        auto& first = Salix::ScriptFactory::get();
        auto& second = Salix::ScriptFactory::get();
        CHECK(&first == &second);
    }

    TEST_CASE("creating an unregistered script name returns null") {
        auto created = Salix::ScriptFactory::get().create_script("Batch1047::missing-never-registered");
        CHECK(created == nullptr);
    }

    TEST_CASE("registered creator produces the requested concrete script type") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1047::A", [] { return std::make_unique<Batch1047ScriptA>(); });
        auto created = factory.create_script("Batch1047::A");
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptA*>(created.get()) != nullptr);
        CHECK(std::string(created->get_class_name()) == "Batch1047ScriptA");
    }

    TEST_CASE("each create call invokes the creator and returns independent ownership") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1047::independent", [] { return std::make_unique<Batch1047ScriptA>(); });
        auto first = factory.create_script("Batch1047::independent");
        auto second = factory.create_script("Batch1047::independent");
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
        CHECK(first.get() != second.get());
        first->set_name("First");
        second->set_name("Second");
        CHECK(first->get_name() == "First");
        CHECK(second->get_name() == "Second");
    }

    TEST_CASE("registration stores creator without invoking it until creation") {
        auto calls = std::make_shared<int>(0);
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1047::deferred", [calls] {
            ++(*calls);
            return std::make_unique<Batch1047ScriptA>();
        });
        CHECK(*calls == 0);
        auto first = factory.create_script("Batch1047::deferred");
        CHECK(*calls == 1);
        auto second = factory.create_script("Batch1047::deferred");
        CHECK(*calls == 2);
        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
    }

    TEST_CASE("registering an existing name replaces its creator") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1047::replace", [] { return std::make_unique<Batch1047ScriptA>(); });
        REQUIRE(dynamic_cast<Batch1047ScriptA*>(factory.create_script("Batch1047::replace").get()) != nullptr);
        factory.register_script("Batch1047::replace", [] { return std::make_unique<Batch1047ScriptB>(); });
        auto replaced = factory.create_script("Batch1047::replace");
        REQUIRE(replaced != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(replaced.get()) != nullptr);
    }

    TEST_CASE("script registration names remain case sensitive") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1047::CaseSensitive", [] { return std::make_unique<Batch1047ScriptA>(); });
        CHECK(factory.create_script("Batch1047::casesensitive") == nullptr);
        CHECK(factory.create_script("BATCH1047::CASESENSITIVE") == nullptr);
        CHECK(factory.create_script("Batch1047::CaseSensitive") != nullptr);
    }

    TEST_CASE("empty string is a valid explicit registration key") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("", [] { return std::make_unique<Batch1047ScriptB>(); });
        auto created = factory.create_script("");
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(created.get()) != nullptr);
    }

    TEST_CASE("factory propagates a creator that deliberately returns null") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script(
            "Batch1047::null-result",
            []() -> std::unique_ptr<Salix::ScriptElement> { return nullptr; });
        CHECK(factory.create_script("Batch1047::null-result") == nullptr);
    }

    TEST_CASE("different registered names retain independent creator mappings") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1047::map-A", [] { return std::make_unique<Batch1047ScriptA>(); });
        factory.register_script("Batch1047::map-B", [] { return std::make_unique<Batch1047ScriptB>(); });
        auto a = factory.create_script("Batch1047::map-A");
        auto b = factory.create_script("Batch1047::map-B");
        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptA*>(a.get()) != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(b.get()) != nullptr);
    }
}
