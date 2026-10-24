#include <doctest.h>
#include <Salix/scripting/ScriptFactory.h>
#include <Salix/ecs/ScriptElement.h>
#include <memory>
#include <string>
#include <functional>
#include <stdexcept>

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

namespace {
class Batch1847ScriptA : public Salix::ScriptElement {
public: const char* get_class_name() const override { return "Batch1847ScriptA"; }
};
class Batch1847ScriptB : public Salix::ScriptElement {
public: const char* get_class_name() const override { return "Batch1847ScriptB"; }
};
}

TEST_SUITE("Salix::scripting::ScriptFactoryBatch1847") {
    TEST_CASE("creator may capture shared state and observe every factory invocation") {
        auto calls = std::make_shared<int>(0);
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1847::capture", [calls]{ ++*calls; return std::make_unique<Batch1847ScriptA>(); });
        CHECK(*calls == 0); factory.create_script("Batch1847::capture"); factory.create_script("Batch1847::capture"); CHECK(*calls == 2);
    }

    TEST_CASE("creator can initialize returned script state before ownership is transferred") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1847::initialized", []{ auto p=std::make_unique<Batch1847ScriptA>(); p->set_name("Configured"); return p; });
        auto created = factory.create_script("Batch1847::initialized");
        REQUIRE(created != nullptr); CHECK(created->get_name() == "Configured");
    }

    TEST_CASE("creator exceptions propagate to caller rather than being converted to null") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1847::throws", []() -> std::unique_ptr<Salix::ScriptElement> { throw std::runtime_error("creator failure"); });
        CHECK_THROWS_AS(factory.create_script("Batch1847::throws"), std::runtime_error);
    }

    TEST_CASE("empty std function creator preserves standard bad function call behavior") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1847::empty-function", Salix::ScriptFactory::Creator{});
        CHECK_THROWS_AS(factory.create_script("Batch1847::empty-function"), std::bad_function_call);
    }

    TEST_CASE("registration keys preserve leading and trailing whitespace exactly") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script(" Batch1847::spaced ", []{ return std::make_unique<Batch1847ScriptA>(); });
        CHECK(factory.create_script(" Batch1847::spaced ") != nullptr);
        CHECK(factory.create_script("Batch1847::spaced") == nullptr);
    }

    TEST_CASE("punctuation-rich registration keys remain exact map keys") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key = "Batch1847::player/controller-v2.test";
        factory.register_script(key, []{ return std::make_unique<Batch1847ScriptB>(); });
        auto created = factory.create_script(key);
        REQUIRE(created != nullptr); CHECK(dynamic_cast<Batch1847ScriptB*>(created.get()) != nullptr);
    }

    TEST_CASE("prefix-related script names remain independent registrations") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1847::Prefix", []{ return std::make_unique<Batch1847ScriptA>(); });
        factory.register_script("Batch1847::PrefixLong", []{ return std::make_unique<Batch1847ScriptB>(); });
        CHECK(dynamic_cast<Batch1847ScriptA*>(factory.create_script("Batch1847::Prefix").get()) != nullptr);
        CHECK(dynamic_cast<Batch1847ScriptB*>(factory.create_script("Batch1847::PrefixLong").get()) != nullptr);
    }

    TEST_CASE("replacing one creator does not alter another registered mapping") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1847::replace-one", []{ return std::make_unique<Batch1847ScriptA>(); });
        factory.register_script("Batch1847::stable", []{ return std::make_unique<Batch1847ScriptA>(); });
        factory.register_script("Batch1847::replace-one", []{ return std::make_unique<Batch1847ScriptB>(); });
        CHECK(dynamic_cast<Batch1847ScriptB*>(factory.create_script("Batch1847::replace-one").get()) != nullptr);
        CHECK(dynamic_cast<Batch1847ScriptA*>(factory.create_script("Batch1847::stable").get()) != nullptr);
    }

    TEST_CASE("destroying one created script never consumes its registration") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1847::repeat-after-destroy", []{ return std::make_unique<Batch1847ScriptA>(); });
        { auto first = factory.create_script("Batch1847::repeat-after-destroy"); REQUIRE(first != nullptr); }
        auto second = factory.create_script("Batch1847::repeat-after-destroy");
        REQUIRE(second != nullptr); CHECK(dynamic_cast<Batch1847ScriptA*>(second.get()) != nullptr);
    }

    TEST_CASE("replacement changes future dynamic type without affecting already created instance") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch1847::future-type", []{ return std::make_unique<Batch1847ScriptA>(); });
        auto old_instance = factory.create_script("Batch1847::future-type");
        factory.register_script("Batch1847::future-type", []{ return std::make_unique<Batch1847ScriptB>(); });
        auto new_instance = factory.create_script("Batch1847::future-type");
        REQUIRE(old_instance != nullptr); REQUIRE(new_instance != nullptr);
        CHECK(dynamic_cast<Batch1847ScriptA*>(old_instance.get()) != nullptr);
        CHECK(dynamic_cast<Batch1847ScriptB*>(new_instance.get()) != nullptr);
    }
}
