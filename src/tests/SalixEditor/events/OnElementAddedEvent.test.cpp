// Tests for Editor/events/OnElementAddedEvent.h.
#include "../fixtures/EditorEventFixtures.h"
#include <Editor/events/OnElementAddedEvent.h>

TEST_SUITE("EditorCRI013_OnElementAddedEvent") {
    using namespace EditorCri;
    using Capture=EditorEventCapture<OnElementAddedEvent>;
    using Fixture=EditorEventFixture<OnElementAddedEvent>;
    TEST_CASE("CRI013 OnElementAddedEvent: event retains authored element and owner IDs") {
        auto a=element("Camera",21);a.owner_id=id(11);OnElementAddedEvent e(id(11),a);CHECK(e.parent_entity_id==id(11));CHECK(e.element_archetype.id==id(21));CHECK(e.element_archetype.owner_id==id(11));
    }

    TEST_CASE("CRI013 OnElementAddedEvent: payload preserves structured property data") {
        auto a=element("Transform",21,"{position: {x: 2,y: 3,z: 4}}");OnElementAddedEvent e(id(11),a);CHECK(e.element_archetype.data["position"]["y"].as<int>()==3);CHECK(e.element_archetype.type_name=="Transform");
    }

    TEST_CASE("CRI013 OnElementAddedEvent: event preserves unsaved visibility and duplication metadata") {
        auto a=element();a.state=ArchetypeState::New;a.is_visible=false;a.allows_duplication=false;a.display_order=9;OnElementAddedEvent e(id(11),a);CHECK(e.element_archetype.state==ArchetypeState::New);CHECK_FALSE(e.element_archetype.is_visible);CHECK_FALSE(e.element_archetype.allows_duplication);CHECK(e.element_archetype.display_order==9);
    }

    TEST_CASE("CRI013 OnElementAddedEvent: clone retains event type and handled metadata") {
        OnElementAddedEvent e(id(11),element("Camera",21));e.handled=true;auto c=e.clone();auto* p=dynamic_cast<OnElementAddedEvent*>(c.get());REQUIRE(p);CHECK(p->handled);CHECK(p->parent_entity_id==id(11));CHECK(p->element_archetype.id==id(21));CHECK(p->get_event_type()==EventType::EditorOnElementAdded);
    }

    TEST_CASE("CRI013 OnElementAddedEvent: clone metadata changes do not rename original payload") {
        OnElementAddedEvent e(id(11),element("Camera",21));auto c=e.clone();auto* p=dynamic_cast<OnElementAddedEvent*>(c.get());REQUIRE(p);p->element_archetype.name="Other";p->parent_entity_id=id(12);CHECK(e.element_archetype.name=="Camera");CHECK(e.parent_entity_id==id(11));
    }

    TEST_CASE("CRI013 OnElementAddedEvent: queued element survives source event destruction") {
        Fixture f;{OnElementAddedEvent e(id(11),element("Sprite2D",21,"{texture_path: 'a.png'}"));f.context.events.dispatch(e);}f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).element_archetype.data["texture_path"].as<std::string>()=="a.png");
    }

    TEST_CASE("CRI013 OnElementAddedEvent: manager emits element addition only after actual insertion") {
        Fixture f;f.root(11);f.drain();auto a=element("Camera",21);a.owner_id=id(11);f.manager().add_element_to_entity(id(11),a);REQUIRE(f.manager().get_archetype(id(11))->get_element_by_id(id(21)));f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).parent_entity_id==id(11));CHECK(f.capture.at(0).element_archetype.id==id(21));
    }

    TEST_CASE("CRI013 OnElementAddedEvent: manager missing owner does not emit element addition") {
        Fixture f;f.root(11);f.drain();f.manager().add_element_to_entity(id(99),element("Camera",21));f.drain();CHECK(f.capture.count()==0);CHECK(f.manager().get_archetype(id(11))->elements.empty());
    }

    TEST_CASE("CRI013 OnElementAddedEvent: multiple additions preserve owner and component ordering") {
        Fixture f;f.root(11);f.root(12);f.drain();f.manager().add_element_to_entity(id(11),element("Camera",21));f.manager().add_element_to_entity(id(12),element("Sprite2D",22));f.drain();REQUIRE(f.capture.count()==2);CHECK(f.capture.at(0).parent_entity_id==id(11));CHECK(f.capture.at(0).element_archetype.id==id(21));CHECK(f.capture.at(1).parent_entity_id==id(12));CHECK(f.capture.at(1).element_archetype.id==id(22));
    }

    TEST_CASE("CRI013 OnElementAddedEvent: element notification excludes mouse category") {
        OnElementAddedEvent e(id(11),element());CHECK(std::string(e.get_name())=="EditorOnElementAdded");CHECK(e.is_in_category(EventCategory::Editor));CHECK_FALSE(e.is_mouse_event());Capture c;EventManager m;m.subscribe(EventCategory::Mouse,&c);m.dispatch(e);m.process_queue();CHECK(c.count()==0);
    }
}
