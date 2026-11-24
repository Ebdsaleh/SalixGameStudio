// Tests for Editor/panels/RealmPortalPanel.
#include "../fixtures/HeadlessGuiFixtures.h"
#include <Editor/panels/RealmPortalPanel.h>
#include <Editor/events/OnMainCameraChangedEvent.h>
#include <Salix/events/BeforeEntityPurgedEvent.h>
#include <Salix/events/BeforeElementPurgedEvent.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/Entity.h>
namespace {
using namespace EditorCri;
struct PortalFixture {PanelContext c; Realm realm; RealmPortalPanel panel;
    PortalFixture(){c.ctx.active_realm=&realm;panel.initialize(&c.ctx);}
    Camera* add_camera(uint64_t n){auto* e=realm.create_entity(id(n),"Camera");return e->add_element<Camera>();}
    void select(uint64_t n){OnMainCameraChangedEvent e(id(n));panel.on_event(e);}
};
}
TEST_SUITE("EditorCRI002_RealmPortalPanel") {
    TEST_CASE("CRI002 RealmPortalPanel: default identity and viewport") {
        RealmPortalPanel p; CHECK(p.get_name()=="Realm Portal"); CHECK(p.get_framebuffer_id()==0); CHECK(p.get_viewport_size().x==1280); CHECK(p.get_viewport_size().y==720); CHECK(p.get_visibility());
    }

    TEST_CASE("CRI002 RealmPortalPanel: no context prevents framebuffer creation") {
        RealmPortalPanel p; p.initialize(nullptr); CHECK(p.get_framebuffer_id()==0); p.on_render(); CHECK(p.get_visibility());
    }

    TEST_CASE("CRI002 RealmPortalPanel: missing renderer leaves framebuffer absent") {
        PanelContext c; c.init.renderer=nullptr; RealmPortalPanel p; p.initialize(&c.ctx); CHECK(c.renderer.creates==0); CHECK(p.get_framebuffer_id()==0);
    }

    TEST_CASE("CRI002 RealmPortalPanel: successful initialization uses probe framebuffer") {
        PortalFixture f; CHECK(f.panel.get_framebuffer_id()==73); CHECK(f.c.renderer.creates==1); CHECK(f.c.renderer.width==1280); CHECK(f.c.renderer.height==720);
    }

    TEST_CASE("CRI002 RealmPortalPanel: no active realm or hidden panel does not render") {
        PortalFixture f; f.c.ctx.active_realm=nullptr; f.panel.on_render(); CHECK(f.c.renderer.begins==0); f.c.ctx.active_realm=&f.realm; f.panel.set_visibility(false); f.panel.on_render(); CHECK(f.c.renderer.begins==0);
    }

    TEST_CASE("CRI002 RealmPortalPanel: empty realm uses fallback camera and balanced render pass") {
        PortalFixture f; f.panel.on_render(); CHECK(f.c.renderer.begins==1); CHECK(f.c.renderer.ends==1); CHECK(f.c.renderer.clears==1); CHECK(f.c.renderer.last_id==73); REQUIRE(f.c.renderer.camera); CHECK_FALSE(f.c.renderer.camera->get_is_active());
    }

    TEST_CASE("CRI002 RealmPortalPanel: camera selection activates actual camera") {
        PortalFixture f; auto* camera=f.add_camera(11); REQUIRE(camera); f.select(11); CHECK(f.realm.get_main_camera_entity_id()==id(11)); CHECK(camera->get_is_active()); f.panel.on_render(); CHECK(f.c.renderer.camera==camera);
    }

    TEST_CASE("CRI002 RealmPortalPanel: switching camera deactivates previous camera") {
        PortalFixture f; auto* a=f.add_camera(11); auto* b=f.add_camera(12); REQUIRE(a); REQUIRE(b); f.select(11); f.select(12); CHECK_FALSE(a->get_is_active()); CHECK(b->get_is_active()); f.panel.on_render(); CHECK(f.c.renderer.camera==b);
    }

    TEST_CASE("CRI002 RealmPortalPanel: stale camera ID cleared and fallback used") {
        PortalFixture f; f.realm.set_main_camera_entity(id(99)); f.panel.on_render(); CHECK_FALSE(f.realm.get_main_camera_entity_id().is_valid()); REQUIRE(f.c.renderer.camera); CHECK_FALSE(f.c.renderer.camera->get_is_active());
    }

    TEST_CASE("CRI002 RealmPortalPanel: camera deselection clears active ID and uses fallback") {
        PortalFixture f; auto* a=f.add_camera(11); REQUIRE(a); f.select(11); f.select(0); CHECK_FALSE(a->get_is_active()); CHECK_FALSE(f.realm.get_main_camera_entity_id().is_valid()); f.panel.on_render(); REQUIRE(f.c.renderer.camera); CHECK(f.c.renderer.camera!=a);
    }
}
