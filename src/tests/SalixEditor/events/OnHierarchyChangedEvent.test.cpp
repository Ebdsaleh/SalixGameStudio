// Tests for Editor/events/OnHierarchyChangedEvent.h.
#include "../fixtures/EditorEventFixtures.h"
#include <Editor/events/OnHierarchyChangedEvent.h>

TEST_SUITE("EditorCRI013_OnHierarchyChangedEvent") {
    using namespace EditorCri;
    using Capture=EditorEventCapture<OnHierarchyChangedEvent>;
    using Fixture=EditorEventFixture<OnHierarchyChangedEvent>;
    TEST_CASE("CRI013 OnHierarchyChangedEvent: parent change preserves full-width stable identities") {
        OnHierarchyChangedEvent e(id(0x100000001ULL),id(0x200000002ULL));CHECK(e.entity_id==id(0x100000001ULL));CHECK(e.parent_id==id(0x200000002ULL));
    }

    TEST_CASE("CRI013 OnHierarchyChangedEvent: orphan notification represents root without a parent") {
        OnHierarchyChangedEvent e(id(11),SimpleGuid::invalid());CHECK(e.entity_id==id(11));CHECK_FALSE(e.parent_id.is_valid());
    }

    TEST_CASE("CRI013 OnHierarchyChangedEvent: metadata routes hierarchy changes to Editor listeners") {
        OnHierarchyChangedEvent e(id(11),id(12));CHECK(e.get_event_type()==EventType::EditorOnHierarchyChanged);CHECK(std::string(e.get_name())=="EditorOnHierarchyChanged");CHECK(e.is_in_category(EventCategory::Editor));CHECK_FALSE(e.is_input_event());
    }

    TEST_CASE("CRI013 OnHierarchyChangedEvent: clone preserves identity and handled status") {
        OnHierarchyChangedEvent e(id(11),id(12));e.handled=true;auto c=e.clone();auto* h=dynamic_cast<OnHierarchyChangedEvent*>(c.get());REQUIRE(h);CHECK(h->entity_id==id(11));CHECK(h->parent_id==id(12));CHECK(h->handled);
    }

    TEST_CASE("CRI013 OnHierarchyChangedEvent: queued clone retains prior parent when source is edited") {
        Fixture f;OnHierarchyChangedEvent e(id(11),id(12));f.context.events.dispatch(e);e.parent_id=id(13);f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).parent_id==id(12));
    }

    TEST_CASE("CRI013 OnHierarchyChangedEvent: queued parent changes preserve order including release") {
        Fixture f;f.context.events.dispatch(OnHierarchyChangedEvent(id(11),id(12)));f.context.events.dispatch(OnHierarchyChangedEvent(id(11),id(13)));f.context.events.dispatch(OnHierarchyChangedEvent(id(11),SimpleGuid::invalid()));f.drain();REQUIRE(f.capture.count()==3);CHECK(f.capture.at(0).parent_id==id(12));CHECK(f.capture.at(1).parent_id==id(13));CHECK_FALSE(f.capture.at(2).parent_id.is_valid());
    }

    TEST_CASE("CRI013 OnHierarchyChangedEvent: release manager emits actual orphan notification") {
        Fixture f;f.root(11);f.child(12,11);f.drain();f.manager().release_from_parent(id(12));f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).entity_id==id(12));CHECK_FALSE(f.capture.at(0).parent_id.is_valid());CHECK_FALSE(f.manager().get_archetype(id(12))->parent_id.is_valid());
    }

    TEST_CASE("CRI013 OnHierarchyChangedEvent: release notification leaves descendants attached to released child") {
        Fixture f;f.root(11);f.child(12,11);f.child(13,12);f.drain();f.manager().release_from_parent(id(12));f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).entity_id==id(12));CHECK(f.manager().get_archetype(id(13))->parent_id==id(12));
    }

    TEST_CASE("CRI013 OnHierarchyChangedEvent: application subscriber does not receive hierarchy event") {
        Capture c;EventManager m;m.subscribe(EventCategory::Application,&c);m.dispatch(OnHierarchyChangedEvent(id(11),id(12)));m.process_queue();CHECK(c.count()==0);CHECK(m.is_queue_empty());
    }

    TEST_CASE("CRI013 OnHierarchyChangedEvent: handled hierarchy notification stops subsequent subscriber") {
        Capture first,second;EventManager m;first.consume=true;m.subscribe(EventCategory::Editor,&first);m.subscribe(EventCategory::Editor,&second);m.dispatch(OnHierarchyChangedEvent(id(11),id(12)));m.process_queue();CHECK(first.count()==1);CHECK(second.count()==0);
    }
}
