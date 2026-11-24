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

TEST_SUITE("EditorCRI005_RealmPortalPanel") {
    TEST_CASE("CRI005 RealmPortalPanel: missing GUI prevents framebuffer and render pass") {
        PanelContext c; Realm realm; c.ctx.active_realm=&realm; c.ctx.gui=nullptr;
        RealmPortalPanel p; p.initialize(&c.ctx); p.on_render();
        CHECK(c.renderer.creates==0); CHECK(c.renderer.begins==0);
    }
    TEST_CASE("CRI005 RealmPortalPanel: missing event manager prevents partial initialization") {
        PanelContext c; c.ctx.event_manager=nullptr;
        RealmPortalPanel p; p.initialize(&c.ctx);
        CHECK(p.get_framebuffer_id()==0); CHECK(c.renderer.creates==0);
    }
    TEST_CASE("CRI005 RealmPortalPanel: failed framebuffer never starts render pass") {
        PanelContext c; Realm realm; c.ctx.active_realm=&realm; c.renderer.result=0;
        RealmPortalPanel p; p.initialize(&c.ctx); p.on_render();
        CHECK(c.renderer.creates==1); CHECK(p.get_framebuffer_id()==0);
        CHECK(c.renderer.begins==0); CHECK(c.renderer.ends==0);
    }
    TEST_CASE("CRI005 RealmPortalPanel: locked collapsed window keeps disabled stack balanced") {
        GuiFrame gui; PortalFixture f; f.panel.lock();
        for(int i=0;i<2;++i) gui.frame([&]{
            ImGui::SetNextWindowCollapsed(true,ImGuiCond_Always);
            f.panel.on_gui_update();
            CHECK(gui.current->DisabledStackSize==0);
        });
        CHECK(f.panel.is_locked()); CHECK(f.panel.get_visibility());
    }
    TEST_CASE("CRI005 RealmPortalPanel: event before initialization is safe") {
        RealmPortalPanel p; OnMainCameraChangedEvent e(id(11)); p.on_event(e);
        CHECK(p.get_framebuffer_id()==0); CHECK(p.get_visibility());
    }
}

#include <Salix/events/sdl/SDLEvent.h>

TEST_SUITE("EditorCRI006_RealmPortalPanel") {
    TEST_CASE("CRI006 RealmPortalPanel: name mutation retains framebuffer identity") {
        PortalFixture f; f.panel.set_name("Preview output"); CHECK(f.panel.get_name()=="Preview output"); CHECK(f.panel.get_framebuffer_id()==73);
    }

    TEST_CASE("CRI006 RealmPortalPanel: lock and unlock preserve visibility and framebuffer") {
        PortalFixture f; f.panel.lock(); CHECK(f.panel.is_locked()); f.panel.unlock(); CHECK_FALSE(f.panel.is_locked()); CHECK(f.panel.get_visibility()); CHECK(f.panel.get_framebuffer_id()==73);
    }

    TEST_CASE("CRI006 RealmPortalPanel: viewport update recreates framebuffer for new size") {
        GuiFrame g; PortalFixture f; for(int i=0;i<2;++i)g.frame([&]{ImGui::SetNextWindowSize(ImVec2(500,400),ImGuiCond_Always); f.panel.on_gui_update();}); CHECK(f.c.renderer.deletes>=1); CHECK(f.c.renderer.creates>=2); CHECK(f.panel.get_viewport_size().x<1280); CHECK(f.panel.get_viewport_size().y<720);
    }

    TEST_CASE("CRI006 RealmPortalPanel: viewport resize updates selected camera projection") {
        GuiFrame g; PortalFixture f; auto* camera=f.add_camera(11); REQUIRE(camera); f.select(11); f.c.ctx.main_camera=camera; camera->set_viewport_size(100,100); auto before=camera->get_projection_matrix()[0][0]; for(int i=0;i<2;++i)g.frame([&]{ImGui::SetNextWindowSize(ImVec2(600,300),ImGuiCond_Always);f.panel.on_gui_update();}); CHECK(camera->get_projection_matrix()[0][0]!=before);
    }

    TEST_CASE("CRI006 RealmPortalPanel: missing renderer during GUI update leaves viewport unchanged") {
        GuiFrame g; PortalFixture f; f.c.init.renderer=nullptr; g.frame([&]{ImGui::SetNextWindowSize(ImVec2(400,300),ImGuiCond_Always);f.panel.on_gui_update();}); CHECK(f.c.renderer.creates==1); CHECK(f.panel.get_viewport_size().x==1280);
    }

    TEST_CASE("CRI006 RealmPortalPanel: missing renderer during render prevents pass") {
        PortalFixture f; f.c.ctx.renderer=nullptr; f.panel.on_render(); CHECK(f.c.renderer.begins==0); CHECK(f.c.renderer.ends==0);
    }

    TEST_CASE("CRI006 RealmPortalPanel: unrelated resize event does not change camera selection") {
        PortalFixture f; auto* camera=f.add_camera(11); REQUIRE(camera); f.select(11); WindowResizeEvent e(20,30); f.panel.on_event(e); f.panel.on_render(); CHECK(f.c.renderer.camera==camera);
    }

    TEST_CASE("CRI006 RealmPortalPanel: entity purge notification for different entity preserves camera") {
        PortalFixture f; auto* camera=f.add_camera(11); REQUIRE(camera); f.select(11); auto* other=f.realm.create_entity(id(12),"Other"); BeforeEntityPurgedEvent e(other); f.panel.on_event(e); f.panel.on_render(); CHECK(f.c.renderer.camera==camera);
    }

    TEST_CASE("CRI006 RealmPortalPanel: element purge notification for different ID preserves camera") {
        PortalFixture f; auto* camera=f.add_camera(11); REQUIRE(camera); f.select(11); BeforeElementPurgedEvent e(id(99),id(11)); f.panel.on_event(e); f.panel.on_render(); CHECK(f.c.renderer.camera==camera);
    }

    TEST_CASE("CRI006 RealmPortalPanel: camera deactivated outside panel uses fallback") {
        PortalFixture f; auto* camera=f.add_camera(11); REQUIRE(camera); f.select(11); camera->deactivate(); f.panel.on_render(); REQUIRE(f.c.renderer.camera); CHECK(f.c.renderer.camera!=camera); CHECK(f.c.renderer.begins==f.c.renderer.ends);
    }
}


namespace {
using namespace EditorCri;
struct PortalSprites {
    AssetManager assets; PortalFixture fixture;
    PortalSprites(){assets.initialize(&fixture.c.renderer); REQUIRE(fixture.add_camera(11)); fixture.select(11);}
    Sprite2D* sprite(uint64_t n,const char* path){return add_textured_sprite(fixture.realm,assets,n,path);}
    void render(){fixture.panel.on_render(); CHECK(fixture.c.renderer.begins==fixture.c.renderer.ends);}
    auto& draws(){return fixture.c.renderer.sprites;}
};
}
TEST_SUITE("EditorCRI009_RealmPortalPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI009 RealmPortalPanel: textured sprite produces expected scaled model matrix") {
        PortalSprites v; auto* s=v.sprite(21,"a.png"); v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].texture==s->get_texture()); CHECK(v.draws()[0].matrix[0][0]==doctest::Approx(.8)); CHECK(v.draws()[0].matrix[1][1]==doctest::Approx(.4));
    }

    TEST_CASE("CRI009 RealmPortalPanel: sorting layers control texture draw order") {
        PortalSprites v; auto* a=v.sprite(21,"a.png"); auto* b=v.sprite(22,"b.png"); a->sorting_layer=7; b->sorting_layer=-2; v.render(); REQUIRE(v.draws().size()==2); CHECK(v.draws()[0].texture==b->get_texture()); CHECK(v.draws()[1].texture==a->get_texture());
    }

    TEST_CASE("CRI009 RealmPortalPanel: hidden entity excludes textured sprite") {
        PortalSprites v; auto* s=v.sprite(21,"a.png"); s->get_owner()->set_visible(false); v.sprite(22,"b.png"); v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].texture!=s->get_texture());
    }

    TEST_CASE("CRI009 RealmPortalPanel: hidden component is excluded independently of entity") {
        PortalSprites v; auto* s=v.sprite(21,"a.png"); s->set_visibility(false); v.render(); CHECK(v.draws().empty()); CHECK(s->get_owner()->is_visible());
    }

    TEST_CASE("CRI009 RealmPortalPanel: component with no texture is excluded") {
        PortalSprites v; auto* s=v.sprite(21,"a.png"); s->load_texture(&v.assets,""); CHECK(s->get_texture()==nullptr); v.render(); CHECK(v.draws().empty());
    }

    TEST_CASE("CRI009 RealmPortalPanel: flip flags reverse local matrix axes") {
        PortalSprites v; auto* s=v.sprite(21,"a.png"); s->flip_h=true; s->flip_v=true; v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[0][0]==doctest::Approx(-.8)); CHECK(v.draws()[0].matrix[1][1]==doctest::Approx(-.4));
    }

    TEST_CASE("CRI009 RealmPortalPanel: sprite offset combines with entity position") {
        PortalSprites v; auto* s=v.sprite(21,"a.png"); s->get_owner()->get_transform()->set_position(3,4,5); s->offset=Vector2(2,-1); v.render(); REQUIRE(v.draws().size()==1); const auto& m=v.draws()[0].matrix; CHECK(m[3][0]==doctest::Approx(5)); CHECK(m[3][1]==doctest::Approx(3)); CHECK(m[3][2]==doctest::Approx(5));
    }

    TEST_CASE("CRI009 RealmPortalPanel: pivot shifts local geometry before translation") {
        PortalSprites v; auto* s=v.sprite(21,"a.png"); s->pivot=Vector2(0,1); v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[3][0]==doctest::Approx(.4)); CHECK(v.draws()[0].matrix[3][1]==doctest::Approx(-.2));
    }

    TEST_CASE("CRI009 RealmPortalPanel: local rotation and tint reach renderer") {
        PortalSprites v; auto* s=v.sprite(21,"a.png"); s->local_rotation=90; s->color=Color(.2f,.3f,.4f,.5f); v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[0][1]==doctest::Approx(.8)); CHECK(v.draws()[0].matrix[1][0]==doctest::Approx(-.4)); CHECK(v.draws()[0].color==s->color);
    }

    TEST_CASE("CRI009 RealmPortalPanel: entity scale only applied when full entity transform requested") {
        PortalSprites v; auto* s=v.sprite(21,"a.png"); s->get_owner()->get_transform()->set_scale(2,3,1); s->use_entity_rotation=true; v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[0][0]==doctest::Approx(1.6)); CHECK(v.draws()[0].matrix[1][1]==doctest::Approx(1.2)); v.draws().clear(); s->use_entity_rotation=false; v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[0][0]==doctest::Approx(.8));
    }
}
