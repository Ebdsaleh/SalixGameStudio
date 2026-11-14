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

#include <Salix/reflection/PropertyHandleLive.h>
#include <Salix/ecs/Element.h>
#include <Salix/math/Rect.h>
#include <cstdint>

namespace {
class Batch7047DrawerElement : public Salix::Element {
public:
    int integer = 7047;
    std::uint64_t identifier = 7047;
    float scalar = 70.47f;
    bool flag = true;
    std::string text = "Batch7047";
    Salix::Vector2 vector2{7.0f, 47.0f};
    Salix::Vector3 vector3{7.0f, 0.0f, 47.0f};
    Salix::Color color{255.0f, 128.0f, 64.0f, 255.0f};
    Salix::Point point{70, 47};
    Salix::Rect rect{7, 0, 4, 7};
    glm::mat4 matrix{1.0f};
    const char* get_class_name() const override { return "Batch7047DrawerElement"; }
};

Salix::Property batch7047_drawer_property(
    const char* name, Salix::PropertyType type,
    Salix::getter_func getter, Salix::setter_func setter = {}) {
    Salix::Property property;
    property.name = name;
    property.type = type;
    property.get_data = std::move(getter);
    property.set_data = std::move(setter);
    return property;
}
}

TEST_SUITE("Salix::reflection::TypeDrawerLiveBatch7047") {
    TEST_CASE("batch7047 property handle integer branch executes in bare ImGui frame") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Int7047",Salix::PropertyType::Int,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->integer;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Int7047",&h));
        CHECK(e.integer==7047);
    }

    TEST_CASE("batch7047 property handle float branch executes in bare ImGui frame") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Float7047",Salix::PropertyType::Float,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->scalar;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Float7047",h));
        CHECK(e.scalar==doctest::Approx(70.47f));
    }

    TEST_CASE("batch7047 property handle bool branch executes in bare ImGui frame") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Bool7047",Salix::PropertyType::Bool,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->flag;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Bool7047",h));
        CHECK(e.flag);
    }

    TEST_CASE("batch7047 property handle string branch executes in bare ImGui frame") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("String7047",Salix::PropertyType::String,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->text;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("String7047",h));
        CHECK(e.text=="Batch7047");
    }

    TEST_CASE("batch7047 property handle Vector2 branch executes in bare ImGui frame") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Vector27047",Salix::PropertyType::Vector2,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->vector2;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Vector27047",h));
        CHECK(e.vector2==Salix::Vector2(7.0f,47.0f));
    }

    TEST_CASE("batch7047 property handle Vector3 branch executes in bare ImGui frame") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Vector37047",Salix::PropertyType::Vector3,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->vector3;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Vector37047",h));
        CHECK(e.vector3==Salix::Vector3(7.0f,0.0f,47.0f));
    }

    TEST_CASE("batch7047 property handle color branch executes in bare ImGui frame") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Color7047",Salix::PropertyType::Color,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->color;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Color7047",h));
        CHECK(e.color==Salix::Color(255.0f,128.0f,64.0f,255.0f));
    }

    TEST_CASE("batch7047 property handle matrix branch executes read only display") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Matrix7047",Salix::PropertyType::GlmMat4,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->matrix;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Matrix7047",h));
        CHECK(e.matrix==glm::mat4(1.0f));
    }

    TEST_CASE("batch7047 property handle UInt64 reaches unsupported UI fallback safely") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("UInt7047",Salix::PropertyType::UInt64,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->identifier;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("UInt7047",h));
        CHECK(e.identifier==7047u);
    }

    TEST_CASE("batch7047 property handle Point reaches unsupported UI fallback safely") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Point7047",Salix::PropertyType::Point,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->point;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Point7047",h));
        CHECK((e.point==Salix::Point{70,47}));
    }

    TEST_CASE("batch7047 property handle Rect reaches unsupported UI fallback safely") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Rect7047",Salix::PropertyType::Rect,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->rect;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Rect7047",h));
        CHECK(e.rect==Salix::Rect(7,0,4,7));
    }

    TEST_CASE("batch7047 property handle Class reaches generic unimplemented display") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Class7047",Salix::PropertyType::Class,
            [](void*)->void*{return nullptr;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Class7047",h));
    }

    TEST_CASE("batch7047 property handle Struct reaches generic unimplemented display") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("Struct7047",Salix::PropertyType::Struct,
            [](void*)->void*{return nullptr;});
        Salix::PropertyHandleLive h(p,&e);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Struct7047",h));
    }

    TEST_CASE("batch7047 repeated integer draw remains non mutating without user input") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("RepeatInt7047",Salix::PropertyType::Int,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->integer;});
        Salix::PropertyHandleLive h(p,&e);
        Salix::TypeDrawerLive::draw_property("RepeatInt7047A",h);
        Salix::TypeDrawerLive::draw_property("RepeatInt7047B",h);
        CHECK(e.integer==7047);
    }

    TEST_CASE("batch7047 repeated vector draw remains non mutating without user input") {
        Batch6947DrawerFrame frame; Batch7047DrawerElement e;
        auto p=batch7047_drawer_property("RepeatVector7047",Salix::PropertyType::Vector3,
            [](void* x){return &static_cast<Batch7047DrawerElement*>(x)->vector3;});
        Salix::PropertyHandleLive h(p,&e);
        Salix::TypeDrawerLive::draw_property("RepeatVector7047A",h);
        Salix::TypeDrawerLive::draw_property("RepeatVector7047B",h);
        CHECK(e.vector3==Salix::Vector3(7.0f,0.0f,47.0f));
    }
}
