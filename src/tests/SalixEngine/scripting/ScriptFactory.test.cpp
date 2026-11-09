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

namespace {
class Batch2147ScriptA : public Salix::ScriptElement {
public: const char* get_class_name() const override { return "Batch2147ScriptA"; }
};
class Batch2147ScriptB : public Salix::ScriptElement {
public: const char* get_class_name() const override { return "Batch2147ScriptB"; }
};
}

TEST_SUITE("Salix::scripting::ScriptFactoryBatch2147") {
    TEST_CASE("batch2147 singleton accessor returns the same factory object") {
        CHECK(&Salix::ScriptFactory::get()==&Salix::ScriptFactory::get());
    }
    TEST_CASE("batch2147 missing script lookup remains null across repeated attempts") {
        auto& f=Salix::ScriptFactory::get(); CHECK(f.create_script("Batch2147::never-registered")==nullptr); CHECK(f.create_script("Batch2147::never-registered")==nullptr);
    }
    TEST_CASE("batch2147 registered creator produces a fresh object on each request") {
        auto& f=Salix::ScriptFactory::get(); f.register_script("Batch2147::fresh",[]{return std::make_unique<Batch2147ScriptA>();});
        auto a=f.create_script("Batch2147::fresh"); auto b=f.create_script("Batch2147::fresh"); REQUIRE(a!=nullptr); REQUIRE(b!=nullptr); CHECK(a.get()!=b.get());
    }
    TEST_CASE("batch2147 separate registration keys produce requested dynamic types") {
        auto& f=Salix::ScriptFactory::get(); f.register_script("Batch2147::A",[]{return std::make_unique<Batch2147ScriptA>();}); f.register_script("Batch2147::B",[]{return std::make_unique<Batch2147ScriptB>();});
        CHECK(dynamic_cast<Batch2147ScriptA*>(f.create_script("Batch2147::A").get())!=nullptr); CHECK(dynamic_cast<Batch2147ScriptB*>(f.create_script("Batch2147::B").get())!=nullptr);
    }
    TEST_CASE("batch2147 registrations remain case sensitive") {
        auto& f=Salix::ScriptFactory::get(); f.register_script("Batch2147::Case",[]{return std::make_unique<Batch2147ScriptA>();});
        CHECK(f.create_script("Batch2147::Case")!=nullptr); CHECK(f.create_script("batch2147::case")==nullptr);
    }
    TEST_CASE("batch2147 empty string is a valid exact registration key") {
        auto& f=Salix::ScriptFactory::get(); f.register_script("",[]{return std::make_unique<Batch2147ScriptA>();}); CHECK(f.create_script("")!=nullptr);
    }
    TEST_CASE("batch2147 replacing creator changes only future instances") {
        auto& f=Salix::ScriptFactory::get(); f.register_script("Batch2147::replace",[]{return std::make_unique<Batch2147ScriptA>();}); auto old=f.create_script("Batch2147::replace");
        f.register_script("Batch2147::replace",[]{return std::make_unique<Batch2147ScriptB>();}); auto current=f.create_script("Batch2147::replace");
        REQUIRE(old!=nullptr); REQUIRE(current!=nullptr); CHECK(dynamic_cast<Batch2147ScriptA*>(old.get())!=nullptr); CHECK(dynamic_cast<Batch2147ScriptB*>(current.get())!=nullptr);
    }
    TEST_CASE("batch2147 creator can return null and registration remains present") {
        auto& f=Salix::ScriptFactory::get(); f.register_script("Batch2147::null",[]()->std::unique_ptr<Salix::ScriptElement>{return nullptr;}); CHECK(f.create_script("Batch2147::null")==nullptr); CHECK(f.create_script("Batch2147::null")==nullptr);
    }
    TEST_CASE("batch2147 creator capture state advances only when script is requested") {
        auto calls=std::make_shared<int>(0); auto& f=Salix::ScriptFactory::get(); f.register_script("Batch2147::calls",[calls]{++*calls; return std::make_unique<Batch2147ScriptA>();});
        CHECK(*calls==0); f.create_script("Batch2147::calls"); CHECK(*calls==1); f.create_script("Batch2147::calls"); CHECK(*calls==2);
    }
    TEST_CASE("batch2147 punctuation and path separators remain exact registration key characters") {
        auto& f=Salix::ScriptFactory::get(); const std::string key="Batch2147::scripts/player.controller-v3"; f.register_script(key,[]{return std::make_unique<Batch2147ScriptB>();});
        CHECK(f.create_script(key)!=nullptr); CHECK(f.create_script("Batch2147::scripts/player/controller-v3")==nullptr);
    }
}


#include <vector>
#include <Salix/core/SimpleGuid.h>

namespace {
class Batch3047ScriptA : public Salix::ScriptElement {
public: const char* get_class_name() const override { return "Batch3047ScriptA"; }
};
class Batch3047ScriptB : public Salix::ScriptElement {
public: const char* get_class_name() const override { return "Batch3047ScriptB"; }
};
}

TEST_SUITE("Salix::scripting::ScriptFactoryBatch3047") {
    TEST_CASE("batch3047 UTF8 registration key is preserved as an exact map key") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key = std::string("Batch3047::caf") + "\xC3\xA9" + "-controller";
        factory.register_script(key, [] { return std::make_unique<Batch3047ScriptA>(); });
        auto created = factory.create_script(key);
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch3047ScriptA*>(created.get()) != nullptr);
    }

    TEST_CASE("batch3047 embedded NUL bytes are valid registration key data") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key("Batch3047\0Hidden", 16);
        factory.register_script(key, [] { return std::make_unique<Batch3047ScriptA>(); });
        CHECK(factory.create_script(key) != nullptr);
        CHECK(factory.create_script("Batch3047") == nullptr);
    }

    TEST_CASE("batch3047 replacement with null creator affects only future requests") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key = "Batch3047::replace-null";
        factory.register_script(key, [] { return std::make_unique<Batch3047ScriptA>(); });
        auto existing = factory.create_script(key);
        REQUIRE(existing != nullptr);
        factory.register_script(key, []() -> std::unique_ptr<Salix::ScriptElement> { return nullptr; });
        CHECK(factory.create_script(key) == nullptr);
        CHECK(dynamic_cast<Batch3047ScriptA*>(existing.get()) != nullptr);
    }

    TEST_CASE("batch3047 throwing creator can be replaced by a working creator") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key = "Batch3047::recover-throw";
        factory.register_script(key, []() -> std::unique_ptr<Salix::ScriptElement> {
            throw std::runtime_error("batch3047");
        });
        CHECK_THROWS_AS(factory.create_script(key), std::runtime_error);
        factory.register_script(key, [] { return std::make_unique<Batch3047ScriptB>(); });
        auto recovered = factory.create_script(key);
        REQUIRE(recovered != nullptr);
        CHECK(dynamic_cast<Batch3047ScriptB*>(recovered.get()) != nullptr);
    }

    TEST_CASE("batch3047 creator can assign distinct names from captured invocation count") {
        auto& factory = Salix::ScriptFactory::get();
        auto calls = std::make_shared<int>(0);
        const std::string key = "Batch3047::numbered";
        factory.register_script(key, [calls] {
            auto script = std::make_unique<Batch3047ScriptA>();
            script->set_name("Instance" + std::to_string(++*calls));
            return script;
        });
        auto first = factory.create_script(key);
        auto second = factory.create_script(key);
        REQUIRE(first != nullptr); REQUIRE(second != nullptr);
        CHECK(first->get_name() == "Instance1");
        CHECK(second->get_name() == "Instance2");
    }

    TEST_CASE("batch3047 keys differing only by trailing slash remain independent") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch3047::path", [] { return std::make_unique<Batch3047ScriptA>(); });
        factory.register_script("Batch3047::path/", [] { return std::make_unique<Batch3047ScriptB>(); });
        CHECK(dynamic_cast<Batch3047ScriptA*>(factory.create_script("Batch3047::path").get()) != nullptr);
        CHECK(dynamic_cast<Batch3047ScriptB*>(factory.create_script("Batch3047::path/").get()) != nullptr);
    }

    TEST_CASE("batch3047 creator invocation order follows request order across different keys") {
        auto& factory = Salix::ScriptFactory::get();
        auto order = std::make_shared<std::vector<int>>();
        factory.register_script("Batch3047::order-a", [order] { order->push_back(1); return std::make_unique<Batch3047ScriptA>(); });
        factory.register_script("Batch3047::order-b", [order] { order->push_back(2); return std::make_unique<Batch3047ScriptB>(); });
        factory.create_script("Batch3047::order-b");
        factory.create_script("Batch3047::order-a");
        REQUIRE(order->size() == 2);
        CHECK((*order)[0] == 2);
        CHECK((*order)[1] == 1);
    }

    TEST_CASE("batch3047 moved creator remains callable after registration") {
        auto& factory = Salix::ScriptFactory::get();
        Salix::ScriptFactory::Creator creator = [] { return std::make_unique<Batch3047ScriptA>(); };
        factory.register_script("Batch3047::moved-creator", std::move(creator));
        auto created = factory.create_script("Batch3047::moved-creator");
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch3047ScriptA*>(created.get()) != nullptr);
    }

    TEST_CASE("batch3047 registration through one singleton reference is visible through another") {
        auto& first = Salix::ScriptFactory::get();
        auto& second = Salix::ScriptFactory::get();
        first.register_script("Batch3047::shared-reference", [] { return std::make_unique<Batch3047ScriptB>(); });
        auto created = second.create_script("Batch3047::shared-reference");
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch3047ScriptB*>(created.get()) != nullptr);
    }

    TEST_CASE("batch3047 creator can preconfigure inherited element visibility before transfer") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch3047::hidden", [] {
            auto script = std::make_unique<Batch3047ScriptA>();
            script->set_visibility(false);
            return script;
        });
        auto created = factory.create_script("Batch3047::hidden");
        REQUIRE(created != nullptr);
        CHECK_FALSE(created->is_visible());
    }
}
TEST_SUITE("Salix::scripting::ScriptFactoryBatch3747") {
    TEST_CASE("batch3747 registration key preserves newline and tab control characters exactly") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key = "Batch3747::line\n\tkey";
        factory.register_script(key, [] { return std::make_unique<Batch1047ScriptA>(); });
        REQUIRE(factory.create_script(key) != nullptr);
        CHECK(factory.create_script("Batch3747::line key") == nullptr);
    }

    TEST_CASE("batch3747 registration key preserves embedded null bytes independently from prefix") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key("Batch3747::nul\0suffix", 21);
        factory.register_script(key, [] { return std::make_unique<Batch1047ScriptB>(); });
        auto created = factory.create_script(key);
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(created.get()) != nullptr);
        CHECK(factory.create_script("Batch3747::nul") == nullptr);
    }

    TEST_CASE("batch3747 replacing working creator with empty function affects future calls only") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key = "Batch3747::working-to-empty";
        factory.register_script(key, [] { return std::make_unique<Batch1047ScriptA>(); });
        auto existing = factory.create_script(key);
        REQUIRE(existing != nullptr);
        factory.register_script(key, Salix::ScriptFactory::Creator{});
        CHECK_THROWS_AS(factory.create_script(key), std::bad_function_call);
        CHECK(dynamic_cast<Batch1047ScriptA*>(existing.get()) != nullptr);
    }

    TEST_CASE("batch3747 empty creator mapping can be repaired by later working registration") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key = "Batch3747::empty-to-working";
        factory.register_script(key, Salix::ScriptFactory::Creator{});
        CHECK_THROWS_AS(factory.create_script(key), std::bad_function_call);
        factory.register_script(key, [] { return std::make_unique<Batch1047ScriptB>(); });
        auto recovered = factory.create_script(key);
        REQUIRE(recovered != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(recovered.get()) != nullptr);
    }

    TEST_CASE("batch3747 two registration keys can share one captured invocation counter") {
        auto& factory = Salix::ScriptFactory::get();
        auto calls = std::make_shared<int>(0);
        auto creator = [calls] {
            ++*calls;
            return std::make_unique<Batch1047ScriptA>();
        };
        factory.register_script("Batch3747::shared-A", creator);
        factory.register_script("Batch3747::shared-B", creator);
        factory.create_script("Batch3747::shared-A");
        factory.create_script("Batch3747::shared-B");
        factory.create_script("Batch3747::shared-A");
        CHECK(*calls == 3);
    }

    TEST_CASE("batch3747 missing lookup never invokes unrelated registered creator") {
        auto& factory = Salix::ScriptFactory::get();
        auto calls = std::make_shared<int>(0);
        factory.register_script("Batch3747::unrelated", [calls] {
            ++*calls;
            return std::make_unique<Batch1047ScriptA>();
        });
        CHECK(factory.create_script("Batch3747::missing") == nullptr);
        CHECK(*calls == 0);
        REQUIRE(factory.create_script("Batch3747::unrelated") != nullptr);
        CHECK(*calls == 1);
    }

    TEST_CASE("batch3747 creator can recursively request another registered script") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch3747::inner", [] { return std::make_unique<Batch1047ScriptB>(); });
        factory.register_script("Batch3747::outer", [&factory] {
            return factory.create_script("Batch3747::inner");
        });
        auto created = factory.create_script("Batch3747::outer");
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(created.get()) != nullptr);
    }

    TEST_CASE("batch3747 creator can register a new mapping during invocation") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch3747::registrar", [&factory] {
            factory.register_script("Batch3747::registered-during-call", [] {
                return std::make_unique<Batch1047ScriptB>();
            });
            return std::make_unique<Batch1047ScriptA>();
        });
        auto registrar = factory.create_script("Batch3747::registrar");
        REQUIRE(registrar != nullptr);
        auto late = factory.create_script("Batch3747::registered-during-call");
        REQUIRE(late != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(late.get()) != nullptr);
    }

    TEST_CASE("batch3747 creator can preconfigure name visibility and identifier before transfer") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch3747::preconfigured", [] {
            auto script = std::make_unique<Batch1047ScriptA>();
            script->set_name("Configured3747");
            script->set_visibility(false);
            script->set_id(Salix::SimpleGuid::from_value(3747007ULL));
            return script;
        });
        auto created = factory.create_script("Batch3747::preconfigured");
        REQUIRE(created != nullptr);
        CHECK(created->get_name() == "Configured3747");
        CHECK_FALSE(created->is_visible());
        CHECK(created->get_id().get_value() == 3747007ULL);
    }

    TEST_CASE("batch3747 repeated throwing creator remains registered after each exception") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch3747::repeat-throw", []() -> std::unique_ptr<Salix::ScriptElement> {
            throw std::runtime_error("Batch3747 repeat");
        });
        CHECK_THROWS_AS(factory.create_script("Batch3747::repeat-throw"), std::runtime_error);
        CHECK_THROWS_AS(factory.create_script("Batch3747::repeat-throw"), std::runtime_error);
    }
}

TEST_SUITE("Salix::scripting::ScriptFactoryBatch4147") {
    TEST_CASE("batch4147 singleton accessor returns the same process wide factory") {
        CHECK(&Salix::ScriptFactory::get() == &Salix::ScriptFactory::get());
    }

    TEST_CASE("batch4147 missing unique registration key returns null") {
        CHECK(Salix::ScriptFactory::get().create_script("Batch4147::missing-never-registered") == nullptr);
    }

    TEST_CASE("batch4147 registered creator constructs requested concrete script") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4147::A", [] { return std::make_unique<Batch1047ScriptA>(); });
        auto created = factory.create_script("Batch4147::A");
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptA*>(created.get()) != nullptr);
    }

    TEST_CASE("batch4147 repeated creation returns independent ownership") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4147::independent", [] { return std::make_unique<Batch1047ScriptA>(); });
        auto first = factory.create_script("Batch4147::independent");
        auto second = factory.create_script("Batch4147::independent");
        REQUIRE(first != nullptr); REQUIRE(second != nullptr);
        CHECK(first.get() != second.get());
    }

    TEST_CASE("batch4147 replacing mapping changes only future creations") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4147::replace", [] { return std::make_unique<Batch1047ScriptA>(); });
        auto existing = factory.create_script("Batch4147::replace");
        REQUIRE(existing != nullptr);
        factory.register_script("Batch4147::replace", [] { return std::make_unique<Batch1047ScriptB>(); });
        auto replaced = factory.create_script("Batch4147::replace");
        REQUIRE(replaced != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptA*>(existing.get()) != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(replaced.get()) != nullptr);
    }

    TEST_CASE("batch4147 creator invocation remains deferred until create request") {
        auto& factory = Salix::ScriptFactory::get();
        auto calls = std::make_shared<int>(0);
        factory.register_script("Batch4147::deferred", [calls] {
            ++*calls;
            return std::make_unique<Batch1047ScriptA>();
        });
        CHECK(*calls == 0);
        factory.create_script("Batch4147::deferred");
        factory.create_script("Batch4147::deferred");
        CHECK(*calls == 2);
    }

    TEST_CASE("batch4147 deliberate null creator result propagates as null") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4147::null",
            []() -> std::unique_ptr<Salix::ScriptElement> { return nullptr; });
        CHECK(factory.create_script("Batch4147::null") == nullptr);
    }

    TEST_CASE("batch4147 empty creator preserves standard bad function call contract") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4147::empty", Salix::ScriptFactory::Creator{});
        CHECK_THROWS_AS(factory.create_script("Batch4147::empty"), std::bad_function_call);
    }

    TEST_CASE("batch4147 registration keys preserve whitespace and punctuation exactly") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key = " Batch4147::player/controller-v3 ";
        factory.register_script(key, [] { return std::make_unique<Batch1047ScriptB>(); });
        REQUIRE(factory.create_script(key) != nullptr);
        CHECK(factory.create_script("Batch4147::player/controller-v3") == nullptr);
    }

    TEST_CASE("batch4147 creator can delegate to another registered mapping") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4147::inner", [] { return std::make_unique<Batch1047ScriptB>(); });
        factory.register_script("Batch4147::outer", [&factory] {
            return factory.create_script("Batch4147::inner");
        });
        auto created = factory.create_script("Batch4147::outer");
        REQUIRE(created != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(created.get()) != nullptr);
    }
}

TEST_SUITE("Salix::scripting::ScriptFactoryBatch4547") {
    TEST_CASE("batch4547 unique missing registration returns null without affecting factory") {
        auto& factory = Salix::ScriptFactory::get();
        CHECK(factory.create_script("Batch4547::missing-never-registered") == nullptr);
        factory.register_script("Batch4547::after-missing", [] { return std::make_unique<Batch1047ScriptA>(); });
        CHECK(factory.create_script("Batch4547::after-missing") != nullptr);
    }

    TEST_CASE("batch4547 creator can return preconfigured inherited script metadata") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4547::preconfigured", [] {
            auto script = std::make_unique<Batch1047ScriptA>();
            script->set_name("Configured4547");
            script->set_visibility(false);
            return script;
        });
        auto created = factory.create_script("Batch4547::preconfigured");
        REQUIRE(created != nullptr);
        CHECK(created->get_name() == "Configured4547");
        CHECK_FALSE(created->is_visible());
    }

    TEST_CASE("batch4547 captured creator counter increments exactly once per creation") {
        auto& factory = Salix::ScriptFactory::get();
        auto calls = std::make_shared<int>(0);
        factory.register_script("Batch4547::counter", [calls] {
            ++*calls;
            return std::make_unique<Batch1047ScriptA>();
        });
        REQUIRE(factory.create_script("Batch4547::counter") != nullptr);
        REQUIRE(factory.create_script("Batch4547::counter") != nullptr);
        REQUIRE(factory.create_script("Batch4547::counter") != nullptr);
        CHECK(*calls == 3);
    }

    TEST_CASE("batch4547 replacement leaves previously created object concrete type unchanged") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4547::replace", [] { return std::make_unique<Batch1047ScriptA>(); });
        auto old_object = factory.create_script("Batch4547::replace");
        REQUIRE(old_object != nullptr);
        factory.register_script("Batch4547::replace", [] { return std::make_unique<Batch1047ScriptB>(); });
        auto new_object = factory.create_script("Batch4547::replace");
        REQUIRE(new_object != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptA*>(old_object.get()) != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(new_object.get()) != nullptr);
    }

    TEST_CASE("batch4547 empty creator can be repaired after bad function call") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4547::repair-empty", Salix::ScriptFactory::Creator{});
        CHECK_THROWS_AS(factory.create_script("Batch4547::repair-empty"), std::bad_function_call);
        factory.register_script("Batch4547::repair-empty", [] { return std::make_unique<Batch1047ScriptB>(); });
        auto recovered = factory.create_script("Batch4547::repair-empty");
        REQUIRE(recovered != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(recovered.get()) != nullptr);
    }

    TEST_CASE("batch4547 deliberate null creator can later be replaced with working creator") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4547::null-then-working",
            []() -> std::unique_ptr<Salix::ScriptElement> { return nullptr; });
        CHECK(factory.create_script("Batch4547::null-then-working") == nullptr);
        factory.register_script("Batch4547::null-then-working", [] { return std::make_unique<Batch1047ScriptA>(); });
        REQUIRE(factory.create_script("Batch4547::null-then-working") != nullptr);
    }

    TEST_CASE("batch4547 case distinct registration keys resolve independently") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4547::Case", [] { return std::make_unique<Batch1047ScriptA>(); });
        factory.register_script("Batch4547::case", [] { return std::make_unique<Batch1047ScriptB>(); });
        auto upper = factory.create_script("Batch4547::Case");
        auto lower = factory.create_script("Batch4547::case");
        REQUIRE(upper != nullptr); REQUIRE(lower != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptA*>(upper.get()) != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(lower.get()) != nullptr);
    }

    TEST_CASE("batch4547 embedded null registration key remains distinct from visible prefix") {
        auto& factory = Salix::ScriptFactory::get();
        const std::string key("Batch4547::nul\0tail", 19);
        factory.register_script(key, [] { return std::make_unique<Batch1047ScriptA>(); });
        REQUIRE(factory.create_script(key) != nullptr);
        CHECK(factory.create_script("Batch4547::nul") == nullptr);
    }

    TEST_CASE("batch4547 creator may register another mapping for a later request") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4547::registrar", [&factory] {
            factory.register_script("Batch4547::late", [] { return std::make_unique<Batch1047ScriptB>(); });
            return std::make_unique<Batch1047ScriptA>();
        });
        auto registrar = factory.create_script("Batch4547::registrar");
        REQUIRE(registrar != nullptr);
        auto late = factory.create_script("Batch4547::late");
        REQUIRE(late != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(late.get()) != nullptr);
    }

    TEST_CASE("batch4547 throwing creator can be replaced with working creator after exception") {
        auto& factory = Salix::ScriptFactory::get();
        factory.register_script("Batch4547::throw-then-working",
            []() -> std::unique_ptr<Salix::ScriptElement> { throw std::runtime_error("Batch4547"); });
        CHECK_THROWS_AS(factory.create_script("Batch4547::throw-then-working"), std::runtime_error);
        factory.register_script("Batch4547::throw-then-working", [] { return std::make_unique<Batch1047ScriptB>(); });
        auto recovered = factory.create_script("Batch4547::throw-then-working");
        REQUIRE(recovered != nullptr);
        CHECK(dynamic_cast<Batch1047ScriptB*>(recovered.get()) != nullptr);
    }
}
