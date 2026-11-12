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

namespace {
struct Batch2147EnumA {};
struct Batch2147EnumB {};
struct Batch2147EnumC {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch2147") {
    TEST_CASE("batch2147 enum data reverse lookup returns mapped value") {
        Salix::EnumRegistry::EnumData data; data.value_to_string[1]="Same"; data.value_to_string[2]="Other";
        CHECK(data.get_value("Same")==1);
    }
    TEST_CASE("batch2147 enum data ordered names remain independent from value map mutation") {
        Salix::EnumRegistry::EnumData data; data.ordered_names={"A","B"}; data.value_to_string[1]="A"; data.value_to_string.clear();
        REQUIRE(data.get_names().size()==2); CHECK(data.get_names()[0]=="A"); CHECK(data.get_names()[1]=="B");
    }
    TEST_CASE("batch2147 registering empty enum data still creates a retrievable entry") {
        Salix::EnumRegistry::register_enum(typeid(Batch2147EnumA), Salix::EnumRegistry::EnumData{});
        const auto* p=Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch2147EnumA)); REQUIRE(p!=nullptr); CHECK(p->get_names().empty());
    }
    TEST_CASE("batch2147 registration preserves explicit negative and positive map keys together") {
        Salix::EnumRegistry::EnumData data; data.value_to_string[-2]="Minus"; data.value_to_string[4]="Plus";
        Salix::EnumRegistry::register_enum(typeid(Batch2147EnumB),std::move(data));
        const auto& stored=Salix::EnumRegistry::get_enum_data(typeid(Batch2147EnumB)); CHECK(stored.get_name(-2)=="Minus"); CHECK(stored.get_name(4)=="Plus");
    }
    TEST_CASE("batch2147 replacing a registration removes names that existed only in the old value map") {
        Salix::EnumRegistry::EnumData first; first.value_to_string[1]="Old"; Salix::EnumRegistry::register_enum(typeid(Batch2147EnumC),std::move(first));
        Salix::EnumRegistry::EnumData second; second.value_to_string[2]="New"; Salix::EnumRegistry::register_enum(typeid(Batch2147EnumC),std::move(second));
        const auto& stored=Salix::EnumRegistry::get_enum_data(typeid(Batch2147EnumC)); CHECK(stored.get_name(1)=="Unknown"); CHECK(stored.get_name(2)=="New");
    }
    TEST_CASE("batch2147 canonical projection ordered names exactly match reverse lookups") {
        Salix::EnumRegistry::register_all_enums(); const auto& d=Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        REQUIRE(d.get_names().size()==2); CHECK(d.get_value(d.get_names()[0])==static_cast<int>(Salix::ProjectionMode::Perspective)); CHECK(d.get_value(d.get_names()[1])==static_cast<int>(Salix::ProjectionMode::Orthographic));
    }
    TEST_CASE("batch2147 canonical editor mode ordered names exactly match reverse lookups") {
        Salix::EnumRegistry::register_all_enums(); const auto& d=Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode));
        REQUIRE(d.get_names().size()==2); CHECK(d.get_value(d.get_names()[0])==static_cast<int>(Salix::EditorDataMode::Live)); CHECK(d.get_value(d.get_names()[1])==static_cast<int>(Salix::EditorDataMode::Yaml));
    }
    TEST_CASE("batch2147 repeated canonical registration preserves pointer availability for both enum types") {
        Salix::EnumRegistry::register_all_enums(); Salix::EnumRegistry::register_all_enums();
        CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Salix::ProjectionMode))!=nullptr); CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Salix::EditorDataMode))!=nullptr);
    }
    TEST_CASE("batch2147 unknown numeric lookup remains stable after canonical re-registration") {
        Salix::EnumRegistry::register_all_enums(); CHECK(Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_name(-999)=="Unknown");
        Salix::EnumRegistry::register_all_enums(); CHECK(Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_name(-999)=="Unknown");
    }
    TEST_CASE("batch2147 unknown text lookup remains case sensitive for canonical editor names") {
        Salix::EnumRegistry::register_all_enums(); const auto& d=Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode));
        CHECK(d.get_value("Live")==static_cast<int>(Salix::EditorDataMode::Live)); CHECK(d.get_value("live")==-1); CHECK(d.get_value("YAML")==-1);
    }
}

namespace {
struct Batch2747EnumA {};
struct Batch2747EnumB {};
struct Batch2747EnumC {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch2747") {
    TEST_CASE("batch2747 unknown type pointer lookup returns null before registration") {
        CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch2747EnumA))==nullptr);
    }
    TEST_CASE("batch2747 registering one value makes pointer lookup immediately available") {
        Salix::EnumRegistry::EnumData data; data.value_to_string[7]="Seven";
        Salix::EnumRegistry::register_enum(typeid(Batch2747EnumA),std::move(data));
        const auto* stored=Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch2747EnumA)); REQUIRE(stored!=nullptr); CHECK(stored->get_name(7)=="Seven");
    }
    TEST_CASE("batch2747 empty ordered names can coexist with populated value map") {
        Salix::EnumRegistry::EnumData data; data.value_to_string[1]="One";
        Salix::EnumRegistry::register_enum(typeid(Batch2747EnumB),std::move(data));
        const auto& stored=Salix::EnumRegistry::get_enum_data(typeid(Batch2747EnumB)); CHECK(stored.get_names().empty()); CHECK(stored.get_name(1)=="One");
    }
    TEST_CASE("batch2747 ordered names can contain labels absent from value map") {
        Salix::EnumRegistry::EnumData data; data.ordered_names={"DisplayOnly"};
        Salix::EnumRegistry::register_enum(typeid(Batch2747EnumC),std::move(data));
        const auto& stored=Salix::EnumRegistry::get_enum_data(typeid(Batch2747EnumC)); REQUIRE(stored.get_names().size()==1); CHECK(stored.get_names()[0]=="DisplayOnly"); CHECK(stored.get_value("DisplayOnly")==-1);
    }
    TEST_CASE("batch2747 canonical projection names remain in declared display order") {
        Salix::EnumRegistry::register_all_enums(); const auto& d=Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        REQUIRE(d.get_names().size()==2); CHECK(d.get_names()[0]=="Perspective"); CHECK(d.get_names()[1]=="Orthographic");
    }
    TEST_CASE("batch2747 canonical editor names remain in declared display order") {
        Salix::EnumRegistry::register_all_enums(); const auto& d=Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode));
        REQUIRE(d.get_names().size()==2); CHECK(d.get_names()[0]=="Live"); CHECK(d.get_names()[1]=="Yaml");
    }
    TEST_CASE("batch2747 projection reverse lookups cover both canonical values") {
        Salix::EnumRegistry::register_all_enums(); const auto& d=Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        CHECK(d.get_name(static_cast<int>(Salix::ProjectionMode::Perspective))=="Perspective");
        CHECK(d.get_name(static_cast<int>(Salix::ProjectionMode::Orthographic))=="Orthographic");
    }
    TEST_CASE("batch2747 editor reverse lookups cover both canonical values") {
        Salix::EnumRegistry::register_all_enums(); const auto& d=Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode));
        CHECK(d.get_name(static_cast<int>(Salix::EditorDataMode::Live))=="Live");
        CHECK(d.get_name(static_cast<int>(Salix::EditorDataMode::Yaml))=="Yaml");
    }
    TEST_CASE("batch2747 repeated canonical registration keeps projection lookup content unchanged") {
        Salix::EnumRegistry::register_all_enums(); const auto first=Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_names();
        Salix::EnumRegistry::register_all_enums(); const auto second=Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_names();
        CHECK(first==second);
    }
    TEST_CASE("batch2747 unknown canonical numeric values map to Unknown") {
        Salix::EnumRegistry::register_all_enums();
        CHECK(Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_name(2747)=="Unknown");
        CHECK(Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode)).get_name(2747)=="Unknown");
    }
}


#include <stdexcept>

namespace {
struct Batch3047EnumA {};
struct Batch3047EnumB {};
struct Batch3047EnumC {};
struct Batch3047EnumD {};
struct Batch3047EnumNeverRegistered {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch3047") {
    TEST_CASE("batch3047 custom enum supports zero numeric key and empty display string") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[0] = "";
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumA), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3047EnumA));
        CHECK(stored.get_name(0).empty());
        CHECK(stored.get_value("") == 0);
    }

    TEST_CASE("batch3047 custom enum preserves whitespace rich names exactly") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[7] = "  Seven  ";
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumB), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3047EnumB));
        CHECK(stored.get_name(7) == "  Seven  ");
        CHECK(stored.get_value("Seven") == -1);
        CHECK(stored.get_value("  Seven  ") == 7);
    }

    TEST_CASE("batch3047 ordered names preserve duplicates and insertion order") {
        Salix::EnumRegistry::EnumData data;
        data.ordered_names = {"First", "First", "Second"};
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumC), std::move(data));
        const auto& names = Salix::EnumRegistry::get_enum_data(typeid(Batch3047EnumC)).get_names();
        REQUIRE(names.size() == 3);
        CHECK(names[0] == "First");
        CHECK(names[1] == "First");
        CHECK(names[2] == "Second");
    }

    TEST_CASE("batch3047 pointer lookup and reference lookup identify the same stored object") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[3] = "Three";
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumD), std::move(data));
        const auto* ptr = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch3047EnumD));
        REQUIRE(ptr != nullptr);
        CHECK(ptr == &Salix::EnumRegistry::get_enum_data(typeid(Batch3047EnumD)));
    }

    TEST_CASE("batch3047 stored enum pointer remains valid after unrelated registrations") {
        Salix::EnumRegistry::EnumData first;
        first.value_to_string[1] = "One";
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumA), std::move(first));
        const auto* before = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch3047EnumA));
        REQUIRE(before != nullptr);
        Salix::EnumRegistry::EnumData other;
        other.value_to_string[2] = "Two";
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumB), std::move(other));
        CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch3047EnumA)) == before);
        CHECK(before->get_name(1) == "One");
    }

    TEST_CASE("batch3047 replacing one registration updates content at its existing registry entry") {
        Salix::EnumRegistry::EnumData first;
        first.value_to_string[1] = "Before";
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumC), std::move(first));
        const auto* before = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch3047EnumC));
        REQUIRE(before != nullptr);
        Salix::EnumRegistry::EnumData replacement;
        replacement.value_to_string[2] = "After";
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumC), std::move(replacement));
        const auto* after = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch3047EnumC));
        REQUIRE(after != nullptr);
        CHECK(after == before);
        CHECK(after->get_name(1) == "Unknown");
        CHECK(after->get_name(2) == "After");
    }

    TEST_CASE("batch3047 canonical registration does not remove unrelated custom enum data") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[3047] = "Custom";
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumD), std::move(data));
        Salix::EnumRegistry::register_all_enums();
        const auto* custom = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch3047EnumD));
        REQUIRE(custom != nullptr);
        CHECK(custom->get_name(3047) == "Custom");
    }

    TEST_CASE("batch3047 custom registration does not disturb canonical projection data") {
        Salix::EnumRegistry::register_all_enums();
        const auto before = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_names();
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[9] = "Nine";
        Salix::EnumRegistry::register_enum(typeid(Batch3047EnumA), std::move(data));
        const auto after = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_names();
        CHECK(after == before);
    }

    TEST_CASE("batch3047 missing pointer lookup remains null even after canonical registrations") {
        Salix::EnumRegistry::register_all_enums();
        CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch3047EnumNeverRegistered)) == nullptr);
    }

    TEST_CASE("batch3047 reference lookup of an unregistered type preserves out of range failure contract") {
        CHECK_THROWS_AS(
            Salix::EnumRegistry::get_enum_data(typeid(Batch3047EnumNeverRegistered)),
            std::out_of_range);
    }
}
namespace {
struct Batch3447EnumA {};
struct Batch3447EnumB {};
struct Batch3447EnumC {};
struct Batch3447EnumD {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch3447") {
    TEST_CASE("batch3447 embedded null display string roundtrips through reverse lookup") {
        const std::string label("Enum\0Label", 10);
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[3447] = label;
        Salix::EnumRegistry::register_enum(typeid(Batch3447EnumA), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3447EnumA));
        CHECK(stored.get_name(3447) == label);
        CHECK(stored.get_value(label) == 3447);
    }

    TEST_CASE("batch3447 case distinct display strings map to independent numeric values") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "Mode3447";
        data.value_to_string[2] = "mode3447";
        Salix::EnumRegistry::register_enum(typeid(Batch3447EnumB), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3447EnumB));
        CHECK(stored.get_value("Mode3447") == 1);
        CHECK(stored.get_value("mode3447") == 2);
    }

    TEST_CASE("batch3447 replacing custom registration with empty data removes old maps and ordered names") {
        Salix::EnumRegistry::EnumData first;
        first.value_to_string[7] = "Seven3447";
        first.ordered_names = {"Seven3447"};
        Salix::EnumRegistry::register_enum(typeid(Batch3447EnumC), std::move(first));
        Salix::EnumRegistry::EnumData empty;
        Salix::EnumRegistry::register_enum(typeid(Batch3447EnumC), std::move(empty));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3447EnumC));
        CHECK(stored.get_name(7) == "Unknown");
        CHECK(stored.get_value("Seven3447") == -1);
        CHECK(stored.get_names().empty());
    }

    TEST_CASE("batch3447 canonical registration restores ProjectionMode after custom replacement") {
        Salix::EnumRegistry::EnumData replacement;
        replacement.value_to_string[3447] = "Temporary3447";
        replacement.ordered_names = {"Temporary3447"};
        Salix::EnumRegistry::register_enum(typeid(Salix::ProjectionMode), std::move(replacement));
        Salix::EnumRegistry::register_all_enums();
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        CHECK(stored.get_value("Perspective") == static_cast<int>(Salix::ProjectionMode::Perspective));
        CHECK(stored.get_value("Orthographic") == static_cast<int>(Salix::ProjectionMode::Orthographic));
        CHECK(stored.get_value("Temporary3447") == -1);
    }

    TEST_CASE("batch3447 canonical registration restores EditorDataMode after custom replacement") {
        Salix::EnumRegistry::EnumData replacement;
        replacement.value_to_string[3447] = "Temporary3447";
        replacement.ordered_names = {"Temporary3447"};
        Salix::EnumRegistry::register_enum(typeid(Salix::EditorDataMode), std::move(replacement));
        Salix::EnumRegistry::register_all_enums();
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode));
        CHECK(stored.get_value("Live") == static_cast<int>(Salix::EditorDataMode::Live));
        CHECK(stored.get_value("Yaml") == static_cast<int>(Salix::EditorDataMode::Yaml));
        CHECK(stored.get_value("Temporary3447") == -1);
    }

    TEST_CASE("batch3447 unrelated custom registration survives repeated canonical registration") {
        Salix::EnumRegistry::EnumData custom;
        custom.value_to_string[-3447] = "Custom3447";
        Salix::EnumRegistry::register_enum(typeid(Batch3447EnumD), std::move(custom));
        Salix::EnumRegistry::register_all_enums();
        Salix::EnumRegistry::register_all_enums();
        const auto* stored = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch3447EnumD));
        REQUIRE(stored != nullptr);
        CHECK(stored->get_name(-3447) == "Custom3447");
    }

    TEST_CASE("batch3447 repeated get names calls expose the same stored vector reference") {
        Salix::EnumRegistry::EnumData data;
        data.ordered_names = {"A3447", "B3447"};
        Salix::EnumRegistry::register_enum(typeid(Batch3447EnumA), std::move(data));
        const auto& first = Salix::EnumRegistry::get_enum_data(typeid(Batch3447EnumA)).get_names();
        const auto& second = Salix::EnumRegistry::get_enum_data(typeid(Batch3447EnumA)).get_names();
        CHECK(&first == &second);
        REQUIRE(first.size() == 2);
    }

    TEST_CASE("batch3447 mapped values remain retrievable when omitted from ordered UI names") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[10] = "Visible3447";
        data.value_to_string[20] = "Hidden3447";
        data.ordered_names = {"Visible3447"};
        Salix::EnumRegistry::register_enum(typeid(Batch3447EnumB), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3447EnumB));
        CHECK(stored.get_value("Hidden3447") == 20);
        CHECK(stored.get_name(20) == "Hidden3447");
        REQUIRE(stored.get_names().size() == 1);
    }

    TEST_CASE("batch3447 ProjectionMode registry object address survives canonical refresh") {
        Salix::EnumRegistry::register_all_enums();
        const auto* before = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Salix::ProjectionMode));
        REQUIRE(before != nullptr);
        Salix::EnumRegistry::register_all_enums();
        const auto* after = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Salix::ProjectionMode));
        CHECK(after == before);
    }

    TEST_CASE("batch3447 EditorDataMode registry object address survives canonical refresh") {
        Salix::EnumRegistry::register_all_enums();
        const auto* before = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Salix::EditorDataMode));
        REQUIRE(before != nullptr);
        Salix::EnumRegistry::register_all_enums();
        const auto* after = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Salix::EditorDataMode));
        CHECK(after == before);
    }
}
namespace {
struct Batch3847EnumA {};
struct Batch3847EnumB {};
struct Batch3847EnumC {};
struct Batch3847EnumD {};
struct Batch3847NeverRegistered {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch3847") {
    TEST_CASE("batch3847 enum data supports minimum and maximum signed integer keys") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[(std::numeric_limits<int>::min)()] = "Min3847";
        data.value_to_string[(std::numeric_limits<int>::max)()] = "Max3847";
        CHECK(data.get_name((std::numeric_limits<int>::min)()) == "Min3847");
        CHECK(data.get_name((std::numeric_limits<int>::max)()) == "Max3847");
        CHECK(data.get_value("Min3847") == (std::numeric_limits<int>::min)());
        CHECK(data.get_value("Max3847") == (std::numeric_limits<int>::max)());
    }

    TEST_CASE("batch3847 empty enum data returns stable unknown fallbacks") {
        Salix::EnumRegistry::EnumData data;
        CHECK(data.get_name(0) == "Unknown");
        CHECK(data.get_name(-3847) == "Unknown");
        CHECK(data.get_value("") == -1);
        CHECK(data.get_value("Missing3847") == -1);
        CHECK(data.get_names().empty());
    }

    TEST_CASE("batch3847 ordered names may intentionally contain entries absent from value map") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "Mapped3847";
        data.ordered_names = {"Mapped3847", "DisplayOnly3847"};
        Salix::EnumRegistry::register_enum(typeid(Batch3847EnumA), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3847EnumA));
        REQUIRE(stored.get_names().size() == 2);
        CHECK(stored.get_names()[1] == "DisplayOnly3847");
        CHECK(stored.get_value("DisplayOnly3847") == -1);
    }

    TEST_CASE("batch3847 value map may contain entries absent from ordered UI names") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[7] = "Hidden3847";
        Salix::EnumRegistry::register_enum(typeid(Batch3847EnumB), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3847EnumB));
        CHECK(stored.get_name(7) == "Hidden3847");
        CHECK(stored.get_value("Hidden3847") == 7);
        CHECK(stored.get_names().empty());
    }

    TEST_CASE("batch3847 replacement with empty data clears every previous mapping") {
        Salix::EnumRegistry::EnumData first;
        first.value_to_string[4] = "Four3847";
        first.ordered_names = {"Four3847"};
        Salix::EnumRegistry::register_enum(typeid(Batch3847EnumC), std::move(first));
        Salix::EnumRegistry::EnumData empty;
        Salix::EnumRegistry::register_enum(typeid(Batch3847EnumC), std::move(empty));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3847EnumC));
        CHECK(stored.get_name(4) == "Unknown");
        CHECK(stored.get_value("Four3847") == -1);
        CHECK(stored.get_names().empty());
    }

    TEST_CASE("batch3847 embedded null enum display name roundtrips by full string identity") {
        const std::string name("Enum\0Name3847", 13);
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[38] = name;
        data.ordered_names = {name};
        Salix::EnumRegistry::register_enum(typeid(Batch3847EnumD), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch3847EnumD));
        CHECK(stored.get_name(38) == name);
        CHECK(stored.get_value(name) == 38);
        CHECK(stored.get_names().front().size() == name.size());
    }

    TEST_CASE("batch3847 missing pointer lookup stays null after unrelated registrations") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "One3847";
        Salix::EnumRegistry::register_enum(typeid(Batch3847EnumA), std::move(data));
        CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch3847NeverRegistered)) == nullptr);
    }

    TEST_CASE("batch3847 canonical refresh leaves unknown lookup fallback unchanged") {
        Salix::EnumRegistry::register_all_enums();
        const auto& projection = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        CHECK(projection.get_name(9993847) == "Unknown");
        CHECK(projection.get_value("NotAProjection3847") == -1);
    }

    TEST_CASE("batch3847 canonical projection ordered names align with mapped values") {
        Salix::EnumRegistry::register_all_enums();
        const auto& projection = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        const auto& names = projection.get_names();
        REQUIRE(names.size() == 2);
        CHECK(projection.get_value(names[0]) == static_cast<int>(Salix::ProjectionMode::Perspective));
        CHECK(projection.get_value(names[1]) == static_cast<int>(Salix::ProjectionMode::Orthographic));
    }

    TEST_CASE("batch3847 canonical editor ordered names align with mapped values") {
        Salix::EnumRegistry::register_all_enums();
        const auto& editor = Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode));
        const auto& names = editor.get_names();
        REQUIRE(names.size() == 2);
        CHECK(editor.get_value(names[0]) == static_cast<int>(Salix::EditorDataMode::Live));
        CHECK(editor.get_value(names[1]) == static_cast<int>(Salix::EditorDataMode::Yaml));
    }
}

namespace {
struct Batch4447EnumA {};
struct Batch4447EnumB {};
struct Batch4447EnumC {};
struct Batch4447NeverRegistered {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch4447") {
    TEST_CASE("batch4447 custom enum supports zero negative and positive integer mappings together") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[-4] = "Negative4447";
        data.value_to_string[0] = "Zero4447";
        data.value_to_string[47] = "Positive4447";
        CHECK(data.get_name(-4) == "Negative4447");
        CHECK(data.get_name(0) == "Zero4447");
        CHECK(data.get_value("Positive4447") == 47);
    }

    TEST_CASE("batch4447 empty display name can be a deliberate mapped value") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[4447] = "";
        data.ordered_names = {""};
        Salix::EnumRegistry::register_enum(typeid(Batch4447EnumA), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch4447EnumA));
        CHECK(stored.get_name(4447).empty());
        CHECK(stored.get_value("") == 4447);
        REQUIRE(stored.get_names().size() == 1);
        CHECK(stored.get_names()[0].empty());
    }

    TEST_CASE("batch4447 reregistering custom type replaces both value map and ordered names") {
        Salix::EnumRegistry::EnumData first;
        first.value_to_string[1] = "First4447";
        first.ordered_names = {"First4447"};
        Salix::EnumRegistry::register_enum(typeid(Batch4447EnumB), std::move(first));
        Salix::EnumRegistry::EnumData second;
        second.value_to_string[2] = "Second4447";
        second.ordered_names = {"Second4447"};
        Salix::EnumRegistry::register_enum(typeid(Batch4447EnumB), std::move(second));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch4447EnumB));
        CHECK(stored.get_name(1) == "Unknown");
        CHECK(stored.get_name(2) == "Second4447");
        CHECK(stored.get_names() == std::vector<std::string>{"Second4447"});
    }

    TEST_CASE("batch4447 two custom type registrations retain independent data") {
        Salix::EnumRegistry::EnumData a;
        a.value_to_string[10] = "A4447";
        Salix::EnumRegistry::EnumData b;
        b.value_to_string[20] = "B4447";
        Salix::EnumRegistry::register_enum(typeid(Batch4447EnumA), std::move(a));
        Salix::EnumRegistry::register_enum(typeid(Batch4447EnumB), std::move(b));
        CHECK(Salix::EnumRegistry::get_enum_data(typeid(Batch4447EnumA)).get_name(10) == "A4447");
        CHECK(Salix::EnumRegistry::get_enum_data(typeid(Batch4447EnumB)).get_name(20) == "B4447");
    }

    TEST_CASE("batch4447 ordered names preserve duplicate display entries exactly") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "Repeated4447";
        data.ordered_names = {"Repeated4447", "Repeated4447"};
        Salix::EnumRegistry::register_enum(typeid(Batch4447EnumC), std::move(data));
        const auto& names = Salix::EnumRegistry::get_enum_data(typeid(Batch4447EnumC)).get_names();
        REQUIRE(names.size() == 2);
        CHECK(names[0] == "Repeated4447");
        CHECK(names[1] == "Repeated4447");
    }

    TEST_CASE("batch4447 ordered name reference remains stable across value and name lookups") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[4] = "Four4447";
        data.ordered_names = {"Four4447"};
        Salix::EnumRegistry::register_enum(typeid(Batch4447EnumA), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch4447EnumA));
        const auto* names = &stored.get_names();
        CHECK(stored.get_name(4) == "Four4447");
        CHECK(stored.get_value("Four4447") == 4);
        CHECK(&stored.get_names() == names);
    }

    TEST_CASE("batch4447 canonical projection names retain public UI order") {
        Salix::EnumRegistry::register_all_enums();
        const auto& names = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_names();
        REQUIRE(names.size() == 2);
        CHECK(names[0] == "Perspective");
        CHECK(names[1] == "Orthographic");
    }

    TEST_CASE("batch4447 canonical editor mode names retain public UI order") {
        Salix::EnumRegistry::register_all_enums();
        const auto& names = Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode)).get_names();
        REQUIRE(names.size() == 2);
        CHECK(names[0] == "Live");
        CHECK(names[1] == "Yaml");
    }

    TEST_CASE("batch4447 canonical refresh never removes unrelated custom enum registration") {
        Salix::EnumRegistry::EnumData custom;
        custom.value_to_string[4447] = "Custom4447";
        Salix::EnumRegistry::register_enum(typeid(Batch4447EnumC), std::move(custom));
        Salix::EnumRegistry::register_all_enums();
        const auto* stored = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch4447EnumC));
        REQUIRE(stored != nullptr);
        CHECK(stored->get_name(4447) == "Custom4447");
    }

    TEST_CASE("batch4447 never registered type remains a null pointer after canonical refresh") {
        Salix::EnumRegistry::register_all_enums();
        CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch4447NeverRegistered)) == nullptr);
    }
}

namespace {
struct Batch4947EnumA {};
struct Batch4947EnumB {};
struct Batch4947EnumC {};
struct Batch4947EnumD {};
struct Batch4947NeverRegistered {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch4947") {
    TEST_CASE("batch4947 whitespace and newline display name roundtrips exactly") {
        const std::string name = "  Value 4947\n";
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[49] = name;
        data.ordered_names = {name};
        Salix::EnumRegistry::register_enum(typeid(Batch4947EnumA), std::move(data));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch4947EnumA));
        CHECK(stored.get_name(49) == name);
        CHECK(stored.get_value(name) == 49);
        CHECK(stored.get_names().front() == name);
    }

    TEST_CASE("batch4947 pointer and reference lookup identify the same registered object") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "One4947";
        Salix::EnumRegistry::register_enum(typeid(Batch4947EnumB), std::move(data));
        const auto* pointer = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch4947EnumB));
        REQUIRE(pointer != nullptr);
        const auto& reference = Salix::EnumRegistry::get_enum_data(typeid(Batch4947EnumB));
        CHECK(pointer == &reference);
    }

    TEST_CASE("batch4947 custom enum object address remains stable when mapping is replaced") {
        Salix::EnumRegistry::EnumData first;
        first.value_to_string[1] = "First4947";
        Salix::EnumRegistry::register_enum(typeid(Batch4947EnumC), std::move(first));
        const auto* before = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch4947EnumC));
        REQUIRE(before != nullptr);
        Salix::EnumRegistry::EnumData second;
        second.value_to_string[2] = "Second4947";
        second.ordered_names = {"Second4947"};
        Salix::EnumRegistry::register_enum(typeid(Batch4947EnumC), std::move(second));
        const auto* after = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch4947EnumC));
        REQUIRE(after != nullptr);
        CHECK(after == before);
        CHECK(after->get_name(2) == "Second4947");
    }

    TEST_CASE("batch4947 direct reference lookup of never registered type throws out of range") {
        CHECK_THROWS_AS(
            Salix::EnumRegistry::get_enum_data(typeid(Batch4947NeverRegistered)),
            std::out_of_range);
    }

    TEST_CASE("batch4947 ordered names retain caller order independently from numeric key order") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[100] = "Hundred4947";
        data.value_to_string[-1] = "Negative4947";
        data.value_to_string[0] = "Zero4947";
        data.ordered_names = {"Zero4947", "Hundred4947", "Negative4947"};
        Salix::EnumRegistry::register_enum(typeid(Batch4947EnumD), std::move(data));
        const auto& names = Salix::EnumRegistry::get_enum_data(typeid(Batch4947EnumD)).get_names();
        REQUIRE(names.size() == 3);
        CHECK(names[0] == "Zero4947");
        CHECK(names[1] == "Hundred4947");
        CHECK(names[2] == "Negative4947");
    }

    TEST_CASE("batch4947 enum display name lookup remains case sensitive") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[7] = "Case4947";
        CHECK(data.get_value("Case4947") == 7);
        CHECK(data.get_value("case4947") == -1);
        CHECK(data.get_value("CASE4947") == -1);
    }

    TEST_CASE("batch4947 canonical refresh repairs deliberately replaced projection mapping") {
        Salix::EnumRegistry::EnumData custom;
        custom.value_to_string[4947] = "CustomProjection4947";
        custom.ordered_names = {"CustomProjection4947"};
        Salix::EnumRegistry::register_enum(typeid(Salix::ProjectionMode), std::move(custom));
        CHECK(Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)).get_name(4947) ==
              "CustomProjection4947");
        Salix::EnumRegistry::register_all_enums();
        const auto& projection = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        CHECK(projection.get_name(static_cast<int>(Salix::ProjectionMode::Perspective)) == "Perspective");
        CHECK(projection.get_name(static_cast<int>(Salix::ProjectionMode::Orthographic)) == "Orthographic");
        CHECK(projection.get_name(4947) == "Unknown");
    }

    TEST_CASE("batch4947 canonical projection pointer and reference stay identical after refresh") {
        Salix::EnumRegistry::register_all_enums();
        const auto* pointer = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Salix::ProjectionMode));
        REQUIRE(pointer != nullptr);
        CHECK(pointer == &Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode)));
    }

    TEST_CASE("batch4947 unrelated custom registration survives two canonical refreshes") {
        Salix::EnumRegistry::EnumData custom;
        custom.value_to_string[4947] = "Persistent4947";
        Salix::EnumRegistry::register_enum(typeid(Batch4947EnumA), std::move(custom));
        Salix::EnumRegistry::register_all_enums();
        Salix::EnumRegistry::register_all_enums();
        const auto* stored = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch4947EnumA));
        REQUIRE(stored != nullptr);
        CHECK(stored->get_name(4947) == "Persistent4947");
    }

    TEST_CASE("batch4947 never registered pointer remains null after custom and canonical registrations") {
        Salix::EnumRegistry::EnumData custom;
        custom.value_to_string[1] = "One4947";
        Salix::EnumRegistry::register_enum(typeid(Batch4947EnumB), std::move(custom));
        Salix::EnumRegistry::register_all_enums();
        CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch4947NeverRegistered)) == nullptr);
    }
}

namespace {
struct Batch5147EnumA {};
struct Batch5147EnumB {};
struct Batch5147EnumC {};
struct Batch5147NeverRegistered {};
}

TEST_SUITE("Salix::reflection::EnumRegistryBatch5147") {
    TEST_CASE("batch5147 empty enum data exposes stable fallback behavior") {
        Salix::EnumRegistry::EnumData data;
        CHECK(data.get_name(0) == "Unknown");
        CHECK(data.get_value("Missing5147") == -1);
        CHECK(data.get_names().empty());
    }

    TEST_CASE("batch5147 negative and zero numeric keys roundtrip independently") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[-51] = "Negative5147";
        data.value_to_string[0] = "Zero5147";
        CHECK(data.get_name(-51) == "Negative5147");
        CHECK(data.get_name(0) == "Zero5147");
        CHECK(data.get_value("Negative5147") == -51);
        CHECK(data.get_value("Zero5147") == 0);
    }

    TEST_CASE("batch5147 empty display name can be intentionally mapped") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[5147] = "";
        CHECK(data.get_name(5147).empty());
        CHECK(data.get_value("") == 5147);
    }

    TEST_CASE("batch5147 ordered names remain independently empty when numeric mappings exist") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[1] = "One5147";
        CHECK(data.get_name(1) == "One5147");
        CHECK(data.get_names().empty());
    }

    TEST_CASE("batch5147 custom registration pointer and reference resolve same object") {
        Salix::EnumRegistry::EnumData data;
        data.value_to_string[51] = "FiftyOne5147";
        data.ordered_names = {"FiftyOne5147"};
        Salix::EnumRegistry::register_enum(typeid(Batch5147EnumA), std::move(data));
        const auto* pointer = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch5147EnumA));
        REQUIRE(pointer != nullptr);
        CHECK(pointer == &Salix::EnumRegistry::get_enum_data(typeid(Batch5147EnumA)));
    }

    TEST_CASE("batch5147 replacing registration removes old numeric mapping and ordered names") {
        Salix::EnumRegistry::EnumData first;
        first.value_to_string[1] = "Old5147";
        first.ordered_names = {"Old5147"};
        Salix::EnumRegistry::register_enum(typeid(Batch5147EnumB), std::move(first));
        Salix::EnumRegistry::EnumData second;
        second.value_to_string[2] = "New5147";
        second.ordered_names = {"New5147"};
        Salix::EnumRegistry::register_enum(typeid(Batch5147EnumB), std::move(second));
        const auto& stored = Salix::EnumRegistry::get_enum_data(typeid(Batch5147EnumB));
        CHECK(stored.get_name(1) == "Unknown");
        CHECK(stored.get_name(2) == "New5147");
        REQUIRE(stored.get_names().size() == 1);
        CHECK(stored.get_names()[0] == "New5147");
    }

    TEST_CASE("batch5147 canonical projection registration preserves UI ordering") {
        Salix::EnumRegistry::register_all_enums();
        const auto& data = Salix::EnumRegistry::get_enum_data(typeid(Salix::ProjectionMode));
        const auto& names = data.get_names();
        REQUIRE(names.size() == 2);
        CHECK(names[0] == "Perspective");
        CHECK(names[1] == "Orthographic");
    }

    TEST_CASE("batch5147 canonical editor data registration preserves UI ordering") {
        Salix::EnumRegistry::register_all_enums();
        const auto& data = Salix::EnumRegistry::get_enum_data(typeid(Salix::EditorDataMode));
        const auto& names = data.get_names();
        REQUIRE(names.size() == 2);
        CHECK(names[0] == "Live");
        CHECK(names[1] == "Yaml");
        CHECK(data.get_value("Live") == static_cast<int>(Salix::EditorDataMode::Live));
        CHECK(data.get_value("Yaml") == static_cast<int>(Salix::EditorDataMode::Yaml));
    }

    TEST_CASE("batch5147 unrelated custom registration survives canonical refresh") {
        Salix::EnumRegistry::EnumData custom;
        custom.value_to_string[5147] = "Persistent5147";
        Salix::EnumRegistry::register_enum(typeid(Batch5147EnumC), std::move(custom));
        Salix::EnumRegistry::register_all_enums();
        const auto* stored = Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch5147EnumC));
        REQUIRE(stored != nullptr);
        CHECK(stored->get_name(5147) == "Persistent5147");
    }

    TEST_CASE("batch5147 never registered pointer remains null after canonical refresh") {
        Salix::EnumRegistry::register_all_enums();
        CHECK(Salix::EnumRegistry::get_enum_data_as_ptr(typeid(Batch5147NeverRegistered)) == nullptr);
    }
}

