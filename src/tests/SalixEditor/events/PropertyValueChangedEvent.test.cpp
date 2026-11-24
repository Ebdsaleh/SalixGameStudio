// Tests for Editor/events/PropertyValueChangedEvent.h.
#include "../fixtures/ArchetypeFixtures.h"
#include <Editor/events/PropertyValueChangedEvent.h>
#include <Salix/events/EventManager.h>
#include <Salix/events/IEventListener.h>



TEST_SUITE("EditorCRI006_PropertyValueChangedEvent") {
    using namespace EditorCri;
    TEST_CASE("CRI006 PropertyValueChangedEvent: constructor preserves entity and element identities") {
        PropertyValueChangedEvent e(id(11),id(12),"Transform","name",std::string("Local")); CHECK(e.entity_id==id(11)); CHECK(e.element_id==id(12)); CHECK(e.element_type_name=="Transform"); CHECK(e.property_name=="name");
    }

    TEST_CASE("CRI006 PropertyValueChangedEvent: reload defaults false for ordinary scalar update") {
        PropertyValueChangedEvent e(id(11),id(12),"Camera","active",true); CHECK_FALSE(e.requires_reload); CHECK(std::get<bool>(e.new_value));
    }

    TEST_CASE("CRI006 PropertyValueChangedEvent: reload flag retained for asset update") {
        PropertyValueChangedEvent e(id(11),id(12),"Sprite2D","texture_path",std::string("Assets/a.png"),true); CHECK(e.requires_reload); CHECK(std::get<std::string>(e.new_value)=="Assets/a.png");
    }

    TEST_CASE("CRI006 PropertyValueChangedEvent: vector payload retains typed coordinates") {
        PropertyValueChangedEvent e(id(11),id(12),"Transform","position",Vector3(2,3,4)); CHECK(std::get<Vector3>(e.new_value)==Vector3(2,3,4));
    }

    TEST_CASE("CRI006 PropertyValueChangedEvent: large unsigned payload not narrowed") {
        uint64_t value=0xfedcba9876543210ULL; PropertyValueChangedEvent e(id(11),id(12),"Custom","id",value); CHECK(std::get<uint64_t>(e.new_value)==value);
    }

    TEST_CASE("CRI006 PropertyValueChangedEvent: payload copied independently from caller") {
        PropertyValue value=std::string("Original"); PropertyValueChangedEvent e(id(11),id(12),"Transform","name",value); value=std::string("Changed"); CHECK(std::get<std::string>(e.new_value)=="Original");
    }

    TEST_CASE("CRI006 PropertyValueChangedEvent: clone preserves metadata and reload flag") {
        PropertyValueChangedEvent e(id(11),id(12),"Sprite2D","texture_path",std::string("a.png"),true); e.handled=true; auto copy=e.clone(); auto* p=dynamic_cast<PropertyValueChangedEvent*>(copy.get()); REQUIRE(p); CHECK(p->entity_id==e.entity_id); CHECK(p->element_id==e.element_id); CHECK(p->property_name==e.property_name); CHECK(p->element_type_name==e.element_type_name); CHECK(p->new_value==e.new_value); CHECK(p->requires_reload); CHECK(p->handled);
    }

    TEST_CASE("CRI006 PropertyValueChangedEvent: clone payload can change independently") {
        PropertyValueChangedEvent e(id(11),id(12),"Camera","active",true); auto copy=e.clone(); auto* p=dynamic_cast<PropertyValueChangedEvent*>(copy.get()); REQUIRE(p); p->new_value=false; CHECK(std::get<bool>(e.new_value)); CHECK_FALSE(std::get<bool>(p->new_value));
    }

    TEST_CASE("CRI006 PropertyValueChangedEvent: editor event metadata excludes input categories") {
        PropertyValueChangedEvent e(id(11),id(12),"Transform","name",std::string("Local")); CHECK(e.get_event_type()==EventType::EditorPropertyValueChanged); CHECK(std::string(e.get_name())=="EditorPropertyValueChanged"); CHECK(e.is_in_category(EventCategory::Editor)); CHECK_FALSE(e.is_input_event());
    }

    TEST_CASE("CRI006 PropertyValueChangedEvent: queued property update retains copied value after source changes") {
        struct Listener:IEventListener{int calls=0;std::string value;void on_event(IEvent& e)override{++calls;value=std::get<std::string>(static_cast<PropertyValueChangedEvent&>(e).new_value);}} listener; EventManager manager; manager.subscribe(EventCategory::Editor,&listener); PropertyValueChangedEvent e(id(11),id(12),"Transform","name",std::string("Queued")); manager.dispatch(e); e.new_value=std::string("Edited"); manager.process_queue(); CHECK(listener.calls==1); CHECK(listener.value=="Queued");
    }
}
