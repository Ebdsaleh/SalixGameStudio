// Tests for Editor/events/OnRootEntityAddedEvent.h.
#include "../fixtures/EditorEventFixtures.h"
#include <Editor/events/OnRootEntityAddedEvent.h>
#include <Editor/panels/RealmDesignerPanel.h>

TEST_SUITE("EditorCRI014_OnRootEntityAddedEvent") {
    using namespace EditorCri;
    using Capture=EditorEventCapture<OnRootEntityAddedEvent>;
    using Fixture=EditorEventFixture<OnRootEntityAddedEvent>;
    TEST_CASE("CRI014 OnRootEntityAddedEvent: root payload preserves authored identity name and invalid parent") {
        auto a=entity(11,"Root");OnRootEntityAddedEvent e(a);CHECK(e.archetype.id==id(11));CHECK(e.archetype.name=="Root");CHECK_FALSE(e.archetype.parent_id.is_valid());
    }

    TEST_CASE("CRI014 OnRootEntityAddedEvent: root payload preserves visibility order and unsaved status") {
        auto a=entity(11);a.is_visible=false;a.display_order=9;a.state=ArchetypeState::New;OnRootEntityAddedEvent e(a);CHECK_FALSE(e.archetype.is_visible);CHECK(e.archetype.display_order==9);CHECK(e.archetype.state==ArchetypeState::New);
    }

    TEST_CASE("CRI014 OnRootEntityAddedEvent: root payload carries reflected component properties") {
        auto a=entity(11);auto t=element("Transform",21,"{position: {x: 3,y: 4,z: 5}}");t.owner_id=a.id;a.elements.push_back(t);OnRootEntityAddedEvent e(a);REQUIRE(e.archetype.elements.size()==1);CHECK(e.archetype.elements[0].owner_id==id(11));CHECK(e.archetype.elements[0].data["position"]["x"].as<int>()==3);
    }

    TEST_CASE("CRI014 OnRootEntityAddedEvent: clone retains root metadata and handled status") {
        OnRootEntityAddedEvent e(entity(11,"Root"));e.handled=true;auto copy=e.clone();auto* p=dynamic_cast<OnRootEntityAddedEvent*>(copy.get());REQUIRE(p);CHECK(p->archetype.id==id(11));CHECK(p->archetype.name=="Root");CHECK(p->handled);CHECK(p->get_event_type()==EventType::EditorOnRootEntityAdded);
    }

    TEST_CASE("CRI014 OnRootEntityAddedEvent: cloned root vector structure independent from source") {
        auto a=entity(11);a.elements.push_back(element("Transform",21));OnRootEntityAddedEvent e(a);auto copy=e.clone();auto* p=dynamic_cast<OnRootEntityAddedEvent*>(copy.get());REQUIRE(p);p->archetype.elements.clear();CHECK(e.archetype.elements.size()==1);
    }

    TEST_CASE("CRI014 OnRootEntityAddedEvent: queued root payload survives source event destruction") {
        Fixture f;{OnRootEntityAddedEvent e(entity(11,"Created"));f.context.events.dispatch(e);}f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).archetype.name=="Created");CHECK(f.capture.at(0).archetype.id==id(11));
    }

    TEST_CASE("CRI014 OnRootEntityAddedEvent: manager addition produces actual root notification") {
        Fixture f;f.root(11);REQUIRE(f.manager().get_archetype(id(11)));f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).archetype.id==id(11));CHECK(f.capture.at(0).archetype.name=="Root");
    }

    TEST_CASE("CRI014 OnRootEntityAddedEvent: child insertion does not emit another root notification") {
        Fixture f;f.root(11);f.drain();f.child(12,11);f.drain();CHECK(f.capture.count()==1);CHECK(f.manager().get_archetype(id(12))->parent_id==id(11));
    }

    TEST_CASE("CRI014 OnRootEntityAddedEvent: actual queued root event creates corresponding preview entity") {
        PanelContext c;RealmDesignerPanel p;p.initialize(&c.ctx);auto a=entity(11,"Live root");c.ctx.editor_realm_manager->add_entity(a);CHECK(c.ctx.preview_realm->get_entity_by_id(id(11))==nullptr);c.events.process_queue();auto* live=c.ctx.preview_realm->get_entity_by_id(id(11));REQUIRE(live);CHECK(live->get_name()=="Live root");CHECK(live->is_root());
    }

    TEST_CASE("CRI014 OnRootEntityAddedEvent: root addition metadata excludes input subscribers") {
        OnRootEntityAddedEvent e(entity(11));CHECK(std::string(e.get_name())=="EditorOnRootEntityAdded");CHECK(e.is_in_category(EventCategory::Editor));CHECK_FALSE(e.is_input_event());Capture c;EventManager m;m.subscribe(EventCategory::Input,&c);m.dispatch(e);m.process_queue();CHECK(c.count()==0);
    }
}
