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

#include <Salix/input/ImGuiInputManager.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
#include <Salix/math/Point.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <string>

namespace {
struct Batch6947DrawerFrame {
    ImGuiContext* previous_test = ImGui::GetCurrentContext();
    ImGuiContext* previous_engine = Salix::ImGuiInputManager::get_context_for_testing();
    ImGuiContext* owned = ImGui::CreateContext();
    Batch6947DrawerFrame() {
        ImGui::SetCurrentContext(owned);
        Salix::ImGuiInputManager::set_context_for_testing(owned);
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(800.0f, 600.0f);
        io.DeltaTime = 1.0f / 60.0f;
        if (io.Fonts->Fonts.empty()) io.Fonts->AddFontDefault();
        io.Fonts->Build();
        ImGui::NewFrame();
        ImGui::Begin("Batch6947DrawerWindow");
        Salix::TypeDrawerLive::register_all_type_drawers();
    }
    ~Batch6947DrawerFrame() {
        ImGui::End();
        ImGui::EndFrame();
        Salix::ImGuiInputManager::set_context_for_testing(previous_engine);
        ImGui::DestroyContext(owned);
        ImGui::SetCurrentContext(previous_test);
    }
};
template<typename T>
Salix::Property batch6947_draw_property(const std::string& name, Salix::PropertyType type, T& value) {
    return Salix::Property{
        name, type, nullptr,
        [&value](void*) -> void* { return &value; },
        [&value](void*, void* incoming) { value = *static_cast<T*>(incoming); }
    };
}
}

TEST_SUITE("Salix::reflection::TypeDrawerLiveBatch6947") {
    TEST_CASE("batch6947 registered integer drawer executes inside bare ImGui frame") {
        Batch6947DrawerFrame frame;
        int value = 6947;
        auto property = batch6947_draw_property("Int6947", Salix::PropertyType::Int, value);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Int)(property, nullptr));
        CHECK(value == 6947);
    }

    TEST_CASE("batch6947 registered float drawer executes inside bare ImGui frame") {
        Batch6947DrawerFrame frame;
        float value = 69.47f;
        auto property = batch6947_draw_property("Float6947", Salix::PropertyType::Float, value);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Float)(property, nullptr));
        CHECK(value == doctest::Approx(69.47f));
    }

    TEST_CASE("batch6947 registered bool drawer executes inside bare ImGui frame") {
        Batch6947DrawerFrame frame;
        bool value = true;
        auto property = batch6947_draw_property("Bool6947", Salix::PropertyType::Bool, value);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Bool)(property, nullptr));
        CHECK(value);
    }

    TEST_CASE("batch6947 registered Vector2 drawer executes inside bare ImGui frame") {
        Batch6947DrawerFrame frame;
        Salix::Vector2 value(6.0f, 947.0f);
        auto property = batch6947_draw_property("Vector26947", Salix::PropertyType::Vector2, value);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Vector2)(property, nullptr));
        CHECK(value == Salix::Vector2(6.0f, 947.0f));
    }

    TEST_CASE("batch6947 registered Vector3 drawer executes inside bare ImGui frame") {
        Batch6947DrawerFrame frame;
        Salix::Vector3 value(6.0f, 9.0f, 47.0f);
        auto property = batch6947_draw_property("Vector36947", Salix::PropertyType::Vector3, value);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Vector3)(property, nullptr));
        CHECK(value == Salix::Vector3(6.0f, 9.0f, 47.0f));
    }

    TEST_CASE("batch6947 registered color drawer executes inside bare ImGui frame") {
        Batch6947DrawerFrame frame;
        Salix::Color value(0.69f, 0.47f, 0.25f, 1.0f);
        auto property = batch6947_draw_property("Color6947", Salix::PropertyType::Color, value);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Color)(property, nullptr));
        CHECK(value == Salix::Color(0.69f, 0.47f, 0.25f, 1.0f));
    }

    TEST_CASE("batch6947 registered point drawer executes inside bare ImGui frame") {
        Batch6947DrawerFrame frame;
        Salix::Point value{69, 47};
        auto property = batch6947_draw_property("Point6947", Salix::PropertyType::Point, value);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Point)(property, nullptr));
        CHECK((value == Salix::Point{69, 47}));
    }

    TEST_CASE("batch6947 registered string drawer executes inside bare ImGui frame") {
        Batch6947DrawerFrame frame;
        std::string value = "Batch6947";
        auto property = batch6947_draw_property("String6947", Salix::PropertyType::String, value);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::String)(property, nullptr));
        CHECK(value == "Batch6947");
    }

    TEST_CASE("batch6947 registered matrix drawer executes all four matrix rows") {
        Batch6947DrawerFrame frame;
        glm::mat4 value(1.0f);
        auto property = batch6947_draw_property("Matrix6947", Salix::PropertyType::GlmMat4, value);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::GlmMat4)(property, nullptr));
        CHECK(value == glm::mat4(1.0f));
    }

    TEST_CASE("batch6947 nested class drawer recurses into contained integer property") {
        Batch6947DrawerFrame frame;
        int nested_value = 47;
        Salix::TypeInfo nested;
        nested.name = "Nested6947";
        nested.properties.push_back(batch6947_draw_property("NestedInt6947", Salix::PropertyType::Int, nested_value));
        Salix::Property outer{
            "Outer6947", Salix::PropertyType::Class, &nested,
            [](void*) -> void* { return nullptr; },
            [](void*, void*) {}
        };
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Class)(outer, nullptr));
        CHECK(nested_value == 47);
    }
}
