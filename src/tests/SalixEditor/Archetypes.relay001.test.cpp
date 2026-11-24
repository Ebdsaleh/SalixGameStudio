// CRI relay 001: Archetypes; actual Editor DLL APIs, no native windows.
#include "CriRelay001Fixtures.h"

namespace {
using namespace EditorCri;

}
TEST_SUITE("EditorCRI001_Archetypes") {
    TEST_CASE("CRI001 Archetypes: empty entities compare equal") {
        auto a=entity(); auto b=entity(); CHECK_FALSE(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: child order is significant") {
        auto a=entity(); auto b=entity(); a.child_ids={id(7),id(8)}; b.child_ids={id(8),id(7)}; CHECK(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: lookup distinguishes IDs sharing names") {
        auto a=entity(); a.elements={element(),element("Transform",20002)}; CHECK(a.get_element_by_id(id(20002))==&a.elements[1]);
    }

    TEST_CASE("CRI001 Archetypes: primary transform ignores colliders") {
        auto a=entity(); a.elements={element("BoxCollider"),element("Transform",20002)}; a.elements[0].allows_duplication=false; CHECK(a.get_primary_transform_id()==id(20002));
    }

    TEST_CASE("CRI001 Archetypes: empty property maps compare equal") {
        auto a=element("Transform",1,"{}"); auto b=element("Transform",1,"{}"); CHECK_FALSE(a.data_is_different(b));
    }

    TEST_CASE("CRI001 Archetypes: empty versus populated properties compare different") {
        auto a=element("Transform",1,"{}"); auto b=element(); CHECK(a.data_is_different(b)); CHECK(b.data_is_different(a));
    }

    TEST_CASE("CRI001 Archetypes: nested map size changes are detected") {
        auto a=element("Transform",1,"{p: {x: 1}}"); auto b=element("Transform",1,"{p: {x: 1, y: 2}}"); CHECK(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: nested sequence length changes are detected") {
        auto a=element("Transform",1,"{p: [1,2]}"); auto b=element("Transform",1,"{p: [1]}"); CHECK(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: nested YAML node type changes are detected") {
        auto a=element("Transform",1,"{p: [1]}"); auto b=element("Transform",1,"{p: 1}"); CHECK(a.is_different_from(b));
    }

    TEST_CASE("CRI001 Archetypes: owner changes propagate through entity comparison") {
        auto a=entity(); auto b=entity(); a.elements.push_back(element()); b.elements.push_back(element()); b.elements[0].owner_id=id(900); CHECK(a.is_different_from(b));
    }
}
