// Tests for Editor/events/OnEntityFamilyAddedEvent.h.
#include "../fixtures/ArchetypeFixtures.h"
#include <Editor/events/OnEntityFamilyAddedEvent.h>
#include <Salix/events/EventManager.h>
#include <Salix/events/IEventListener.h>



TEST_SUITE("EditorCRI006_OnEntityFamilyAddedEvent") {
    using namespace EditorCri;
    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: empty family payload remains empty") {
        OnEntityFamilyAddedEvent e({}); CHECK(e.archetypes.empty());
    }

    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: family preserves parent and child ordering") {
        auto a=entity(11,"Parent"); auto b=entity(12,"Child"); a.child_ids={b.id}; b.parent_id=a.id; OnEntityFamilyAddedEvent e({a,b}); REQUIRE(e.archetypes.size()==2); CHECK(e.archetypes[0].child_ids[0]==id(12)); CHECK(e.archetypes[1].parent_id==id(11));
    }

    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: copy of input vector survives source clear") {
        std::vector<EntityArchetype> source{entity(11,"Parent"),entity(12,"Child")}; OnEntityFamilyAddedEvent e(source); source.clear(); REQUIRE(e.archetypes.size()==2); CHECK(e.archetypes[1].name=="Child");
    }

    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: payload keeps component IDs and owner") {
        auto a=entity(11); auto b=element("Transform",12); b.owner_id=a.id; a.elements.push_back(b); OnEntityFamilyAddedEvent e({a}); REQUIRE(e.archetypes.size()==1); REQUIRE(e.archetypes[0].elements.size()==1); CHECK(e.archetypes[0].elements[0].id==id(12)); CHECK(e.archetypes[0].elements[0].owner_id==id(11));
    }

    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: family state and visibility survive event construction") {
        auto a=entity(11); a.state=ArchetypeState::New; a.is_visible=false; OnEntityFamilyAddedEvent e({a}); REQUIRE(e.archetypes.size()==1); CHECK(e.archetypes[0].state==ArchetypeState::New); CHECK_FALSE(e.archetypes[0].is_visible);
    }

    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: clone preserves family data and handled state") {
        OnEntityFamilyAddedEvent e({entity(11,"First"),entity(12,"Second")}); e.handled=true; auto copy=e.clone(); auto* p=dynamic_cast<OnEntityFamilyAddedEvent*>(copy.get()); REQUIRE(p); REQUIRE(p->archetypes.size()==2); CHECK(p->archetypes[0].id==id(11)); CHECK(p->archetypes[1].name=="Second"); CHECK(p->handled);
    }

    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: clone vector structure is independent") {
        OnEntityFamilyAddedEvent e({entity(11,"Original")}); auto copy=e.clone(); auto* p=dynamic_cast<OnEntityFamilyAddedEvent*>(copy.get()); REQUIRE(p); p->archetypes[0].name="Edited"; p->archetypes.push_back(entity(12)); REQUIRE(e.archetypes.size()==1); CHECK(e.archetypes[0].name=="Original");
    }

    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: family addition is editor not input event") {
        OnEntityFamilyAddedEvent e({}); CHECK(e.get_event_type()==EventType::EditorOnEntityFamilyAdded); CHECK(std::string(e.get_name())=="EditorOnEntityFamilyAdded"); CHECK(e.is_in_category(EventCategory::Editor)); CHECK_FALSE(e.is_input_event());
    }

    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: queued family survives destruction of source event") {
        struct Listener:IEventListener{std::vector<SimpleGuid> ids;void on_event(IEvent& e)override{for(auto& a:static_cast<OnEntityFamilyAddedEvent&>(e).archetypes)ids.push_back(a.id);}} listener; EventManager manager; manager.subscribe(EventCategory::Editor,&listener); {OnEntityFamilyAddedEvent e({entity(11),entity(12)});manager.dispatch(e);} manager.process_queue(); REQUIRE(listener.ids.size()==2); CHECK(listener.ids[0]==id(11)); CHECK(listener.ids[1]==id(12));
    }

    TEST_CASE("CRI006 OnEntityFamilyAddedEvent: event dispatch to unrelated category does not deliver family") {
        struct Listener:IEventListener{int calls=0;void on_event(IEvent&)override{++calls;}} listener; EventManager manager; manager.subscribe(EventCategory::Input,&listener); OnEntityFamilyAddedEvent e({entity(11)}); manager.dispatch(e); manager.process_queue(); CHECK(listener.calls==0); CHECK(manager.is_queue_empty());
    }
}
