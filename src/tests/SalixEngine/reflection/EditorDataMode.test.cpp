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
