#include <doctest.h>
#include <Salix/reflection/EditorDataMode.h>
#include <array>
#include <map>
#include <set>
#include <string>
#include <type_traits>

TEST_SUITE("Salix::reflection::EditorDataModeBatch1647") {
    TEST_CASE("value initialized editor data mode selects Live") {
        Salix::EditorDataMode mode{};
        CHECK(mode == Salix::EditorDataMode::Live);
    }

    TEST_CASE("live and YAML editor data modes remain distinct") {
        CHECK(Salix::EditorDataMode::Live != Salix::EditorDataMode::Yaml);
    }

    TEST_CASE("editor data mode defines exactly two unique current modes") {
        const std::array<Salix::EditorDataMode, 2> modes{
            Salix::EditorDataMode::Live,
            Salix::EditorDataMode::Yaml
        };
        std::set<Salix::EditorDataMode> unique(modes.begin(), modes.end());
        CHECK(unique.size() == modes.size());
    }

    TEST_CASE("editor data mode is a strongly typed enum") {
        CHECK((std::is_enum_v<Salix::EditorDataMode>));
        CHECK_FALSE((std::is_convertible_v<Salix::EditorDataMode, int>));
    }

    TEST_CASE("editor data mode uses an integral underlying representation") {
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::EditorDataMode>>));
    }

    TEST_CASE("editor data mode copy construction preserves Live") {
        const Salix::EditorDataMode source = Salix::EditorDataMode::Live;
        const Salix::EditorDataMode copy = source;
        CHECK(copy == Salix::EditorDataMode::Live);
    }

    TEST_CASE("editor data mode copy construction preserves YAML") {
        const Salix::EditorDataMode source = Salix::EditorDataMode::Yaml;
        const Salix::EditorDataMode copy = source;
        CHECK(copy == Salix::EditorDataMode::Yaml);
    }

    TEST_CASE("editor data mode assignment switches between live and YAML") {
        Salix::EditorDataMode mode = Salix::EditorDataMode::Live;
        mode = Salix::EditorDataMode::Yaml;
        CHECK(mode == Salix::EditorDataMode::Yaml);
        mode = Salix::EditorDataMode::Live;
        CHECK(mode == Salix::EditorDataMode::Live);
    }

    TEST_CASE("editor data mode can be used as an ordered configuration key") {
        std::map<Salix::EditorDataMode, std::string> labels;
        labels[Salix::EditorDataMode::Live] = "Live";
        labels[Salix::EditorDataMode::Yaml] = "Yaml";
        CHECK(labels.size() == 2);
        CHECK(labels.at(Salix::EditorDataMode::Live) == "Live");
        CHECK(labels.at(Salix::EditorDataMode::Yaml) == "Yaml");
    }

    TEST_CASE("switching on editor data mode reaches one and only one current branch") {
        auto label_for = [](Salix::EditorDataMode mode) {
            switch (mode) {
                case Salix::EditorDataMode::Live: return std::string("live");
                case Salix::EditorDataMode::Yaml: return std::string("yaml");
            }
            return std::string("unknown");
        };
        CHECK(label_for(Salix::EditorDataMode::Live) == "live");
        CHECK(label_for(Salix::EditorDataMode::Yaml) == "yaml");
    }
}


TEST_SUITE("Salix::reflection::EditorDataModeBatch2047") {
    TEST_CASE("Live survives conversion through the enum underlying type") {
        using U = std::underlying_type_t<Salix::EditorDataMode>; const auto mode = Salix::EditorDataMode::Live;
        CHECK(static_cast<Salix::EditorDataMode>(static_cast<U>(mode)) == mode);
    }
    TEST_CASE("Yaml survives conversion through the enum underlying type") {
        using U = std::underlying_type_t<Salix::EditorDataMode>; const auto mode = Salix::EditorDataMode::Yaml;
        CHECK(static_cast<Salix::EditorDataMode>(static_cast<U>(mode)) == mode);
    }
    TEST_CASE("value initialized mode may be reassigned from Live to Yaml") {
        Salix::EditorDataMode mode{}; REQUIRE(mode == Salix::EditorDataMode::Live); mode = Salix::EditorDataMode::Yaml; CHECK(mode == Salix::EditorDataMode::Yaml);
    }
    TEST_CASE("const copy preserves selected mode") {
        const Salix::EditorDataMode source = Salix::EditorDataMode::Yaml; const Salix::EditorDataMode copy(source); CHECK(copy == Salix::EditorDataMode::Yaml);
    }
    TEST_CASE("copy assignment replaces destination without modifying source") {
        Salix::EditorDataMode source = Salix::EditorDataMode::Yaml; Salix::EditorDataMode destination = Salix::EditorDataMode::Live;
        destination = source; CHECK(destination == Salix::EditorDataMode::Yaml); CHECK(source == Salix::EditorDataMode::Yaml);
    }
    TEST_CASE("array storage preserves explicit Live Yaml Live ordering") {
        const std::array<Salix::EditorDataMode, 3> modes{Salix::EditorDataMode::Live, Salix::EditorDataMode::Yaml, Salix::EditorDataMode::Live};
        CHECK(modes[0] == Salix::EditorDataMode::Live); CHECK(modes[1] == Salix::EditorDataMode::Yaml); CHECK(modes[2] == Salix::EditorDataMode::Live);
    }
    TEST_CASE("ordered set collapses duplicate modes by value") {
        std::set<Salix::EditorDataMode> modes; modes.insert(Salix::EditorDataMode::Live); modes.insert(Salix::EditorDataMode::Yaml); modes.insert(Salix::EditorDataMode::Live);
        CHECK(modes.size() == 2);
    }
    TEST_CASE("map update for Live leaves Yaml entry untouched") {
        std::map<Salix::EditorDataMode, std::string> labels; labels[Salix::EditorDataMode::Live] = "old-live"; labels[Salix::EditorDataMode::Yaml] = "yaml";
        labels[Salix::EditorDataMode::Live] = "live"; CHECK(labels.at(Salix::EditorDataMode::Live) == "live"); CHECK(labels.at(Salix::EditorDataMode::Yaml) == "yaml");
    }
    TEST_CASE("underlying values for the two declared modes remain distinct") {
        using U = std::underlying_type_t<Salix::EditorDataMode>;
        CHECK(static_cast<U>(Salix::EditorDataMode::Live) != static_cast<U>(Salix::EditorDataMode::Yaml));
    }
    TEST_CASE("identity function preserves editor data mode across pass and return") {
        const auto identity = [](Salix::EditorDataMode mode) { return mode; };
        CHECK(identity(Salix::EditorDataMode::Live) == Salix::EditorDataMode::Live); CHECK(identity(Salix::EditorDataMode::Yaml) == Salix::EditorDataMode::Yaml);
    }
}
