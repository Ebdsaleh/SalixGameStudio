#define IMGUI_DEFINE_MATH_OPERATORS
#include <doctest.h>
#include <Salix/reflection/ui/TypeDrawerLive.h>
#include <Salix/reflection/ByteMirror.h>
#include <Tests/SalixEngine/mocking/reflection/MockPropertyHandle.h>
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
#include <imgui_internal.h>
#include <string>

namespace {
struct Batch6947DrawerFrame {
    ImGuiContext* previous_test = ImGui::GetCurrentContext();
    ImGuiContext* previous_engine = Salix::ImGuiInputManager::get_context_for_testing();
    ImGuiContext* owned = ImGui::CreateContext();
    bool frame_open = false;

    Batch6947DrawerFrame() {
        ImGui::SetCurrentContext(owned);
        Salix::ImGuiInputManager::set_context_for_testing(owned);
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(800.0f, 600.0f);
        io.DeltaTime = 1.0f / 60.0f;
        if (io.Fonts->Fonts.empty()) io.Fonts->AddFontDefault();
        io.Fonts->Build();
        Salix::TypeDrawerLive::register_all_type_drawers();
        begin();
    }

    ~Batch6947DrawerFrame() {
        end();
        Salix::ImGuiInputManager::set_context_for_testing(previous_engine);
        ImGui::DestroyContext(owned);
        ImGui::SetCurrentContext(previous_test);
    }

    void begin() {
        ImGui::SetCurrentContext(owned);
        ImGui::NewFrame();
        ImGui::Begin("Batch6947DrawerWindow");
        frame_open = true;
    }

    void end() {
        if (!frame_open) {
            return;
        }
        ImGui::End();
        ImGui::EndFrame();
        frame_open = false;
    }

    ImGuiID item_id(const char* label) {
        return ImGui::GetID(label);
    }

    ImGuiID component_id(const char* label, int component) {
        ImGui::PushID(label);
        ImGui::PushID(component);
        const ImGuiID id = ImGui::GetID("");
        ImGui::PopID();
        ImGui::PopID();
        return id;
    }

    ImGuiID matrix_component_id(const char* property_name, int column, int component) {
        static const char* column_labels[] = {"Col 1", "Col 2", "Col 3", "Col 4"};
        ImGui::PushID(property_name);
        ImGui::PushID(column_labels[column]);
        ImGui::PushID(component);
        const ImGuiID id = ImGui::GetID("");
        ImGui::PopID();
        ImGui::PopID();
        ImGui::PopID();
        return id;
    }

    ImGuiID step_button_id(const char* label, const char* symbol) {
        ImGui::PushID(label);
        const ImGuiID id = ImGui::GetID(symbol);
        ImGui::PopID();
        return id;
    }

    template<typename Fn>
    void activate_once(ImGuiID id, Fn&& fn) {
        fn();
        end();
        ImGui::ActivateItemByID(id);
        begin();
        fn();
    }

    template<typename Fn>
    void activate_and_tweak(ImGuiID id, Fn&& fn, ImGuiKey key = ImGuiKey_RightArrow) {
        fn();
        end();

        ImGui::ActivateItemByID(id);
        begin();
        fn();
        end();

        ImGui::GetIO().AddKeyEvent(key, true);
        begin();
        fn();
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
#include <utility>

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
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("Int7047",h));
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

#include <Salix/reflection/EnumRegistry.h>
#include <Salix/ecs/Camera.h>

namespace {
const Salix::TypeInfo* batch7147_projection_type_info() {
    Salix::ByteMirror::register_all_types();
    Salix::EnumRegistry::register_all_enums();
    return Salix::ByteMirror::get_type_info(typeid(Salix::ProjectionMode));
}

Salix::Property batch7147_enum_property(
    const std::string& name,
    Salix::PropertyType type,
    int& value,
    const Salix::TypeInfo* contained_type_info) {
    Salix::Property property;
    property.name = name;
    property.type = type;
    property.contained_type_info = contained_type_info;
    property.get_data = [&value](void*) -> void* { return &value; };
    property.set_data = [&value](void*, void* incoming) {
        value = *static_cast<int*>(incoming);
    };
    return property;
}
}

TEST_SUITE("Salix::reflection::TypeDrawerLiveBatch7147") {
    TEST_CASE("batch7147 direct Enum drawer reaches registered enum metadata path") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = static_cast<int>(Salix::ProjectionMode::Perspective);
        auto property = batch7147_enum_property("Enum7147A", Salix::PropertyType::Enum, value, info);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Enum)(property, nullptr));
        CHECK(value == static_cast<int>(Salix::ProjectionMode::Perspective));
    }

    TEST_CASE("batch7147 direct EnumClass drawer reaches registered enum metadata path") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = static_cast<int>(Salix::ProjectionMode::Orthographic);
        auto property = batch7147_enum_property("EnumClass7147B", Salix::PropertyType::EnumClass, value, info);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::EnumClass)(property, nullptr));
        CHECK(value == static_cast<int>(Salix::ProjectionMode::Orthographic));
    }

    TEST_CASE("batch7147 direct Enum drawer safely skips metadata branch when contained type is null") {
        Batch6947DrawerFrame frame;
        int value = 0;
        auto property = batch7147_enum_property("Enum7147Null", Salix::PropertyType::Enum, value, nullptr);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Enum)(property, nullptr));
        CHECK(value == 0);
    }

    TEST_CASE("batch7147 direct EnumClass drawer safely skips metadata branch when contained type is null") {
        Batch6947DrawerFrame frame;
        int value = 1;
        auto property = batch7147_enum_property("EnumClass7147Null", Salix::PropertyType::EnumClass, value, nullptr);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::EnumClass)(property, nullptr));
        CHECK(value == 1);
    }

    TEST_CASE("batch7147 direct Enum drawer skips metadata branch when type index is absent") {
        Batch6947DrawerFrame frame;
        Salix::TypeInfo no_index;
        no_index.name = "Batch7147NoIndex";
        int value = 0;
        auto property = batch7147_enum_property("Enum7147NoIndex", Salix::PropertyType::Enum, value, &no_index);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Enum)(property, nullptr));
        CHECK(value == 0);
    }

    TEST_CASE("batch7147 direct EnumClass drawer skips metadata branch when type index is absent") {
        Batch6947DrawerFrame frame;
        Salix::TypeInfo no_index;
        no_index.name = "Batch7147NoIndexClass";
        int value = 1;
        auto property = batch7147_enum_property("EnumClass7147NoIndex", Salix::PropertyType::EnumClass, value, &no_index);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::EnumClass)(property, nullptr));
        CHECK(value == 1);
    }

    TEST_CASE("batch7147 direct Enum drawer accepts orthographic integer value without mutation") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = static_cast<int>(Salix::ProjectionMode::Orthographic);
        auto property = batch7147_enum_property("Enum7147Orthographic", Salix::PropertyType::Enum, value, info);
        Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Enum)(property, nullptr);
        CHECK(value == static_cast<int>(Salix::ProjectionMode::Orthographic));
    }

    TEST_CASE("batch7147 direct EnumClass drawer accepts perspective integer value without mutation") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = static_cast<int>(Salix::ProjectionMode::Perspective);
        auto property = batch7147_enum_property("EnumClass7147Perspective", Salix::PropertyType::EnumClass, value, info);
        Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::EnumClass)(property, nullptr);
        CHECK(value == static_cast<int>(Salix::ProjectionMode::Perspective));
    }

    TEST_CASE("batch7147 repeated direct Enum draws remain stable without interaction") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 0;
        auto property = batch7147_enum_property("Enum7147Repeat", Salix::PropertyType::Enum, value, info);
        auto drawer = Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Enum);
        drawer(property, nullptr);
        drawer(property, nullptr);
        CHECK(value == 0);
    }

    TEST_CASE("batch7147 repeated direct EnumClass draws remain stable without interaction") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 1;
        auto property = batch7147_enum_property("EnumClass7147Repeat", Salix::PropertyType::EnumClass, value, info);
        auto drawer = Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::EnumClass);
        drawer(property, nullptr);
        drawer(property, nullptr);
        CHECK(value == 1);
    }

    TEST_CASE("batch7147 PropertyHandle Enum draw reaches enum combo metadata path") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 0;
        auto property = batch7147_enum_property("HandleEnum7147A", Salix::PropertyType::Enum, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("HandleEnum7147A", handle));
        CHECK(std::get<int>(handle.get_value()) == 0);
    }

    TEST_CASE("batch7147 PropertyHandle EnumClass draw reaches enum combo metadata path") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 1;
        auto property = batch7147_enum_property("HandleEnumClass7147B", Salix::PropertyType::EnumClass, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("HandleEnumClass7147B", handle));
        CHECK(std::get<int>(handle.get_value()) == 1);
    }

    TEST_CASE("batch7147 PropertyHandle Enum draw preserves perspective value") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = static_cast<int>(Salix::ProjectionMode::Perspective);
        auto property = batch7147_enum_property("HandleEnum7147Perspective", Salix::PropertyType::Enum, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        Salix::TypeDrawerLive::draw_property("HandleEnum7147Perspective", handle);
        CHECK(value == static_cast<int>(Salix::ProjectionMode::Perspective));
    }

    TEST_CASE("batch7147 PropertyHandle EnumClass draw preserves orthographic value") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = static_cast<int>(Salix::ProjectionMode::Orthographic);
        auto property = batch7147_enum_property("HandleEnumClass7147Orthographic", Salix::PropertyType::EnumClass, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        Salix::TypeDrawerLive::draw_property("HandleEnumClass7147Orthographic", handle);
        CHECK(value == static_cast<int>(Salix::ProjectionMode::Orthographic));
    }

    TEST_CASE("batch7147 repeated PropertyHandle Enum draws remain stable") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 0;
        auto property = batch7147_enum_property("HandleEnum7147Repeat", Salix::PropertyType::Enum, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        Salix::TypeDrawerLive::draw_property("HandleEnum7147RepeatA", handle);
        Salix::TypeDrawerLive::draw_property("HandleEnum7147RepeatB", handle);
        CHECK(value == 0);
    }

    TEST_CASE("batch7147 repeated PropertyHandle EnumClass draws remain stable") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 1;
        auto property = batch7147_enum_property("HandleEnumClass7147Repeat", Salix::PropertyType::EnumClass, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        Salix::TypeDrawerLive::draw_property("HandleEnumClass7147RepeatA", handle);
        Salix::TypeDrawerLive::draw_property("HandleEnumClass7147RepeatB", handle);
        CHECK(value == 1);
    }

    TEST_CASE("batch7147 direct Enum drawer accepts hidden label ID") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 0;
        auto property = batch7147_enum_property("##Batch7147DirectEnumHidden", Salix::PropertyType::Enum, value, info);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Enum)(property, nullptr));
    }

    TEST_CASE("batch7147 direct EnumClass drawer accepts hidden label ID") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 1;
        auto property = batch7147_enum_property("##Batch7147DirectEnumClassHidden", Salix::PropertyType::EnumClass, value, info);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::EnumClass)(property, nullptr));
    }

    TEST_CASE("batch7147 PropertyHandle Enum draw accepts hidden display label ID") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 0;
        auto property = batch7147_enum_property("StoredEnum7147", Salix::PropertyType::Enum, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("##Batch7147HandleEnumHidden", handle));
    }

    TEST_CASE("batch7147 PropertyHandle EnumClass draw accepts hidden display label ID") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 1;
        auto property = batch7147_enum_property("StoredEnumClass7147", Salix::PropertyType::EnumClass, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        CHECK_NOTHROW(Salix::TypeDrawerLive::draw_property("##Batch7147HandleEnumClassHidden", handle));
    }

    TEST_CASE("batch7147 direct Enum drawer tolerates large out of range integer") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 7147;
        auto property = batch7147_enum_property("Enum7147Large", Salix::PropertyType::Enum, value, info);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Enum)(property, nullptr));
        CHECK(value == 7147);
    }

    TEST_CASE("batch7147 direct EnumClass drawer tolerates negative out of range integer") {
        Batch6947DrawerFrame frame;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = -7147;
        auto property = batch7147_enum_property("EnumClass7147Negative", Salix::PropertyType::EnumClass, value, info);
        CHECK_NOTHROW(Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::EnumClass)(property, nullptr));
        CHECK(value == -7147);
    }

    TEST_CASE("batch7147 PropertyHandle Enum exposes integer variant after draw") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 0;
        auto property = batch7147_enum_property("VariantEnum7147", Salix::PropertyType::Enum, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        Salix::TypeDrawerLive::draw_property("VariantEnum7147", handle);
        CHECK(std::holds_alternative<int>(handle.get_value()));
    }

    TEST_CASE("batch7147 PropertyHandle EnumClass exposes integer variant after draw") {
        Batch6947DrawerFrame frame;
        Batch7047DrawerElement element;
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        int value = 1;
        auto property = batch7147_enum_property("VariantEnumClass7147", Salix::PropertyType::EnumClass, value, info);
        Salix::PropertyHandleLive handle(property, &element);
        Salix::TypeDrawerLive::draw_property("VariantEnumClass7147", handle);
        CHECK(std::holds_alternative<int>(handle.get_value()));
    }

    TEST_CASE("batch7147 enum contained type info remains the registered ProjectionMode type") {
        const auto* info = batch7147_projection_type_info();
        REQUIRE(info != nullptr);
        REQUIRE(info->type_index.has_value());
        CHECK(*info->type_index == std::type_index(typeid(Salix::ProjectionMode)));
    }
}


namespace {
template<typename T>
Salix::Property batch7347_mutable_property(
    const std::string& name,
    Salix::PropertyType type,
    T& value,
    int& setter_calls) {
    return Salix::Property{
        name,
        type,
        nullptr,
        [&value](void*) -> void* { return &value; },
        [&value, &setter_calls](void*, void* incoming) {
            ++setter_calls;
            value = *static_cast<T*>(incoming);
        }
    };
}

Salix::Property batch7347_handle_property(const char* name, Salix::PropertyType type) {
    Salix::Property property;
    property.name = name;
    property.type = type;
    return property;
}
}

TEST_SUITE("Salix::reflection::TypeDrawerLiveBatch7347") {
    TEST_CASE("batch7347 direct integer drawer applies ImGui step button mutation through reflected setter") {
        Batch6947DrawerFrame frame;
        int value = 7347;
        int setter_calls = 0;
        auto property = batch7347_mutable_property("Int7347", Salix::PropertyType::Int, value, setter_calls);
        const ImGuiID plus_id = frame.step_button_id("Int7347", "+");

        frame.activate_once(plus_id, [&] {
            Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Int)(property, nullptr);
        });

        CHECK(value == 7348);
        CHECK(setter_calls == 1);
    }

    TEST_CASE("batch7347 direct boolean drawer propagates checkbox activation through reflected setter") {
        Batch6947DrawerFrame frame;
        bool value = true;
        int setter_calls = 0;
        auto property = batch7347_mutable_property("Bool7347", Salix::PropertyType::Bool, value, setter_calls);
        const ImGuiID id = frame.item_id("Bool7347");

        frame.activate_once(id, [&] {
            Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Bool)(property, nullptr);
        });

        CHECK_FALSE(value);
        CHECK(setter_calls == 1);
    }

    TEST_CASE("batch7347 direct Vector2 drawer propagates keyboard drag mutation through reflected setter") {
        Batch6947DrawerFrame frame;
        Salix::Vector2 value(7.0f, 347.0f);
        const Salix::Vector2 before = value;
        int setter_calls = 0;
        auto property = batch7347_mutable_property("Vector27347", Salix::PropertyType::Vector2, value, setter_calls);
        const ImGuiID id = frame.component_id("Vector27347", 0);

        frame.activate_and_tweak(id, [&] {
            Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Vector2)(property, nullptr);
        });

        CHECK_FALSE(value.x == doctest::Approx(before.x));
        CHECK(value.y == doctest::Approx(before.y));
        CHECK(setter_calls == 1);
    }

    TEST_CASE("batch7347 direct Vector3 drawer propagates selected component drag through reflected setter") {
        Batch6947DrawerFrame frame;
        Salix::Vector3 value(7.0f, 3.0f, 47.0f);
        const Salix::Vector3 before = value;
        int setter_calls = 0;
        auto property = batch7347_mutable_property("Vector37347", Salix::PropertyType::Vector3, value, setter_calls);
        const ImGuiID id = frame.component_id("Vector37347", 1);

        frame.activate_and_tweak(id, [&] {
            Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::Vector3)(property, nullptr);
        });

        CHECK(value.x == doctest::Approx(before.x));
        CHECK_FALSE(value.y == doctest::Approx(before.y));
        CHECK(value.z == doctest::Approx(before.z));
        CHECK(setter_calls == 1);
    }

    TEST_CASE("batch7347 direct matrix drawer writes keyboard drag mutation through reflected setter") {
        Batch6947DrawerFrame frame;
        glm::mat4 value(1.0f);
        const float before = value[0][0];
        int setter_calls = 0;
        auto property = batch7347_mutable_property("Matrix7347", Salix::PropertyType::GlmMat4, value, setter_calls);
        const ImGuiID id = frame.matrix_component_id("Matrix7347", 0, 0);

        frame.activate_and_tweak(id, [&] {
            Salix::TypeDrawerLive::get_drawer(Salix::PropertyType::GlmMat4)(property, nullptr);
        });

        CHECK_FALSE(value[0][0] == doctest::Approx(before));
        CHECK(setter_calls == 1);
    }

    TEST_CASE("batch7347 handle integer branch commits keyboard drag mutation through existing reflection mock") {
        Batch6947DrawerFrame frame;
        auto property = batch7347_handle_property("HandleInt7347", Salix::PropertyType::Int);
        MockPropertyHandle handle(property, Salix::PropertyValue{7347});
        const ImGuiID id = frame.item_id("HandleInt7347");

        frame.activate_and_tweak(id, [&] {
            Salix::TypeDrawerLive::draw_property("HandleInt7347", handle);
        });

        CHECK(handle.set_calls == 1);
        CHECK(std::get<int>(handle.get_value()) != 7347);
    }

    TEST_CASE("batch7347 handle float branch commits keyboard drag mutation through existing reflection mock") {
        Batch6947DrawerFrame frame;
        auto property = batch7347_handle_property("HandleFloat7347", Salix::PropertyType::Float);
        MockPropertyHandle handle(property, Salix::PropertyValue{73.47f});
        const ImGuiID id = frame.item_id("HandleFloat7347");

        frame.activate_and_tweak(id, [&] {
            Salix::TypeDrawerLive::draw_property("HandleFloat7347", handle);
        });

        CHECK(handle.set_calls == 1);
        CHECK_FALSE(std::get<float>(handle.get_value()) == doctest::Approx(73.47f));
    }

    TEST_CASE("batch7347 handle boolean branch commits checkbox mutation through existing reflection mock") {
        Batch6947DrawerFrame frame;
        auto property = batch7347_handle_property("HandleBool7347", Salix::PropertyType::Bool);
        MockPropertyHandle handle(property, Salix::PropertyValue{true});
        const ImGuiID id = frame.item_id("HandleBool7347");

        frame.activate_once(id, [&] {
            Salix::TypeDrawerLive::draw_property("HandleBool7347", handle);
        });

        CHECK(handle.set_calls == 1);
        CHECK_FALSE(std::get<bool>(handle.get_value()));
    }

    TEST_CASE("batch7347 handle Vector2 branch commits one component drag through existing reflection mock") {
        Batch6947DrawerFrame frame;
        auto property = batch7347_handle_property("HandleVector27347", Salix::PropertyType::Vector2);
        MockPropertyHandle handle(property, Salix::PropertyValue{Salix::Vector2(7.0f, 347.0f)});
        const ImGuiID id = frame.component_id("HandleVector27347", 0);

        frame.activate_and_tweak(id, [&] {
            Salix::TypeDrawerLive::draw_property("HandleVector27347", handle);
        });

        const auto result = std::get<Salix::Vector2>(handle.get_value());
        CHECK(handle.set_calls == 1);
        CHECK_FALSE(result.x == doctest::Approx(7.0f));
        CHECK(result.y == doctest::Approx(347.0f));
    }

    TEST_CASE("batch7347 handle Vector3 branch commits selected component drag through existing reflection mock") {
        Batch6947DrawerFrame frame;
        auto property = batch7347_handle_property("HandleVector37347", Salix::PropertyType::Vector3);
        MockPropertyHandle handle(property, Salix::PropertyValue{Salix::Vector3(7.0f, 3.0f, 47.0f)});
        const ImGuiID id = frame.component_id("HandleVector37347", 2);

        frame.activate_and_tweak(id, [&] {
            Salix::TypeDrawerLive::draw_property("HandleVector37347", handle);
        });

        const auto result = std::get<Salix::Vector3>(handle.get_value());
        CHECK(handle.set_calls == 1);
        CHECK(result.x == doctest::Approx(7.0f));
        CHECK(result.y == doctest::Approx(3.0f));
        CHECK_FALSE(result.z == doctest::Approx(47.0f));
    }
}
