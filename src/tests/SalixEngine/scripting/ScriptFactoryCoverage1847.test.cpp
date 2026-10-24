#include <doctest.h>
#include <Salix/scripting/ScriptFactory.h>
#include <Salix/ecs/ScriptElement.h>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
class Batch1847ScriptA : public Salix::ScriptElement {
public: const char* get_class_name() const override { return "Batch1847ScriptA"; }
};
class Batch1847ScriptB : public Salix::ScriptElement {
public: const char* get_class_name() const override { return "Batch1847ScriptB"; }
};
}

TEST_SUITE("Salix::scripting::ScriptFactoryCoverage1847") {
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
