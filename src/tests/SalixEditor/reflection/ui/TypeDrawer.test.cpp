// Tests for Editor/reflection/ui/TypeDrawer.
#include "../../fixtures/HeadlessGuiFixtures.h"
#include <Editor/reflection/ui/TypeDrawer.h>
#include <Salix/reflection/PropertyHandle.h>
namespace {
using namespace EditorCri;
struct HandleProbe:PropertyHandle{
    PropertyValue value; int reads=0,writes=0;
    HandleProbe(Property p,PropertyValue v):PropertyHandle(p),value(std::move(v)){}
    PropertyValue get_value()const override {++const_cast<HandleProbe*>(this)->reads;return value;}
    void set_value(const PropertyValue& v)override{++writes;value=v;}
};
Property prop(PropertyType t,const char* n="Value"){Property p{};p.name=n;p.type=t;return p;}
void idle_draw(Property p,PropertyValue v){GuiFrame g;EditorContext c;HandleProbe h(p,v);bool changed=true;int verts=g.draw([&]{changed=TypeDrawer::draw_property(p.name.c_str(),h,&c);});CHECK(verts>0);CHECK_FALSE(changed);CHECK(h.reads>0);CHECK(h.writes==0);CHECK(h.value==v);}
void yaml_idle(PropertyType type,const char* yaml){GuiFrame g;auto n=YAML::Load(yaml);auto before=YAML::Dump(n);CHECK(g.draw([&]{TypeDrawer::draw_yaml_property(prop(type),n);})>0);CHECK(YAML::Dump(n)==before);}
}
TEST_SUITE("EditorCRI002_TypeDrawer") {
    TEST_CASE("CRI002 TypeDrawer: integer and read-only integer draw without mutation") {
        idle_draw(prop(PropertyType::Int),7); auto p=prop(PropertyType::Int);p.is_read_only=true;idle_draw(p,12);
    }

    TEST_CASE("CRI002 TypeDrawer: float draw keeps typed value") {
        idle_draw(prop(PropertyType::Float),2.5f);
    }

    TEST_CASE("CRI002 TypeDrawer: boolean draw keeps typed value") {
        idle_draw(prop(PropertyType::Bool),true);
    }

    TEST_CASE("CRI002 TypeDrawer: string modes preserve value without input") {
        idle_draw(prop(PropertyType::String),std::string("hello"));auto p=prop(PropertyType::String);p.hint=UIHint::MultilineText;idle_draw(p,std::string("line one\\nline two"));
    }

    TEST_CASE("CRI002 TypeDrawer: vector two and vector three typed editors") {
        idle_draw(prop(PropertyType::Vector2),Vector2(1,2));idle_draw(prop(PropertyType::Vector3),Vector3(1,2,3));
    }

    TEST_CASE("CRI002 TypeDrawer: color typed editor preserves RGBA") {
        idle_draw(prop(PropertyType::Color),Color(.2f,.3f,.4f,.5f));
    }

    TEST_CASE("CRI002 TypeDrawer: YAML scalar editors preserve properties") {
        yaml_idle(PropertyType::Int,"{Value: 7}");yaml_idle(PropertyType::Float,"{Value: 2.5}");yaml_idle(PropertyType::Bool,"{Value: true}");
    }

    TEST_CASE("CRI002 TypeDrawer: YAML string editor preserves input including long value") {
        GuiFrame g; auto n=YAML::Load("{}");n["Value"]=std::string(300,'x');auto before=YAML::Dump(n);CHECK(g.draw([&]{TypeDrawer::draw_yaml_property(prop(PropertyType::String),n);})>0);CHECK(YAML::Dump(n)==before);
    }

    TEST_CASE("CRI002 TypeDrawer: YAML vector and color editors preserve structured data") {
        yaml_idle(PropertyType::Vector2,"{Value: {x: 1, y: 2}}");yaml_idle(PropertyType::Vector3,"{Value: {x: 1, y: 2, z: 3}}");yaml_idle(PropertyType::Color,"{Value: {r: 0.1, g: 0.2, b: 0.3, a: 1}}");
    }

    TEST_CASE("CRI002 TypeDrawer: unsupported YAML type and registered drawer lookup") {
        GuiFrame g;auto n=YAML::Load("{Value: 3}");auto before=YAML::Dump(n);CHECK(g.draw([&]{TypeDrawer::draw_yaml_property(prop(PropertyType::Rect),n);})>0);CHECK(YAML::Dump(n)==before);TypeDrawer::register_all_type_drawers();CHECK(static_cast<bool>(TypeDrawer::get_drawer(PropertyType::Int)));CHECK(static_cast<bool>(TypeDrawer::get_drawer(PropertyType::Vector3)));CHECK(static_cast<bool>(TypeDrawer::get_drawer(PropertyType::Class)));
    }
}

#include <Salix/reflection/EnumRegistry.h>
#include <Salix/ecs/Camera.h>
namespace {
template<class T> void draw_registered(PropertyType type, T value) {
    GuiFrame g; TypeDrawer::register_all_type_drawers();
    const T original=value; int reads=0,writes=0;
    auto p=prop(type); p.get_data=[&](void*)->void*{++reads;return &value;};
    p.set_data=[&](void*,void* v){++writes;value=*static_cast<T*>(v);};
    CHECK(g.draw([&]{TypeDrawer::get_drawer(type)(p,nullptr);})>0);
    CHECK(reads==1); CHECK(writes==0); CHECK(value==original);
}
}
TEST_SUITE("EditorCRI006_TypeDrawer") {
    using namespace EditorCri;
    TEST_CASE("CRI006 TypeDrawer: registered scalar drawers read without editing") {
        draw_registered(PropertyType::Int,9); draw_registered(PropertyType::Float,3.25f); draw_registered(PropertyType::Bool,true);
    }

    TEST_CASE("CRI006 TypeDrawer: registered vector drawers preserve live coordinates") {
        draw_registered(PropertyType::Vector2,Vector2(3,4)); draw_registered(PropertyType::Vector3,Vector3(1,2,3));
    }

    TEST_CASE("CRI006 TypeDrawer: registered color drawer preserves live channels") {
        draw_registered(PropertyType::Color,Color(.1f,.2f,.3f,.4f));
    }

    TEST_CASE("CRI006 TypeDrawer: registered string drawer safely displays long live value") {
        draw_registered(PropertyType::String,std::string(600,'x'));
    }

    TEST_CASE("CRI006 TypeDrawer: registered matrix drawer reads four columns") {
        draw_registered(PropertyType::GlmMat4,glm::mat4(2.0f));
    }

    TEST_CASE("CRI006 TypeDrawer: plain enum drawer keeps registered projection selection") {
        EnumRegistry::register_all_enums(); TypeInfo info{}; info.type_index=typeid(ProjectionMode); auto p=prop(PropertyType::Enum); p.contained_type_info=&info; idle_draw(p,static_cast<int>(ProjectionMode::Orthographic));
    }

    TEST_CASE("CRI006 TypeDrawer: enum class drawer keeps registered selection") {
        EnumRegistry::register_all_enums(); TypeInfo info{}; info.type_index=typeid(ProjectionMode); auto p=prop(PropertyType::EnumClass); p.contained_type_info=&info; idle_draw(p,static_cast<int>(ProjectionMode::Perspective));
    }

    TEST_CASE("CRI006 TypeDrawer: missing enum metadata does not dereference null") {
        idle_draw(prop(PropertyType::Enum),0); idle_draw(prop(PropertyType::EnumClass),0);
    }

    TEST_CASE("CRI006 TypeDrawer: enum type without type index remains read-only fallback") {
        TypeInfo info{}; auto p=prop(PropertyType::EnumClass); p.contained_type_info=&info; idle_draw(p,1);
    }

    TEST_CASE("CRI006 TypeDrawer: all file hints preserve paths without opening a dialog") {
        for(auto hint:{UIHint::ImageFile,UIHint::AudioFile,UIHint::TextFile,UIHint::SourceFile,UIHint::FilePath}){auto p=prop(PropertyType::String);p.hint=hint;idle_draw(p,std::string("Assets/fixture.txt"));}
    }
}
