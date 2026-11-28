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

namespace {
using namespace EditorCri;
EntityArchetype authored_family_member() {
    auto a=entity(11,"Family");
    auto t=element("Transform",21,"{position: {x: 3, y: 4, z: 5}, extra: [one, two]}");
    t.owner_id=a.id;
    a.elements.push_back(t);
    return a;
}
struct FamilyPayloadListener : IEventListener {
    int calls=0;
    int x=0;
    void on_event(IEvent& event) override {
        REQUIRE(event.get_event_type()==EventType::EditorOnEntityFamilyAdded);
        REQUIRE(std::string(event.get_name())=="EditorOnEntityFamilyAdded");
        auto& family=static_cast<OnEntityFamilyAddedEvent&>(event);
        REQUIRE(family.archetypes.size()==1);
        REQUIRE(family.archetypes[0].elements.size()==1);
        x=family.archetypes[0].elements[0].data["position"]["x"].as<int>();
        ++calls;
    }
};
}
TEST_SUITE("EditorCRI031_OnEntityFamilyAddedEvent") {
    using namespace EditorCri;
    TEST_CASE("CRI031 OnEntityFamilyAddedEvent: construction detaches nested YAML from authored family") {
        auto a=authored_family_member();
        OnEntityFamilyAddedEvent event({a});
        a.elements[0].data["position"]["x"]=99;
        a.elements[0].data["extra"].push_back("three");
        CHECK(event.archetypes[0].elements[0].data["position"]["x"].as<int>()==3);
        CHECK(event.archetypes[0].elements[0].data["extra"].size()==2);
        event.archetypes[0].elements[0].data["position"]["y"]=77;
        CHECK(a.elements[0].data["position"]["y"].as<int>()==4);
    }
    TEST_CASE("CRI031 OnEntityFamilyAddedEvent: clone owns independent nested YAML and handled state") {
        OnEntityFamilyAddedEvent event({authored_family_member()});
        event.handled=true;
        auto clone=event.clone();
        auto* copied=dynamic_cast<OnEntityFamilyAddedEvent*>(clone.get());
        REQUIRE(copied);
        CHECK(copied->handled);
        copied->archetypes[0].elements[0].data["position"]["x"]=88;
        CHECK(event.archetypes[0].elements[0].data["position"]["x"].as<int>()==3);
        event.archetypes[0].elements[0].data["extra"].push_back("three");
        CHECK(copied->archetypes[0].elements[0].data["extra"].size()==2);
    }
    TEST_CASE("CRI031 OnEntityFamilyAddedEvent: copy assignment detaches payload and preserves handled state") {
        OnEntityFamilyAddedEvent original({authored_family_member()});
        original.handled=true;
        OnEntityFamilyAddedEvent assigned(std::vector<EntityArchetype>{});
        assigned=original;
        REQUIRE(assigned.archetypes.size()==1);
        CHECK(assigned.handled);
        assigned.archetypes[0].elements[0].data["position"]["x"]=42;
        CHECK(original.archetypes[0].elements[0].data["position"]["x"].as<int>()==3);
        const auto before=YAML::Dump(assigned.archetypes[0].elements[0].data);
        auto& same=assigned;
        assigned=same;
        CHECK(YAML::Dump(assigned.archetypes[0].elements[0].data)==before);
        CHECK(assigned.handled);
    }
    TEST_CASE("CRI031 OnEntityFamilyAddedEvent: queued clone retains authored values despite later source edits") {
        FamilyPayloadListener listener;
        EventManager manager;
        manager.subscribe(EventCategory::Editor,&listener);
        auto a=authored_family_member();
        OnEntityFamilyAddedEvent event({a});
        manager.dispatch(event);
        a.elements[0].data["position"]["x"]=80;
        event.archetypes[0].elements[0].data["position"]["x"]=90;
        manager.process_queue();
        CHECK(listener.calls==1);
        CHECK(listener.x==3);
        CHECK(manager.is_queue_empty());
    }
    TEST_CASE("CRI031 OnEntityFamilyAddedEvent: queued owned event survives mutation of originating realm data") {
        FamilyPayloadListener listener;
        EventManager manager;
        manager.subscribe(EventCategory::Editor,&listener);
        auto a=authored_family_member();
        manager.dispatch(std::make_unique<OnEntityFamilyAddedEvent>(std::vector<EntityArchetype>{a}));
        a.elements[0].data["position"]["x"]=100;
        manager.process_queue();
        CHECK(listener.calls==1);
        CHECK(listener.x==3);
        CHECK(manager.is_queue_empty());
    }
}
