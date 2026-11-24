// Tests for Editor/panels/RealmDesignerPanel.
#include "../fixtures/HeadlessGuiFixtures.h"
#include <Editor/panels/RealmDesignerPanel.h>
#include <Editor/events/OnRootEntityAddedEvent.h>
#include <Editor/events/OnEntityAddedEvent.h>
#include <Editor/events/OnChildEntityAddedEvent.h>
#include <Editor/events/OnEntityFamilyAddedEvent.h>
#include <Editor/events/OnEntityPurgedEvent.h>
#include <Editor/events/OnElementAddedEvent.h>
#include <Editor/events/PropertyValueChangedEvent.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
namespace {
using namespace EditorCri;
struct DesignerFixture { PanelContext c; RealmDesignerPanel panel; DesignerFixture(){panel.initialize(&c.ctx);}
    Entity* live(uint64_t n){return c.ctx.preview_realm->get_entity_by_id(id(n));}
    void add(uint64_t n){OnRootEntityAddedEvent e(entity(n,"Root"));panel.on_event(e);}
};
}
TEST_SUITE("EditorCRI002_RealmDesignerPanel") {
    TEST_CASE("CRI002 RealmDesignerPanel: default identity framebuffer and viewport") {
        RealmDesignerPanel p; CHECK(p.get_name()=="Realm Designer"); CHECK(p.get_framebuffer_id()==0); CHECK(p.get_viewport_size().x==1280); CHECK(p.get_viewport_size().y==720); CHECK(p.get_visibility()); CHECK_FALSE(p.is_locked());
    }

    TEST_CASE("CRI002 RealmDesignerPanel: missing renderer prevents preview creation") {
        PanelContext c; c.init.renderer=nullptr; RealmDesignerPanel p; p.initialize(&c.ctx); CHECK(p.get_framebuffer_id()==0); CHECK(c.renderer.creates==0); CHECK(c.ctx.preview_realm==nullptr);
    }

    TEST_CASE("CRI002 RealmDesignerPanel: initializes framebuffer and preview with requested dimensions") {
        DesignerFixture f; CHECK(f.panel.get_framebuffer_id()==73); CHECK(f.c.renderer.creates==1); CHECK(f.c.renderer.width==1280); CHECK(f.c.renderer.height==720); REQUIRE(f.c.ctx.preview_realm); CHECK(f.c.ctx.preview_realm->get_name()=="Preview");
    }

    TEST_CASE("CRI002 RealmDesignerPanel: failed framebuffer creation reported as zero") {
        PanelContext c; c.renderer.result=0; RealmDesignerPanel p; p.initialize(&c.ctx); CHECK(p.get_framebuffer_id()==0); CHECK(c.renderer.creates==1);
    }

    TEST_CASE("CRI002 RealmDesignerPanel: hidden render does not start render pass") {
        DesignerFixture f; f.panel.lock(); f.panel.set_visibility(false); f.panel.on_render(); f.panel.on_gui_update(); CHECK(f.c.renderer.begins==0); CHECK(f.panel.is_locked()); f.panel.unlock(); CHECK_FALSE(f.panel.is_locked());
    }

    TEST_CASE("CRI002 RealmDesignerPanel: root addition instantiates the authored identity") {
        DesignerFixture f; f.add(11); REQUIRE(f.live(11)); CHECK(f.live(11)->get_name()=="Root"); CHECK(f.live(11)->get_transform()!=nullptr);
    }

    TEST_CASE("CRI002 RealmDesignerPanel: entity added as child is linked to live parent") {
        DesignerFixture f; f.add(11); auto a=entity(12,"Child"); a.parent_id=id(11); OnEntityAddedEvent e(a); f.panel.on_event(e); REQUIRE(f.live(12)); CHECK(f.live(12)->get_parent()==f.live(11)); CHECK_FALSE(f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI002 RealmDesignerPanel: family creation establishes parent links") {
        DesignerFixture f; auto a=entity(11,"Parent"); auto b=entity(12,"Child"); b.parent_id=id(11); OnEntityFamilyAddedEvent e({a,b}); f.panel.on_event(e); REQUIRE(f.live(11)); REQUIRE(f.live(12)); CHECK(f.live(12)->get_parent()==f.live(11));
    }

    TEST_CASE("CRI002 RealmDesignerPanel: property event mutates preview transform but live mode ignores it") {
        DesignerFixture f; f.add(11); auto* e=f.live(11); REQUIRE(e); auto* t=e->get_transform(); REQUIRE(t); PropertyValueChangedEvent event(id(11),t->get_id(),"Transform","position",Vector3(3,4,5)); f.panel.on_event(event); CHECK(t->get_position().x==doctest::Approx(3)); f.c.ctx.data_mode=EditorDataMode::Live; PropertyValueChangedEvent other(id(11),t->get_id(),"Transform","position",Vector3(9,9,9)); f.panel.on_event(other); CHECK(t->get_position().x==doctest::Approx(3));
    }

    TEST_CASE("CRI002 RealmDesignerPanel: unknown component and absent parent do not add elements") {
        DesignerFixture f; f.add(11); auto* live=f.live(11); REQUIRE(live); auto n=live->get_all_elements().size(); OnElementAddedEvent unknown(id(11),element("CRI002_Unknown",80)); f.panel.on_event(unknown); OnElementAddedEvent absent(id(99),element("Transform",81)); f.panel.on_event(absent); CHECK(live->get_all_elements().size()==n); CHECK(live->get_element_by_id(id(80))==nullptr);
    }
}
