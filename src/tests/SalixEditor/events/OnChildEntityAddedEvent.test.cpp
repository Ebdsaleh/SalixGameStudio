// Tests for Editor/events/OnChildEntityAddedEvent.h.
#include "../fixtures/EditorEventFixtures.h"
#include <Editor/events/OnChildEntityAddedEvent.h>

TEST_SUITE("EditorCRI013_OnChildEntityAddedEvent") {
    using namespace EditorCri;
    using Capture=EditorEventCapture<OnChildEntityAddedEvent>;
    using Fixture=EditorEventFixture<OnChildEntityAddedEvent>;
    TEST_CASE("CRI013 OnChildEntityAddedEvent: child event preserves explicit parent identity") {
        auto a=entity(12,"Child");a.parent_id=id(11);OnChildEntityAddedEvent e(a);CHECK(e.archetype.id==id(12));CHECK(e.archetype.parent_id==id(11));CHECK(e.archetype.name=="Child");
    }

    TEST_CASE("CRI013 OnChildEntityAddedEvent: child payload retains nested component metadata") {
        auto a=entity(12);a.parent_id=id(11);auto t=element("Transform",21,"{position: {x: 2,y: 3,z: 4}}");t.owner_id=a.id;a.elements.push_back(t);OnChildEntityAddedEvent e(a);REQUIRE(e.archetype.elements.size()==1);CHECK(e.archetype.elements[0].owner_id==id(12));CHECK(e.archetype.elements[0].data["position"]["z"].as<int>()==4);
    }

    TEST_CASE("CRI013 OnChildEntityAddedEvent: child payload retains unsaved state and visibility") {
        auto a=entity(12);a.state=ArchetypeState::New;a.is_visible=false;a.display_order=7;OnChildEntityAddedEvent e(a);CHECK(e.archetype.state==ArchetypeState::New);CHECK_FALSE(e.archetype.is_visible);CHECK(e.archetype.display_order==7);
    }

    TEST_CASE("CRI013 OnChildEntityAddedEvent: clone preserves child identity and event metadata") {
        auto a=entity(12);a.parent_id=id(11);OnChildEntityAddedEvent e(a);e.handled=true;auto c=e.clone();auto* p=dynamic_cast<OnChildEntityAddedEvent*>(c.get());REQUIRE(p);CHECK(p->handled);CHECK(p->archetype.id==id(12));CHECK(p->archetype.parent_id==id(11));CHECK(p->get_event_type()==EventType::EditorOnChildEntityAdded);
    }

    TEST_CASE("CRI013 OnChildEntityAddedEvent: clone owns independent vector membership") {
        auto a=entity(12);a.elements.push_back(element("Transform",21));OnChildEntityAddedEvent e(a);auto c=e.clone();auto* p=dynamic_cast<OnChildEntityAddedEvent*>(c.get());REQUIRE(p);p->archetype.elements.clear();CHECK(e.archetype.elements.size()==1);
    }

    TEST_CASE("CRI013 OnChildEntityAddedEvent: queued clone retains original name and parent after source edit") {
        Fixture f;auto a=entity(12,"Before");a.parent_id=id(11);OnChildEntityAddedEvent e(a);f.context.events.dispatch(e);e.archetype.name="After";e.archetype.parent_id=id(99);f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).archetype.name=="Before");CHECK(f.capture.at(0).archetype.parent_id==id(11));
    }

    TEST_CASE("CRI013 OnChildEntityAddedEvent: manager child addition emits correct model record") {
        Fixture f;f.root(11);f.drain();f.child(12,11);f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).archetype.id==id(12));CHECK(f.capture.at(0).archetype.parent_id==id(11));REQUIRE(f.manager().get_archetype(id(11))->child_ids.size()==1);CHECK(f.manager().get_archetype(id(11))->child_ids[0]==id(12));
    }

    TEST_CASE("CRI013 OnChildEntityAddedEvent: missing parent prevents child notification") {
        Fixture f;f.root(11);f.drain();f.child(12,99);f.drain();CHECK(f.capture.count()==0);CHECK(f.manager().get_archetype(id(12))==nullptr);
    }

    TEST_CASE("CRI013 OnChildEntityAddedEvent: successive child events retain individual parent links") {
        Fixture f;f.root(11);f.child(12,11);f.child(13,12);f.drain();REQUIRE(f.capture.count()==2);CHECK(f.capture.at(0).archetype.id==id(12));CHECK(f.capture.at(0).archetype.parent_id==id(11));CHECK(f.capture.at(1).archetype.parent_id==id(12));
    }

    TEST_CASE("CRI013 OnChildEntityAddedEvent: child event routes only to Editor category") {
        OnChildEntityAddedEvent e(entity(12));CHECK(std::string(e.get_name())=="EditorOnChildEntityAdded");CHECK(e.is_in_category(EventCategory::Editor));CHECK_FALSE(e.is_input_event());Capture c;EventManager m;m.subscribe(EventCategory::Input,&c);m.dispatch(e);m.process_queue();CHECK(c.count()==0);
    }
}
