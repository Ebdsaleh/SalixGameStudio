// Tests for Editor/events/OnMainCameraChangedEvent.h.
#include "../fixtures/EditorEventFixtures.h"
#include <Editor/events/OnMainCameraChangedEvent.h>
#include <Editor/panels/RealmPortalPanel.h>
#include <Salix/ecs/Camera.h>
namespace {
using namespace EditorCri;
struct MainCameraEventFixture {
    PanelContext c; Realm realm; RealmPortalPanel portal;
    MainCameraEventFixture(){c.ctx.active_realm=&realm;portal.initialize(&c.ctx);}
    Camera* add(uint64_t n){auto* camera=realm.create_entity(id(n),"Camera")->add_element<Camera>();REQUIRE(camera);return camera;}
    void select(uint64_t n){c.events.dispatch(OnMainCameraChangedEvent(id(n)));c.events.process_queue();}
};
}
TEST_SUITE("EditorCRI014_OnMainCameraChangedEvent") {
    using namespace EditorCri;
    using Capture=EditorEventCapture<OnMainCameraChangedEvent>;
    using Fixture=EditorEventFixture<OnMainCameraChangedEvent>;
    TEST_CASE("CRI014 OnMainCameraChangedEvent: camera notification preserves full-width owner ID") {
        OnMainCameraChangedEvent e(id(0x100000001ULL));CHECK(e.entity_id==id(0x100000001ULL));
    }

    TEST_CASE("CRI014 OnMainCameraChangedEvent: invalid camera ID represents deselection") {
        OnMainCameraChangedEvent e(SimpleGuid::invalid());CHECK_FALSE(e.entity_id.is_valid());
    }

    TEST_CASE("CRI014 OnMainCameraChangedEvent: metadata identifies Editor camera change") {
        OnMainCameraChangedEvent e(id(11));CHECK(e.get_event_type()==EventType::EditorOnMainCameraChanged);CHECK(std::string(e.get_name())=="EditorOnMainCameraChanged");CHECK(e.is_in_category(EventCategory::Editor));CHECK_FALSE(e.is_input_event());
    }

    TEST_CASE("CRI014 OnMainCameraChangedEvent: clone retains camera identity and handled status") {
        OnMainCameraChangedEvent e(id(11));e.handled=true;auto c=e.clone();auto* p=dynamic_cast<OnMainCameraChangedEvent*>(c.get());REQUIRE(p);CHECK(p->entity_id==id(11));CHECK(p->handled);p->entity_id=id(12);CHECK(e.entity_id==id(11));
    }

    TEST_CASE("CRI014 OnMainCameraChangedEvent: queued clone retains original camera after caller changes event") {
        Fixture f;OnMainCameraChangedEvent e(id(11));f.context.events.dispatch(e);e.entity_id=id(12);f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).entity_id==id(11));
    }

    TEST_CASE("CRI014 OnMainCameraChangedEvent: queued portal camera selection activates correct live camera") {
        MainCameraEventFixture f;auto* a=f.add(11);f.select(11);CHECK(a->get_is_active());CHECK(f.realm.get_main_camera_entity_id()==id(11));CHECK(f.portal.get_game_camera_for_testing()==a);
    }

    TEST_CASE("CRI014 OnMainCameraChangedEvent: queued switch activates second camera and deactivates first") {
        MainCameraEventFixture f;auto* a=f.add(11);auto* b=f.add(12);f.select(11);f.select(12);CHECK_FALSE(a->get_is_active());CHECK(b->get_is_active());CHECK(f.portal.get_game_camera_for_testing()==b);
    }

    TEST_CASE("CRI014 OnMainCameraChangedEvent: queued deselection clears active pointer and identity") {
        MainCameraEventFixture f;auto* a=f.add(11);f.select(11);f.select(0);CHECK_FALSE(a->get_is_active());CHECK_FALSE(f.realm.get_main_camera_entity_id().is_valid());CHECK(f.portal.get_game_camera_for_testing()==nullptr);
    }

    TEST_CASE("CRI014 OnMainCameraChangedEvent: camera change without active realm leaves previous selection intact") {
        MainCameraEventFixture f;auto* a=f.add(11);f.select(11);f.c.ctx.active_realm=nullptr;f.select(99);CHECK(f.portal.get_game_camera_for_testing()==a);CHECK(f.realm.get_main_camera_entity_id()==id(11));
    }

    TEST_CASE("CRI014 OnMainCameraChangedEvent: camera event dispatch excludes keyboard category") {
        Capture c;EventManager m;m.subscribe(EventCategory::Keyboard,&c);m.dispatch(OnMainCameraChangedEvent(id(11)));m.process_queue();CHECK(c.count()==0);CHECK(m.is_queue_empty());
    }
}
