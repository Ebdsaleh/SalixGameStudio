// Tests for Editor/reflection/PropertyHandleFactory.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../fixtures/ArchetypeFixtures.h"
#include <Editor/reflection/PropertyHandleFactory.h>

namespace {
using namespace EditorCri;
using Handles=std::vector<std::unique_ptr<PropertyHandle>>;
PropertyHandle* find(Handles& h,const std::string& name) { for(auto& v:h) if(v->get_name()==name) return v.get(); return nullptr; }
Handles handles(ElementArchetype& e) { register_types(); return PropertyHandleFactory::create_handles_for_element_archetype(&e); }
}
TEST_SUITE("EditorCRI001_PropertyHandleFactory") {
    TEST_CASE("CRI001 PropertyHandleFactory: null entity yields no handles") {
        CHECK(PropertyHandleFactory::create_handles_for_archetype(nullptr).empty());
    }

    TEST_CASE("CRI001 PropertyHandleFactory: null element yields no handles") {
        CHECK(PropertyHandleFactory::create_handles_for_element_archetype(nullptr).empty());
    }

    TEST_CASE("CRI001 PropertyHandleFactory: unknown type ignored even with property YAML") {
        auto e=element("CRI001_Missing"); CHECK(handles(e).empty());
    }

    TEST_CASE("CRI001 PropertyHandleFactory: registered type with empty map yields no handles") {
        auto e=element("Transform",20001,"{}"); CHECK(handles(e).empty());
    }

    TEST_CASE("CRI001 PropertyHandleFactory: only reflected present properties exposed") {
        auto e=element("Transform",20001,"{name: T, imaginary: 5}"); auto h=handles(e); REQUIRE(h.size()==1); CHECK(h[0]->get_name()=="name"); CHECK(h[0]->get_type()==PropertyType::String);
    }

    TEST_CASE("CRI001 PropertyHandleFactory: string handle reads and writes its YAML node") {
        auto e=element(); auto h=handles(e); auto* p=find(h,"name"); REQUIRE(p); CHECK(std::get<std::string>(p->get_value())=="Transform"); p->set_value(PropertyValue{std::string("Changed")}); CHECK(e.data["name"].as<std::string>()=="Changed");
    }

    TEST_CASE("CRI001 PropertyHandleFactory: vector handle exposes typed value") {
        auto e=element("Transform",20001,"{position: {x: 1,y: 2,z: 3}}"); auto h=handles(e); auto* p=find(h,"position"); REQUIRE(p); CHECK(p->get_type()==PropertyType::Vector3); auto v=std::get<Vector3>(p->get_value()); CHECK(v.x==1); CHECK(v.y==2); CHECK(v.z==3);
    }

    TEST_CASE("CRI001 PropertyHandleFactory: bool handle mutation preserves unrelated fields") {
        auto e=element("Transform",20001,"{visible: true, name: Local}"); auto h=handles(e); auto* p=find(h,"visible"); REQUIRE(p); CHECK(std::get<bool>(p->get_value())); p->set_value(PropertyValue{false}); CHECK_FALSE(e.data["visible"].as<bool>()); CHECK(e.data["name"].as<std::string>()=="Local");
    }

    TEST_CASE("CRI001 PropertyHandleFactory: entity handles skip unknown elements and include both known nodes") {
        register_types(); auto a=entity(); a.elements={element("Transform",20001,"{name: First}"),element("CRI001_Missing",20002,"{name: Unknown}"),element("Transform",20003,"{name: Second}")}; auto h=PropertyHandleFactory::create_handles_for_archetype(&a); REQUIRE(h.size()==2); CHECK(std::get<std::string>(h[0]->get_value())=="First"); CHECK(std::get<std::string>(h[1]->get_value())=="Second"); h[1]->set_value(PropertyValue{std::string("Edit")}); CHECK(a.elements[2].data["name"].as<std::string>()=="Edit"); CHECK(a.elements[0].data["name"].as<std::string>()=="First");
    }

    TEST_CASE("CRI001 PropertyHandleFactory: entity with no elements returns no handles") {
        auto a=entity(); CHECK(PropertyHandleFactory::create_handles_for_archetype(&a).empty());
    }
}
