#include <doctest.h>
#include <Salix/reflection/ui/TypeDrawerLive.h>
#include <Salix/reflection/ByteMirror.h>
#include <stdexcept>

TEST_SUITE("Salix::reflection::TypeDrawerLiveBatch1247") {
    TEST_CASE("type drawer registry installs integer drawer without invoking ImGui") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Int)));
    }

    TEST_CASE("type drawer registry installs floating point drawer") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Float)));
    }

    TEST_CASE("type drawer registry installs boolean drawer") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Bool)));
    }

    TEST_CASE("type drawer registry installs Vector2 drawer") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Vector2)));
    }

    TEST_CASE("type drawer registry installs Vector3 drawer") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Vector3)));
    }

    TEST_CASE("type drawer registry installs color drawer") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Color)));
    }

    TEST_CASE("type drawer registry installs point drawer") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Point)));
    }

    TEST_CASE("type drawer registry installs string drawer") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::String)));
    }

    TEST_CASE("type drawer registry installs read-only matrix drawer") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::GlmMat4)));
    }

    TEST_CASE("type drawer registry exposes nested and enum drawers while unsupported types remain explicit") {
        Salix::TypeDrawerLive::register_all_type_drawers();
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Class)));
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Struct)));
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Enum)));
        CHECK(static_cast<bool>(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::EnumClass)));
        CHECK_THROWS_AS(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::UInt64), std::out_of_range);
        CHECK_THROWS_AS(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Rect), std::out_of_range);
    }
}
