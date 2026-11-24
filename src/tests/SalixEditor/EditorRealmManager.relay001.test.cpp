// CRI relay 001: EditorRealmManager; actual Editor DLL APIs, no native windows.
#include "CriRelay001Fixtures.h"
#include <Editor/management/EditorRealmManager.h>
#include <Editor/EditorContext.h>
#include <Salix/events/EventManager.h>
namespace {
using namespace EditorCri;
void load_pair(EditorRealmManager& m, TempRealm& f) {
    m.load_realm_from_file(f.write("Entities: [{name: Root, id: 11}, {name: Child, id: 12, parent: 11}]"));
}
}
TEST_SUITE("EditorCRI001_EditorRealmManager") {
    TEST_CASE("CRI001 EditorRealmManager: default manager starts dirty with no snapshot") {
        EditorRealmManager m; CHECK(m.realm_is_empty()); CHECK(m.realm_map_is_empty()); CHECK(m.get_snapshot()==nullptr); CHECK(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: context constructor has empty clean snapshot") {
        EditorContext c; EditorRealmManager m(&c); REQUIRE(m.get_snapshot()); CHECK_FALSE(m.is_dirty()); CHECK(m.get_realm_size()==0);
    }

    TEST_CASE("CRI001 EditorRealmManager: load establishes hierarchy and clean snapshot") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); REQUIRE(m.get_realm_size()==2); REQUIRE(m.get_hierarchy().size()==1); auto root=m.get_hierarchy()[0]; CHECK(root->entity_id==id(11)); REQUIRE(root->children.size()==1); CHECK(root->children[0]->entity_id==id(12)); CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: lookup works for const and mutable manager") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); const auto& cm=m; REQUIRE(m.get_archetype(id(12))); CHECK(cm.get_archetype(id(12))==m.get_archetype(id(12))); CHECK(cm.get_archetype(id(99))==nullptr); CHECK(m.get_archetype(id(99))==nullptr);
    }

    TEST_CASE("CRI001 EditorRealmManager: all archetypes and name search reflect loaded data") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); auto all=m.get_all_archetypes(); REQUIRE(all.size()==2); CHECK(all[0]->name=="Root"); CHECK(all[1]->name=="Child"); CHECK(m.does_entity_name_exist("Root")); CHECK_FALSE(m.does_entity_name_exist("root"));
    }

    TEST_CASE("CRI001 EditorRealmManager: rename makes dirty until snapshot refreshed") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); REQUIRE(m.get_archetype(id(11))); m.get_archetype(id(11))->name="Renamed"; CHECK(m.is_dirty()); m.take_snapshot(); CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: clear resets realm map hierarchy and snapshot") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); m.clear_realm(); CHECK(m.realm_is_empty()); CHECK(m.realm_map_is_empty()); CHECK(m.get_hierarchy().empty()); REQUIRE(m.get_snapshot()); CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: missing entity purge does not alter existing realm") {
        TempRealm f; EditorRealmManager m; load_pair(m,f); m.purge_entity(id(99)); m.purge_entity_descendants(id(99)); CHECK(m.get_realm_size()==2); CHECK_FALSE(m.is_dirty());
    }

    TEST_CASE("CRI001 EditorRealmManager: root addition queues notification and updates lookup") {
        EventManager events; EditorContext c; c.event_manager=&events; EditorRealmManager m(&c); m.add_entity(entity(11,"Root")); REQUIRE(m.get_archetype(id(11))); CHECK(m.get_realm_size()==1); CHECK(m.is_dirty()); CHECK_FALSE(events.is_queue_empty());
    }

    TEST_CASE("CRI001 EditorRealmManager: root purge updates lookup and queues notification") {
        EventManager events; EditorContext c; c.event_manager=&events; EditorRealmManager m(&c); m.add_entity(entity(11,"Root")); events.process_queue(); m.purge_entity(id(11)); CHECK(m.realm_is_empty()); CHECK(m.realm_map_is_empty()); CHECK(m.get_archetype(id(11))==nullptr); CHECK_FALSE(events.is_queue_empty());
    }
}
