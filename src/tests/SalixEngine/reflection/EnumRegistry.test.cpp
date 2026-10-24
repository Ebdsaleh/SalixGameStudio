#include <doctest.h>
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/reflection/EditorDataMode.h>
#include <Salix/rendering/ICamera.h>
#include <stdexcept>
#include <typeindex>
#include <utility>
#include <limits>
#include <string>

namespace {
struct EnumRegistryUnregisteredTag {};
struct EnumRegistryTagA {};
struct EnumRegistryTagB {};
struct EnumRegistryTagC {};
struct EnumRegistryTagD {};
struct EnumRegistryTagE {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch1047") {
    TEST_CASE("pointer lookup safely returns null for an unregistered type") {
        CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(EnumRegistryUnregisteredTag)) == nullptr);
    }

    TEST_CASE("reference lookup preserves explicit failure for an unregistered type") {
        CHECK_THROWS_AS(
            Salix::EnumRegistry::get_enum_data(typeid(EnumRegistryUnregisteredTag)),
            std::out_of_range);
    }

    TEST_CASE("custom registration maps integer values back to names") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[10] = "Ten";
        data.value_to_string[20] = "Twenty";
        data.ordered_names = {"Ten", "Twenty"};
        Salix::EnumRegistry::register_enum(typeid(EnumRegistryTagA), std::move(data));
        const auto* stored = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(EnumRegistryTagA));
        REQUIRE(stored != nullptr);
        CHECK(stored->get_name(10) == "Ten");
        CHECK(stored->get_name(20) == "Twenty");
    }

    TEST_CASE("enum data reports stable fallback values for unknown names and numbers") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "One";
        CHECK(data.get_name(999) == "Unknown");
        CHECK(data.get_value("Missing") == -1);
    }

    TEST_CASE("ordered names preserve UI order independently of map key order") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[30] = "Third";
        data.value_to_string[10] = "First";
        data.value_to_string[20] = "Second";
        data.ordered_names = {"First", "Second", "Third"};
        Salix::EnumRegistry::register_enum(typeid(EnumRegistryTagB), std::move(data));
        const auto& names = Salix::EnumRegistry::get_enum_data(typeid(EnumRegistryTagB)).get_names();
        REQUIRE(names.size() == 3);
        CHECK(names[0] == "First");
        CHECK(names[1] == "Second");
        CHECK(names[2] == "Third");
    }

    TEST_CASE("registering the same type again replaces its previous enum definition") {
        Salix::EnumRegistry::EnumData first;
        first.value_to_string[1] = "Old";
        first.ordered_names = {"Old"};
        Salix::EnumRegistry::register_enum(typeid(EnumRegistryTagC), std::move(first));

        Salix::EnumRegistry::EnumData second;
        second.value_to_string[2] = "New";
        second.ordered_names = {"New"};
        Salix::EnumRegistry::register_enum(typeid(EnumRegistryTagC), std::move(second));

        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(EnumRegistryTagC));
        CHECK(stored.get_name(1) == "Unknown");
        CHECK(stored.get_name(2) == "New");
        REQUIRE(stored.get_names().size() == 1);
        CHECK(stored.get_names()[0] == "New");
    }

    TEST_CASE("register all enums exposes canonical projection mode mappings") {
        Salix::EnumRegistry::register_all_enums();
        const auto& data = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        CHECK(data.get_name(static_cast<int>(Salix::ProjectionMode::Perspective)) == "Perspective");
        CHECK(data.get_name(static_cast<int>(Salix::ProjectionMode::Orthographic)) == "Orthographic");
        CHECK(data.get_value("Perspective") == static_cast<int>(Salix::ProjectionMode::Perspective));
        CHECK(data.get_value("Orthographic") == static_cast<int>(Salix::ProjectionMode::Orthographic));
    }

    TEST_CASE("register all enums exposes canonical editor data mode mappings") {
        Salix::EnumRegistry::register_all_enums();
        const auto& data = Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode));
        CHECK(data.get_name(static_cast<int>(Salix::EditorDataMode::Live)) == "Live");
        CHECK(data.get_name(static_cast<int>(Salix::EditorDataMode::Yaml)) == "Yaml");
        REQUIRE(data.get_names().size() == 2);
        CHECK(data.get_names()[0] == "Live");
        CHECK(data.get_names()[1] == "Yaml");
    }

    TEST_CASE("pointer and reference lookups expose the same registered data object") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[7] = "Seven";
        data.ordered_names = {"Seven"};
        Salix::EnumRegistry::register_enum(typeid(EnumRegistryTagD), std::move(data));
        const auto* pointer = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(EnumRegistryTagD));
        const auto& reference = Salix::EnumRegistry::get_enum_data(typeid(EnumRegistryTagD));
        REQUIRE(pointer != nullptr);
        CHECK(pointer == &reference);
        CHECK(pointer->get_value("Seven") == 7);
    }

    TEST_CASE("re-registering canonical enums restores projection data after replacement") {
        Salix::EnumRegistry::EnumData temporary;
        temporary.value_to_string[99] = "Temporary";
        temporary.ordered_names = {"Temporary"};
        Salix::EnumRegistry::register_enum(typeid(Salix::ProjectionMode), std::move(temporary));
        CHECK(Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_name(99) == "Temporary");

        Salix::EnumRegistry::register_all_enums();
        const auto& restored = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        CHECK(restored.get_name(99) == "Unknown");
        REQUIRE(restored.get_names().size() == 2);
        CHECK(restored.get_names()[0] == "Perspective");
        CHECK(restored.get_names()[1] == "Orthographic");
    }
}

namespace {
struct Batch1847EnumA {};
struct Batch1847EnumB {};
struct Batch1847EnumC {};
struct Batch1847EnumD {};
struct Batch1847EnumE {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch1847") {
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
