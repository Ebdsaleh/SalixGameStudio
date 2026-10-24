#include <doctest.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/reflection/EditorDataMode.h>
#include <Salix/rendering/ICamera.h>
#include <limits>
#include <string>
#include <typeindex>
#include <utility>

namespace {
struct Batch1847EnumA {};
struct Batch1847EnumB {};
struct Batch1847EnumC {};
struct Batch1847EnumD {};
struct Batch1847EnumE {};
}

TEST_SUITE("Salix::reflection::EnumRegistryCoverage1847") {
    TEST_CASE("empty enum data exposes empty names and fallback lookups") {
        Salix::EnumRegistry::EnumData data;
        CHECK(data.get_names().empty());
        CHECK(data.get_name(0) == "Unknown");
        CHECK(data.get_value("anything") == -1);
    }

    TEST_CASE("empty string can be an explicit enum display name") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[4] = "";
        data.ordered_names.push_back("");
        CHECK(data.get_name(4).empty());
        CHECK(data.get_value("") == 4);
    }

    TEST_CASE("negative integer enum values roundtrip through lookup helpers") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[-8] = "Negative";
        CHECK(data.get_name(-8) == "Negative");
        CHECK(data.get_value("Negative") == -8);
    }

    TEST_CASE("extreme integer enum keys remain valid registry data") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[std::numeric_limits<int>::min()] = "Min";
        data.value_to_string[std::numeric_limits<int>::max()] = "Max";
        CHECK(data.get_name(std::numeric_limits<int>::min()) == "Min");
        CHECK(data.get_value("Max") == std::numeric_limits<int>::max());
    }

    TEST_CASE("ordered UI names may intentionally contain values absent from reverse map") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "Mapped";
        data.ordered_names = {"Mapped", "DisplayOnly"};
        CHECK(data.get_names().size() == 2);
        CHECK(data.get_names()[1] == "DisplayOnly");
        CHECK(data.get_value("DisplayOnly") == -1);
    }

    TEST_CASE("custom registration supports an empty value map with ordered display names") {
        Salix::EnumRegistry::EnumData data;
        data.ordered_names = {"A", "B"};
        Salix::EnumRegistry::register_enum(typeid(Batch1847EnumA), std::move(data));
        const auto* stored = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch1847EnumA));
        REQUIRE(stored != nullptr);
        CHECK(stored->get_names().size() == 2);
        CHECK(stored->get_value("A") == -1);
    }

    TEST_CASE("replacing one custom type leaves a different registered type untouched") {
        Salix::EnumRegistry::EnumData a; a.value_to_string[1] = "A1";
        Salix::EnumRegistry::EnumData b; b.value_to_string[2] = "B2";
        Salix::EnumRegistry::register_enum(typeid(Batch1847EnumB), std::move(a));
        Salix::EnumRegistry::register_enum(typeid(Batch1847EnumC), std::move(b));
        Salix::EnumRegistry::EnumData replacement; replacement.value_to_string[3] = "A3";
        Salix::EnumRegistry::register_enum(typeid(Batch1847EnumB), std::move(replacement));
        CHECK(Salix::EnumRegistry::get_enum_data(typeid(Batch1847EnumB)).get_name(3) == "A3");
        CHECK(Salix::EnumRegistry::get_enum_data(typeid(Batch1847EnumC)).get_name(2) == "B2");
    }

    TEST_CASE("pointer lookup and reference lookup share modifications after replacement") {
        Salix::EnumRegistry::EnumData first; first.value_to_string[5] = "Old";
        Salix::EnumRegistry::register_enum(typeid(Batch1847EnumD), std::move(first));
        const auto* before = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch1847EnumD));
        REQUIRE(before != nullptr);
        Salix::EnumRegistry::EnumData second; second.value_to_string[6] = "New";
        Salix::EnumRegistry::register_enum(typeid(Batch1847EnumD), std::move(second));
        const auto* after = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch1847EnumD));
        REQUIRE(after != nullptr);
        CHECK(after->get_name(6) == "New");
        CHECK(&Salix::EnumRegistry::get_enum_data(typeid(Batch1847EnumD)) == after);
    }

    TEST_CASE("register all enums is idempotent for canonical ordered names") {
        Salix::EnumRegistry::register_all_enums();
        const auto first = Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode)).get_names();
        Salix::EnumRegistry::register_all_enums();
        const auto second = Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode)).get_names();
        CHECK(first == second);
        CHECK(second.size() == 2);
    }

    TEST_CASE("canonical enum name lookup remains case sensitive") {
        Salix::EnumRegistry::register_all_enums();
        const auto& projection = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        CHECK(projection.get_value("Perspective") == static_cast<int>(Salix::ProjectionMode::Perspective));
        CHECK(projection.get_value("perspective") == -1);
        CHECK(projection.get_name(999) == "Unknown");
    }
}
