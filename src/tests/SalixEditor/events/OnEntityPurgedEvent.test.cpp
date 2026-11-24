// Tests for Editor/events/OnEntityPurgedEvent.h.
#include "../fixtures/EditorEventFixtures.h"
#include <Editor/events/OnEntityPurgedEvent.h>
#include <Editor/events/OnHierarchyChangedEvent.h>
#include <Editor/panels/RealmDesignerPanel.h>

TEST_SUITE("EditorCRI014_OnEntityPurgedEvent") {
    using namespace EditorCri;
    using Capture=EditorEventCapture<OnEntityPurgedEvent>;
    using Fixture=EditorEventFixture<OnEntityPurgedEvent>;
    TEST_CASE("CRI014 OnEntityPurgedEvent: purge notification preserves full-width identity") {
        OnEntityPurgedEvent e(id(0x100000001ULL));CHECK(e.entity_id==id(0x100000001ULL));
    }

    TEST_CASE("CRI014 OnEntityPurgedEvent: purge metadata identifies Editor event") {
        OnEntityPurgedEvent e(id(11));CHECK(e.get_event_type()==EventType::EditorOnEntityPurged);CHECK(std::string(e.get_name())=="EditorOnEntityPurged");CHECK(e.is_in_category(EventCategory::Editor));CHECK_FALSE(e.is_input_event());
    }

    TEST_CASE("CRI014 OnEntityPurgedEvent: clone preserves removed ID and handled state") {
        OnEntityPurgedEvent e(id(11));e.handled=true;auto clone=e.clone();auto* p=dynamic_cast<OnEntityPurgedEvent*>(clone.get());REQUIRE(p);CHECK(p->entity_id==id(11));CHECK(p->handled);p->entity_id=id(12);CHECK(e.entity_id==id(11));
    }

    TEST_CASE("CRI014 OnEntityPurgedEvent: queued purge survives destruction of source event") {
        Fixture f;{OnEntityPurgedEvent e(id(11));f.context.events.dispatch(e);}f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).entity_id==id(11));
    }

    TEST_CASE("CRI014 OnEntityPurgedEvent: manager emits purge only after entity removed") {
        Fixture f;f.root(11);f.root(12);f.drain();f.manager().purge_entity(id(11));CHECK(f.manager().get_archetype(id(11))==nullptr);f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).entity_id==id(11));CHECK(f.manager().get_archetype(id(12))!=nullptr);
    }

    TEST_CASE("CRI014 OnEntityPurgedEvent: manager absent identity emits no purge notification") {
        Fixture f;f.root(11);f.drain();f.manager().purge_entity(id(99));f.drain();CHECK(f.capture.count()==0);CHECK(f.manager().get_realm_size()==1);
    }

    TEST_CASE("CRI014 OnEntityPurgedEvent: repeated purge emits notification only for first removal") {
        Fixture f;f.root(11);f.drain();f.manager().purge_entity(id(11));f.manager().purge_entity(id(11));f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).entity_id==id(11));
    }

    TEST_CASE("CRI014 OnEntityPurgedEvent: parent purge precedes orphan hierarchy notifications") {
        Fixture f;f.root(11);f.child(12,11);f.child(13,11);f.drain();struct Order:IEventListener{std::vector<EventType> types;void on_event(IEvent& e)override{types.push_back(e.get_event_type());}} order;f.context.events.subscribe(EventCategory::Editor,&order);f.manager().purge_entity(id(11));f.drain();REQUIRE(order.types.size()==3);CHECK(order.types[0]==EventType::EditorOnEntityPurged);CHECK(order.types[1]==EventType::EditorOnHierarchyChanged);CHECK(order.types[2]==EventType::EditorOnHierarchyChanged);CHECK_FALSE(f.manager().get_archetype(id(12))->parent_id.is_valid());
    }

    TEST_CASE("CRI014 OnEntityPurgedEvent: preview consumes purge and preserves unrelated live entity") {
        PanelContext c;RealmDesignerPanel p;p.initialize(&c.ctx);auto* a=c.ctx.preview_realm->create_entity(id(11),"Removed");auto* b=c.ctx.preview_realm->create_entity(id(12),"Kept");OnEntityPurgedEvent e(id(11));p.on_event(e);CHECK(a->is_purged());CHECK_FALSE(b->is_purged());
    }

    TEST_CASE("CRI014 OnEntityPurgedEvent: purge event excludes application subscriber") {
        Capture c;EventManager m;m.subscribe(EventCategory::Application,&c);m.dispatch(OnEntityPurgedEvent(id(11)));m.process_queue();CHECK(c.count()==0);CHECK(m.is_queue_empty());
    }
}
