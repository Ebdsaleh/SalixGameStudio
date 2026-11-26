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

TEST_SUITE("EditorCRI005_RealmDesignerPanel") {
    TEST_CASE("CRI005 RealmDesignerPanel: missing GUI prevents framebuffer and preview creation") {
        PanelContext c; c.ctx.gui=nullptr; RealmDesignerPanel p; p.initialize(&c.ctx);
        CHECK(c.renderer.creates==0); CHECK(c.ctx.preview_realm==nullptr);
    }
    TEST_CASE("CRI005 RealmDesignerPanel: child event with missing parent does not dispatch invalid hierarchy") {
        DesignerFixture f; auto a=entity(12,"Orphan"); a.parent_id=id(99);
        OnChildEntityAddedEvent e(a); f.panel.on_event(e);
        REQUIRE(f.live(12)); CHECK(f.live(12)->get_parent()==nullptr);
        CHECK(f.c.events.is_queue_empty());
    }
    TEST_CASE("CRI005 RealmDesignerPanel: child event with valid parent retains hierarchy notification") {
        DesignerFixture f; f.c.ctx.preview_realm->create_entity(id(11),"Parent");
        auto a=entity(12,"Child"); a.parent_id=id(11);
        OnChildEntityAddedEvent e(a); f.panel.on_event(e);
        REQUIRE(f.live(12)); CHECK(f.live(12)->get_parent()==f.live(11));
        CHECK_FALSE(f.c.events.is_queue_empty());
    }
}

#include <Editor/events/OnHierarchyChangedEvent.h>
#include <Editor/events/OnEntityFamilyPurgedEvent.h>
#include <Salix/events/sdl/SDLEvent.h>
#include <Salix/ecs/BoxCollider.h>

TEST_SUITE("EditorCRI006_RealmDesignerPanel") {
    TEST_CASE("CRI006 RealmDesignerPanel: reparent notification links existing preview objects") {
        DesignerFixture f; f.add(11); f.add(12); OnHierarchyChangedEvent e(id(12),id(11)); f.panel.on_event(e); REQUIRE(f.live(12)); CHECK(f.live(12)->get_parent()==f.live(11)); CHECK_FALSE(f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI006 RealmDesignerPanel: release notification makes child a root") {
        DesignerFixture f; f.add(11); f.add(12); f.live(12)->set_parent(f.live(11)); OnHierarchyChangedEvent e(id(12),SimpleGuid::invalid()); f.panel.on_event(e); CHECK(f.live(12)->get_parent()==nullptr);
    }

    TEST_CASE("CRI006 RealmDesignerPanel: family event resolves child before parent ordering") {
        DesignerFixture f; auto a=entity(11,"Parent"); auto b=entity(12,"Child"); b.parent_id=a.id; OnEntityFamilyAddedEvent e({b,a}); f.panel.on_event(e); REQUIRE(f.live(12)); CHECK(f.live(12)->get_parent()==f.live(11));
    }

    TEST_CASE("CRI006 RealmDesignerPanel: family with missing parent retains root") {
        DesignerFixture f; auto a=entity(11,"Orphan"); a.parent_id=id(99); OnEntityFamilyAddedEvent e({a}); f.panel.on_event(e); REQUIRE(f.live(11)); CHECK(f.live(11)->get_parent()==nullptr);
    }

    TEST_CASE("CRI006 RealmDesignerPanel: purge event flags only requested independent entity") {
        DesignerFixture f; f.add(11); f.add(12); OnEntityPurgedEvent e(id(11)); f.panel.on_event(e); REQUIRE(f.live(11)); CHECK(f.live(11)->is_purged()); CHECK_FALSE(f.live(12)->is_purged());
    }

    TEST_CASE("CRI006 RealmDesignerPanel: family purge ignores missing ID and flags present members") {
        DesignerFixture f; f.add(11); f.add(12); f.add(13); OnEntityFamilyPurgedEvent e({id(11),id(99),id(12)}); f.panel.on_event(e); CHECK(f.live(11)->is_purged()); CHECK(f.live(12)->is_purged()); CHECK_FALSE(f.live(13)->is_purged());
    }

    TEST_CASE("CRI006 RealmDesignerPanel: valid new collider assigned authored ID") {
        DesignerFixture f; f.add(11); auto a=element("BoxCollider",91,"{}"); OnElementAddedEvent e(id(11),a); f.panel.on_event(e); REQUIRE(f.live(11)); auto* added=f.live(11)->get_element_by_id(id(91)); REQUIRE(added); CHECK(dynamic_cast<BoxCollider*>(added)!=nullptr); CHECK_FALSE(f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI006 RealmDesignerPanel: unrecognised property leaves existing position unchanged") {
        DesignerFixture f; f.add(11); auto* t=f.live(11)->get_transform(); t->set_position(1,2,3); PropertyValueChangedEvent e(id(11),t->get_id(),"Transform","not_a_property",Vector3(7,8,9)); f.panel.on_event(e); CHECK(t->get_position()==Vector3(1,2,3));
    }

    TEST_CASE("CRI006 RealmDesignerPanel: missing entity and element property updates leave live state alone") {
        DesignerFixture f; f.add(11); auto* t=f.live(11)->get_transform(); t->set_position(1,2,3); PropertyValueChangedEvent a(id(99),t->get_id(),"Transform","position",Vector3(7,8,9)); PropertyValueChangedEvent b(id(11),id(99),"Transform","position",Vector3(7,8,9)); f.panel.on_event(a); f.panel.on_event(b); CHECK(t->get_position()==Vector3(1,2,3));
    }

    TEST_CASE("CRI006 RealmDesignerPanel: unrelated event leaves framebuffer and realm intact") {
        DesignerFixture f; f.add(11); WindowResizeEvent e(20,30); f.panel.on_event(e); f.panel.on_gui_render(); CHECK(f.panel.get_framebuffer_id()==73); CHECK(f.live(11)!=nullptr); CHECK(f.c.renderer.begins==0);
    }
}

#include <Editor/camera/EditorCamera.h>
#include <Salix/ecs/Sprite2D.h>
namespace {
using namespace EditorCri;
struct DesignerViewportFixture {
    GuiFrame gui; DesignerFixture f; EditorCamera camera;
    DesignerViewportFixture() { camera.initialize(&f.c.ctx); f.c.ctx.editor_camera=&camera; f.c.ctx.grid_settings.size=2; f.c.ctx.grid_settings.major_division=1; f.c.ctx.grid_settings.minor_division=.5f; }
    void frame(int width=600,int height=400) { gui.frame([&]{ImGui::SetNextWindowPos(ImVec2(20,20),ImGuiCond_Always); ImGui::SetNextWindowSize(ImVec2((float)width,(float)height),ImGuiCond_Always); f.panel.on_gui_update();}); }
};

TEST_SUITE("EditorCRI007_RealmDesignerPanel") {
    TEST_CASE("CRI007 RealmDesignerPanel: viewport resize recreates framebuffer with positive dimensions") {
        DesignerViewportFixture v; v.frame(); CHECK(v.f.c.renderer.deletes>=1); CHECK(v.f.c.renderer.creates>=2); CHECK(v.f.panel.get_viewport_size().x>0); CHECK(v.f.panel.get_viewport_size().x<1280);
    }

    TEST_CASE("CRI007 RealmDesignerPanel: second viewport size changes camera projection") {
        DesignerViewportFixture v; v.frame(600,400); auto before=v.camera.get_projection_matrix()[0][0]; v.frame(800,300); CHECK(v.camera.get_projection_matrix()[0][0]!=before);
    }

    TEST_CASE("CRI007 RealmDesignerPanel: YAML viewport drains sync commands in insertion order") {
        DesignerViewportFixture v; std::vector<int> calls; v.f.c.ctx.sync_queue.push_back([&]{calls.push_back(1);}); v.f.c.ctx.sync_queue.push_back([&]{calls.push_back(2);}); v.frame(); REQUIRE(calls.size()==2); CHECK(calls[0]==1); CHECK(calls[1]==2); CHECK(v.f.c.ctx.sync_queue.empty()); v.frame(); CHECK(calls.size()==2);
    }

    TEST_CASE("CRI007 RealmDesignerPanel: live viewport does not drain YAML sync commands") {
        DesignerViewportFixture v; v.f.c.ctx.data_mode=EditorDataMode::Live; int calls=0; v.f.c.ctx.sync_queue.push_back([&]{++calls;}); v.frame(); CHECK(calls==0); CHECK(v.f.c.ctx.sync_queue.size()==1);
    }

    TEST_CASE("CRI007 RealmDesignerPanel: hidden viewport does not resize or drain commands") {
        DesignerViewportFixture v; v.f.panel.set_visibility(false); int calls=0; v.f.c.ctx.sync_queue.push_back([&]{++calls;}); v.frame(); CHECK(calls==0); CHECK(v.f.c.renderer.creates==1);
    }

    TEST_CASE("CRI007 RealmDesignerPanel: missing renderer during update leaves viewport unchanged") {
        DesignerViewportFixture v; v.f.c.init.renderer=nullptr; v.frame(); CHECK(v.f.c.renderer.creates==1); CHECK(v.f.panel.get_viewport_size().x==1280); CHECK(v.gui.current->StyleVarStack.Size==0);
    }

    TEST_CASE("CRI007 RealmDesignerPanel: headless empty preview renders balanced pass") {
        DesignerViewportFixture v; v.f.c.ctx.grid_settings.snap_enabled=false; v.f.panel.on_render(); CHECK(v.f.c.renderer.begins==1); CHECK(v.f.c.renderer.ends==1); CHECK(v.f.c.renderer.clears==1); CHECK(v.f.c.renderer.camera==&v.camera);
    }

    TEST_CASE("CRI007 RealmDesignerPanel: grid disabled draws only picking ray") {
        DesignerViewportFixture v; v.f.c.ctx.grid_settings.snap_enabled=false; v.f.panel.on_render(); CHECK(v.f.c.renderer.lines==1); CHECK(v.f.c.renderer.ends==1);
    }

    TEST_CASE("CRI007 RealmDesignerPanel: grid enabled draws major and minor lines") {
        DesignerViewportFixture v; v.f.c.ctx.grid_settings.snap_enabled=true; v.f.panel.on_render(); CHECK(v.f.c.renderer.lines>11); CHECK(v.f.c.renderer.begins==v.f.c.renderer.ends);
    }

    TEST_CASE("CRI007 RealmDesignerPanel: grid without minor divisions draws fewer lines") {
        DesignerViewportFixture v; v.f.c.ctx.grid_settings.snap_enabled=true; v.f.c.ctx.grid_settings.minor_division=1; v.f.panel.on_render(); const auto major=v.f.c.renderer.lines; CHECK(major==11); v.f.c.renderer.lines=0; v.f.c.ctx.grid_settings.minor_division=.5f; v.f.panel.on_render(); CHECK(v.f.c.renderer.lines>major);
    }
}
}


namespace {
using namespace EditorCri;
struct DesignerSprites {
    AssetManager assets; DesignerViewportFixture viewport;
    DesignerSprites(){assets.initialize(&viewport.f.c.renderer);viewport.f.c.ctx.grid_settings.snap_enabled=false;}
    Sprite2D* sprite(uint64_t n,const char* path){return add_textured_sprite(*viewport.f.c.ctx.preview_realm,assets,n,path);}
    void render(){viewport.f.panel.on_render(); CHECK(viewport.f.c.renderer.begins==viewport.f.c.renderer.ends);}
    auto& draws(){return viewport.f.c.renderer.sprites;}
};
}
TEST_SUITE("EditorCRI009_RealmDesignerPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI009 RealmDesignerPanel: textured sprite produces expected scaled model matrix") {
        DesignerSprites v; auto* s=v.sprite(21,"a.png"); v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].texture==s->get_texture()); CHECK(v.draws()[0].matrix[0][0]==doctest::Approx(.8)); CHECK(v.draws()[0].matrix[1][1]==doctest::Approx(.4));
    }

    TEST_CASE("CRI009 RealmDesignerPanel: sorting layers control texture draw order") {
        DesignerSprites v; auto* a=v.sprite(21,"a.png"); auto* b=v.sprite(22,"b.png"); a->sorting_layer=7; b->sorting_layer=-2; v.render(); REQUIRE(v.draws().size()==2); CHECK(v.draws()[0].texture==b->get_texture()); CHECK(v.draws()[1].texture==a->get_texture());
    }

    TEST_CASE("CRI009 RealmDesignerPanel: hidden entity excludes textured sprite") {
        DesignerSprites v; auto* s=v.sprite(21,"a.png"); s->get_owner()->set_visible(false); v.sprite(22,"b.png"); v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].texture!=s->get_texture());
    }

    TEST_CASE("CRI009 RealmDesignerPanel: hidden component is excluded independently of entity") {
        DesignerSprites v; auto* s=v.sprite(21,"a.png"); s->set_visibility(false); v.render(); CHECK(v.draws().empty()); CHECK(s->get_owner()->is_visible());
    }

    TEST_CASE("CRI009 RealmDesignerPanel: component with no texture is excluded") {
        DesignerSprites v; auto* s=v.sprite(21,"a.png"); s->load_texture(&v.assets,""); CHECK(s->get_texture()==nullptr); v.render(); CHECK(v.draws().empty());
    }

    TEST_CASE("CRI009 RealmDesignerPanel: flip flags reverse local matrix axes") {
        DesignerSprites v; auto* s=v.sprite(21,"a.png"); s->flip_h=true; s->flip_v=true; v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[0][0]==doctest::Approx(-.8)); CHECK(v.draws()[0].matrix[1][1]==doctest::Approx(-.4));
    }

    TEST_CASE("CRI009 RealmDesignerPanel: sprite offset combines with entity position") {
        DesignerSprites v; auto* s=v.sprite(21,"a.png"); s->get_owner()->get_transform()->set_position(3,4,5); s->offset=Vector2(2,-1); v.render(); REQUIRE(v.draws().size()==1); const auto& m=v.draws()[0].matrix; CHECK(m[3][0]==doctest::Approx(5)); CHECK(m[3][1]==doctest::Approx(3)); CHECK(m[3][2]==doctest::Approx(5));
    }

    TEST_CASE("CRI009 RealmDesignerPanel: pivot shifts local geometry before translation") {
        DesignerSprites v; auto* s=v.sprite(21,"a.png"); s->pivot=Vector2(0,1); v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[3][0]==doctest::Approx(.4)); CHECK(v.draws()[0].matrix[3][1]==doctest::Approx(-.2));
    }

    TEST_CASE("CRI009 RealmDesignerPanel: local rotation and tint reach renderer") {
        DesignerSprites v; auto* s=v.sprite(21,"a.png"); s->local_rotation=90; s->color=Color(.2f,.3f,.4f,.5f); v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[0][1]==doctest::Approx(.8)); CHECK(v.draws()[0].matrix[1][0]==doctest::Approx(-.4)); CHECK(v.draws()[0].color==s->color);
    }

    TEST_CASE("CRI009 RealmDesignerPanel: entity scale only applied when full entity transform requested") {
        DesignerSprites v; auto* s=v.sprite(21,"a.png"); s->get_owner()->get_transform()->set_scale(2,3,1); s->use_entity_rotation=true; v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[0][0]==doctest::Approx(1.6)); CHECK(v.draws()[0].matrix[1][1]==doctest::Approx(1.2)); v.draws().clear(); s->use_entity_rotation=false; v.render(); REQUIRE(v.draws().size()==1); CHECK(v.draws()[0].matrix[0][0]==doctest::Approx(.8));
    }
}

namespace {
using namespace EditorCri;
struct PickingFixture {
    DesignerViewportFixture v;
    PickingFixture() {
        v.camera.set_2D_mode(false);
        v.camera.get_transform()->set_position(0,0,10);
        v.camera.get_transform()->set_rotation(0,0,0);
        v.camera.set_viewport_size(400,400);
        v.camera.set_orthographic_size(10);
    }
    void add(uint64_t n,float x,float y,float z,bool collider=true,uint64_t parent=0) {
        auto a=entity(n,"Pick");
        a.parent_id=id(parent);
        auto t=element("Transform",1000+n,"{position: {x: 0,y: 0,z: 0}, rotation: {x: 0,y: 0,z: 0}, scale: {x: 1,y: 1,z: 1}}");
        t.owner_id=a.id;t.data["position"]["x"]=x;t.data["position"]["y"]=y;t.data["position"]["z"]=z;a.elements.push_back(t);
        if(collider){auto c=element("BoxCollider",2000+n,"{size: {x: 2,y: 2,z: 2}}");c.owner_id=a.id;a.elements.push_back(c);}
        if(parent)v.f.c.ctx.editor_realm_manager->add_child_entity(a);else v.f.c.ctx.editor_realm_manager->add_entity(a);
        auto* live=v.f.c.ctx.preview_realm->create_entity(id(n),"Pick");live->get_transform()->set_position(x,y,z);
    }
    void pick(ProjectionMode mode,float x=200,float y=200,bool twice=false) {
        v.camera.set_projection_mode(mode);
        v.f.panel.pick_at_for_testing(ImVec2(x,y),ImVec2(0,0),ImVec2(400,400),twice);
    }
    SimpleGuid selected(){return v.f.c.ctx.selected_entity_id;}
};
}
TEST_SUITE("EditorCRI011_RealmDesignerPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI011 RealmDesignerPanel: orthographic central collider selects authored entity") {
        PickingFixture f; f.add(11,0,0,0); f.pick(ProjectionMode::Orthographic); CHECK(f.selected()==id(11)); CHECK(f.v.f.c.ctx.selected_entity==f.v.f.live(11));
    }

    TEST_CASE("CRI011 RealmDesignerPanel: orthographic miss clears selection") {
        PickingFixture f; f.add(11,0,0,0); f.v.f.c.ctx.selected_entity_id=id(11); f.pick(ProjectionMode::Orthographic,390,390); CHECK_FALSE(f.selected().is_valid()); CHECK(f.v.f.c.ctx.selected_entity==nullptr);
    }

    TEST_CASE("CRI011 RealmDesignerPanel: orthographic overlapping colliders select closest depth") {
        PickingFixture f; f.add(11,0,0,-2); f.add(12,0,0,2); f.pick(ProjectionMode::Orthographic); CHECK(f.selected()==id(12));
    }

    TEST_CASE("CRI011 RealmDesignerPanel: perspective central ray selects authored collider") {
        PickingFixture f; f.add(11,0,0,0); f.pick(ProjectionMode::Perspective); CHECK(f.selected()==id(11));
    }

    TEST_CASE("CRI011 RealmDesignerPanel: perspective ray misses off-axis collider") {
        PickingFixture f; f.add(11,100,0,0); f.pick(ProjectionMode::Perspective); CHECK_FALSE(f.selected().is_valid());
    }

    TEST_CASE("CRI011 RealmDesignerPanel: perspective overlap selects closest hit") {
        PickingFixture f; f.add(11,0,0,-2); f.add(12,0,0,2); f.pick(ProjectionMode::Perspective); CHECK(f.selected()==id(12));
    }

    TEST_CASE("CRI011 RealmDesignerPanel: collider-free entity is not selected by either projection") {
        PickingFixture f; f.add(11,0,0,0,false); f.pick(ProjectionMode::Perspective); CHECK_FALSE(f.selected().is_valid()); f.pick(ProjectionMode::Orthographic); CHECK_FALSE(f.selected().is_valid());
    }

    TEST_CASE("CRI011 RealmDesignerPanel: parent transform contributes to child's pick location") {
        PickingFixture f; f.add(11,3,0,0,false); f.add(12,-3,0,0,true,11); f.pick(ProjectionMode::Orthographic); CHECK(f.selected()==id(12));
    }

    TEST_CASE("CRI011 RealmDesignerPanel: double click focuses selected live transform") {
        PickingFixture f; f.add(11,0,0,0); f.pick(ProjectionMode::Orthographic,200,200,true); CHECK(f.selected()==id(11)); CHECK(glm::length(f.v.camera.get_transform()->get_position().to_glm())==doctest::Approx(3));
    }

    TEST_CASE("CRI011 RealmDesignerPanel: invalid viewport and uninitialized panel do not change selection") {
        PickingFixture f; f.v.f.c.ctx.selected_entity_id=id(77); f.v.f.panel.pick_at_for_testing(ImVec2(0,0),ImVec2(0,0),ImVec2(0,0)); CHECK(f.selected()==id(77)); RealmDesignerPanel p; CHECK_NOTHROW(p.pick_at_for_testing(ImVec2(1,1),ImVec2(0,0),ImVec2(10,10)));
    }
}

#include <glm/gtc/matrix_transform.hpp>
namespace {
using namespace EditorCri;
struct GizmoFixture {
    PickingFixture pick;
    DesignerViewportFixture& v=pick.v;
    struct Listener:IEventListener {
        std::map<std::string,PropertyValue> values;SimpleGuid owner=SimpleGuid::invalid(),component=SimpleGuid::invalid();
        void on_event(IEvent& e) override {if(e.get_event_type()!=EventType::EditorPropertyValueChanged)return;auto& change=static_cast<PropertyValueChangedEvent&>(e);values[change.property_name]=change.new_value;owner=change.entity_id;component=change.element_id;}
    } listener;
    GizmoFixture(){auto a=entity(11,"Gizmo");auto t=element("Transform",1011,"{position: {x: 0,y: 0,z: 0}, rotation: {x: 0,y: 0,z: 0}, scale: {x: 1,y: 1,z: 1}}");t.owner_id=a.id;a.elements.push_back(t);v.f.c.ctx.editor_realm_manager->add_entity(a);v.f.c.events.process_queue();REQUIRE(v.f.live(11));REQUIRE(v.f.live(11)->get_transform());v.f.c.ctx.selected_entity_id=id(11);v.f.c.events.subscribe(EventCategory::Editor,&listener);}
    Transform* transform(){return v.f.live(11)->get_transform();}
    void apply(float x,float y,float z){v.f.panel.apply_gizmo_matrix_for_testing(glm::translate(glm::mat4(1),glm::vec3(x,y,z)));}
};
}
TEST_SUITE("EditorCRI013_RealmDesignerPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI013 RealmDesignerPanel: gizmo translation applies live world coordinates") {
        GizmoFixture f;f.apply(3,4,5);auto p=f.transform()->get_world_position();CHECK(p.x==doctest::Approx(3));CHECK(p.y==doctest::Approx(4));CHECK(p.z==doctest::Approx(5));
    }

    TEST_CASE("CRI013 RealmDesignerPanel: gizmo scale applies all world axes") {
        GizmoFixture f;auto m=glm::scale(glm::mat4(1),glm::vec3(2,3,4));f.v.f.panel.apply_gizmo_matrix_for_testing(m);auto s=f.transform()->get_world_scale();CHECK(s.x==doctest::Approx(2));CHECK(s.y==doctest::Approx(3));CHECK(s.z==doctest::Approx(4));
    }

    TEST_CASE("CRI013 RealmDesignerPanel: gizmo rotation changes live world basis") {
        GizmoFixture f;auto m=glm::rotate(glm::mat4(1),glm::radians(45.f),glm::vec3(0,0,1));f.v.f.panel.apply_gizmo_matrix_for_testing(m);auto actual=f.transform()->get_model_matrix();CHECK(actual[0][0]==doctest::Approx(m[0][0]));CHECK(actual[0][1]==doctest::Approx(m[0][1]));
    }

    TEST_CASE("CRI013 RealmDesignerPanel: gizmo dispatches local transform property updates") {
        GizmoFixture f;f.apply(3,4,5);f.v.f.c.events.process_queue();REQUIRE(f.listener.values.size()==3);CHECK(std::get<Vector3>(f.listener.values.at("position")).x==doctest::Approx(3));CHECK(f.listener.values.count("rotation")==1);CHECK(f.listener.values.count("scale")==1);CHECK(f.listener.owner==id(11));CHECK(f.listener.component==id(1011));
    }

    TEST_CASE("CRI013 RealmDesignerPanel: locked designer rejects manipulated matrix") {
        GizmoFixture f;f.v.f.panel.lock();f.apply(3,4,5);CHECK(f.transform()->get_position()==Vector3(0,0,0));CHECK(f.v.f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI013 RealmDesignerPanel: live data mode does not mutate authored preview") {
        GizmoFixture f;f.v.f.c.ctx.data_mode=EditorDataMode::Live;f.apply(3,4,5);CHECK(f.transform()->get_position()==Vector3(0,0,0));CHECK(f.v.f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI013 RealmDesignerPanel: missing selected identity produces no transform update") {
        GizmoFixture f;f.v.f.c.ctx.selected_entity_id=id(99);f.apply(3,4,5);CHECK(f.transform()->get_position()==Vector3(0,0,0));CHECK(f.v.f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI013 RealmDesignerPanel: missing authored transform suppresses property events") {
        GizmoFixture f;f.v.f.c.ctx.editor_realm_manager->get_archetype(id(11))->elements.clear();f.apply(3,4,5);CHECK(f.transform()->get_position().x==doctest::Approx(3));CHECK(f.v.f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI013 RealmDesignerPanel: stale hierarchy event with absent entity does not assert") {
        DesignerFixture f;f.add(11);OnHierarchyChangedEvent event(id(99),id(11));CHECK_NOTHROW(f.panel.on_event(event));CHECK(f.live(11)->get_parent()==nullptr);
    }

    TEST_CASE("CRI013 RealmDesignerPanel: missing hierarchy target preserves current parent") {
        DesignerFixture f;f.add(11);f.add(12);f.live(12)->set_parent(f.live(11));OnHierarchyChangedEvent event(id(12),id(99));f.panel.on_event(event);CHECK(f.live(12)->get_parent()==f.live(11));
    }
}

#include "../fixtures/EditorEventFixtures.h"
namespace {
using namespace EditorCri;
bool contains_designer_texture(int n){auto* d=ImGui::GetDrawData();if(!d)return false;for(int i=0;i<d->CmdListsCount;++i)for(const auto& cmd:d->CmdLists[i]->CmdBuffer)if(cmd.GetTexID()==ImTextureID(n))return true;return false;}
struct ReloadFixture {
    DesignerSprites v;
    Sprite2D* sprite;
    EditorEventCapture<PropertyValueChangedEvent> capture;
    ReloadFixture():sprite(v.sprite(21,"old.png")){
        v.viewport.f.c.init.asset_manager=&v.assets;
        v.viewport.f.c.events.subscribe(EventCategory::Editor,&capture);
    }
    void change(const char* path,bool reload){PropertyValueChangedEvent e(id(21),sprite->get_id(),"Sprite2D","texture_path",std::string(path),reload);v.viewport.f.panel.on_event(e);}
};
}
TEST_SUITE("EditorCRI014_RealmDesignerPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI014 RealmDesignerPanel: visible framebuffer is submitted as image in headless viewport") {
        DesignerViewportFixture v;v.f.c.renderer.framebuffer_texture=ImTextureID(77);v.frame();v.frame();CHECK(contains_designer_texture(77));CHECK(v.gui.current->StyleVarStack.Size==0);
    }

    TEST_CASE("CRI014 RealmDesignerPanel: locked viewport still submits framebuffer without selection") {
        DesignerViewportFixture v;v.f.c.renderer.framebuffer_texture=ImTextureID(77);v.f.panel.lock();v.frame();v.frame();CHECK(contains_designer_texture(77));CHECK(v.f.panel.is_locked());CHECK_FALSE(v.f.c.ctx.selected_entity_id.is_valid());
    }

    TEST_CASE("CRI014 RealmDesignerPanel: live viewport presents framebuffer without authored selection") {
        DesignerViewportFixture v;v.f.c.ctx.data_mode=EditorDataMode::Live;v.f.c.renderer.framebuffer_texture=ImTextureID(77);v.frame();v.frame();CHECK(contains_designer_texture(77));CHECK_FALSE(v.f.c.ctx.selected_entity_id.is_valid());
    }

    TEST_CASE("CRI014 RealmDesignerPanel: missing preview realm cannot start render pass") {
        DesignerViewportFixture v;v.f.c.ctx.preview_realm.reset();v.f.panel.on_render();CHECK(v.f.c.renderer.begins==0);CHECK(v.f.c.renderer.ends==0);
    }

    TEST_CASE("CRI014 RealmDesignerPanel: live mode without active realm cannot start render pass") {
        DesignerViewportFixture v;v.f.c.ctx.data_mode=EditorDataMode::Live;v.f.c.ctx.active_realm=nullptr;v.f.panel.on_render();CHECK(v.f.c.renderer.begins==0);CHECK(v.f.c.renderer.ends==0);
    }

    TEST_CASE("CRI014 RealmDesignerPanel: texture reload applies reflected path and reloads synthetic texture") {
        ReloadFixture f;f.change("new.png",true);CHECK(f.sprite->texture_path=="new.png");REQUIRE(f.sprite->get_texture());CHECK(f.sprite->get_texture_width()==80);CHECK(f.sprite->get_texture_height()==40);CHECK(f.v.viewport.f.c.renderer.texture_loads==2);
    }

    TEST_CASE("CRI014 RealmDesignerPanel: reload emits width and height derived property notifications") {
        ReloadFixture f;f.change("new.png",true);f.v.viewport.f.c.events.process_queue();REQUIRE(f.capture.count()==2);std::map<std::string,int> dims;for(size_t i=0;i<f.capture.count();++i){auto& e=f.capture.at(i);CHECK(e.entity_id==id(21));CHECK(e.element_id==f.sprite->get_id());dims[e.property_name]=std::get<int>(e.new_value);}CHECK(dims.at("width")==80);CHECK(dims.at("height")==40);
    }

    TEST_CASE("CRI014 RealmDesignerPanel: nonreload property change does not reload texture or derive dimensions") {
        ReloadFixture f;f.change("new.png",false);CHECK(f.sprite->texture_path=="new.png");CHECK(f.v.viewport.f.c.renderer.texture_loads==1);CHECK(f.v.viewport.f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI014 RealmDesignerPanel: read-only derived width notification does not change live dimensions") {
        ReloadFixture f;PropertyValueChangedEvent e(id(21),f.sprite->get_id(),"Sprite2D","width",999);f.v.viewport.f.panel.on_event(e);CHECK(f.sprite->get_texture_width()==80);CHECK(f.v.viewport.f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI014 RealmDesignerPanel: reload to empty texture clears live size and derived values") {
        ReloadFixture f;f.sprite->load_texture(&f.v.assets,"");f.change("",true);f.v.viewport.f.c.events.process_queue();CHECK(f.sprite->get_texture()==nullptr);REQUIRE(f.capture.count()==2);for(size_t i=0;i<f.capture.count();++i)CHECK(std::get<int>(f.capture.at(i).new_value)==0);
    }
}

namespace {
using namespace EditorCri;
struct DesignerHierarchyFixture {
    DesignerFixture f;
    TempRealm file;
    EditorEventCapture<PropertyValueChangedEvent> changes;
    DesignerHierarchyFixture() {
        f.c.ctx.editor_realm_manager->load_realm_from_file(file.write(
            "Entities: [{name: Root, id: 11, elements: {Transform: {id: 1011}}}, "
            "{name: Child, id: 12, parent: 11, elements: {Transform: {id: 1012}}}, "
            "{name: Grandchild, id: 13, parent: 12, elements: {Transform: {id: 1013}}}, "
            "{name: Target, id: 14, elements: {Transform: {id: 1014}}}, "
            "{name: Other, id: 15, elements: {Transform: {id: 1015}}}]"));
        OnEntityFamilyAddedEvent added(f.c.ctx.editor_realm_manager->get_realm());
        f.panel.on_event(added);
        REQUIRE(f.live(11));
        REQUIRE(f.live(12));
        REQUIRE(f.live(13));
        REQUIRE(f.live(14));
        REQUIRE(f.live(15));
        REQUIRE(f.live(12)->get_parent() == f.live(11));
        REQUIRE(f.live(13)->get_parent() == f.live(12));
        f.c.events.subscribe(EventCategory::Editor, &changes);
    }
    void move(uint64_t owner, uint64_t target) {
        OnHierarchyChangedEvent event(id(owner), id(target));
        f.panel.on_event(event);
    }
    void drain() { f.c.events.process_queue(); }
    void check_properties(uint64_t owner) {
        std::map<std::string, Vector3> values;
        for (size_t i = 0; i < changes.count(); ++i) {
            auto& e = changes.at(i);
            if (e.entity_id != id(owner)) continue;
            CHECK(e.element_id == id(1000 + owner));
            CHECK(e.element_type_name == "Transform");
            REQUIRE(std::holds_alternative<Vector3>(e.new_value));
            values.emplace(e.property_name, std::get<Vector3>(e.new_value));
        }
        REQUIRE(values.size() == 3);
        auto* t = f.live(owner)->get_transform();
        REQUIRE(t);
        CHECK(values.at("position") == t->get_position());
        CHECK(values.at("rotation") == t->get_rotation());
        CHECK(values.at("scale") == t->get_scale());
    }
};
}
TEST_SUITE("EditorCRI016_RealmDesignerPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI016 RealmDesignerPanel: moving authored root preserves live descendant chain") {
        DesignerHierarchyFixture x;
        x.move(11, 14);
        CHECK(x.f.live(11)->get_parent() == x.f.live(14));
        CHECK(x.f.live(12)->get_parent() == x.f.live(11));
        CHECK(x.f.live(13)->get_parent() == x.f.live(12));
        CHECK(x.f.live(15)->get_parent() == nullptr);
        x.drain();
        REQUIRE(x.changes.count() == 9);
    }

    TEST_CASE("CRI016 RealmDesignerPanel: moved root publishes all local transform properties") {
        DesignerHierarchyFixture x;
        x.f.live(11)->get_transform()->set_position(2, 3, 4);
        x.f.live(14)->get_transform()->set_position(5, 6, 7);
        x.move(11, 14);
        x.drain();
        x.check_properties(11);
        x.check_properties(12);
        x.check_properties(13);
    }

    TEST_CASE("CRI016 RealmDesignerPanel: reparenting child publishes only its subtree") {
        DesignerHierarchyFixture x;
        x.move(12, 14);
        x.drain();
        REQUIRE(x.changes.count() == 6);
        x.check_properties(12);
        x.check_properties(13);
        CHECK(x.f.live(12)->get_parent() == x.f.live(14));
        CHECK(x.f.live(11)->get_parent() == nullptr);
        for (size_t i = 0; i < x.changes.count(); ++i)
            CHECK(x.changes.at(i).entity_id != id(11));
    }

    TEST_CASE("CRI016 RealmDesignerPanel: releasing child publishes subtree transforms") {
        DesignerHierarchyFixture x;
        x.move(12, 0);
        x.drain();
        REQUIRE(x.changes.count() == 6);
        CHECK(x.f.live(12)->get_parent() == nullptr);
        CHECK(x.f.live(13)->get_parent() == x.f.live(12));
        x.check_properties(12);
        x.check_properties(13);
    }

    TEST_CASE("CRI016 RealmDesignerPanel: absent authored transform does not suppress descendant synchronization") {
        DesignerHierarchyFixture x;
        auto* child = x.f.c.ctx.editor_realm_manager->get_archetype(id(12));
        REQUIRE(child);
        child->elements.clear();
        x.move(11, 14);
        x.drain();
        REQUIRE(x.changes.count() == 6);
        x.check_properties(11);
        x.check_properties(13);
        for (size_t i = 0; i < x.changes.count(); ++i)
            CHECK(x.changes.at(i).entity_id != id(12));
    }

    TEST_CASE("CRI016 RealmDesignerPanel: live only descendants do not publish nonexistent authored components") {
        DesignerHierarchyFixture x;
        x.f.add(16);
        REQUIRE(x.f.live(16));
        x.f.live(16)->set_parent(x.f.live(13));
        x.move(11, 14);
        x.drain();
        REQUIRE(x.changes.count() == 9);
        CHECK(x.f.live(16)->get_parent() == x.f.live(13));
        for (size_t i = 0; i < x.changes.count(); ++i)
            CHECK(x.changes.at(i).entity_id != id(16));
    }

    TEST_CASE("CRI016 RealmDesignerPanel: descendant target cannot cause preview cycle") {
        DesignerHierarchyFixture x;
        x.move(11, 13);
        CHECK(x.f.live(11)->get_parent() == nullptr);
        CHECK(x.f.live(12)->get_parent() == x.f.live(11));
        CHECK(x.f.live(13)->get_parent() == x.f.live(12));
        CHECK(x.f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI016 RealmDesignerPanel: live mode ignores authored hierarchy notification") {
        DesignerHierarchyFixture x;
        x.f.c.ctx.data_mode = EditorDataMode::Live;
        x.move(11, 14);
        CHECK(x.f.live(11)->get_parent() == nullptr);
        CHECK(x.f.live(12)->get_parent() == x.f.live(11));
        CHECK(x.f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI016 RealmDesignerPanel: missing model service prevents preview reparent") {
        DesignerHierarchyFixture x;
        auto manager = std::move(x.f.c.ctx.editor_realm_manager);
        x.move(11, 14);
        x.f.c.ctx.editor_realm_manager = std::move(manager);
        CHECK(x.f.live(11)->get_parent() == nullptr);
        CHECK(x.f.c.events.is_queue_empty());
    }

    TEST_CASE("CRI016 RealmDesignerPanel: missing event service prevents unsynchronized preview reparent") {
        DesignerHierarchyFixture x;
        x.f.c.ctx.event_manager = nullptr;
        x.move(11, 14);
        x.f.c.ctx.event_manager = &x.f.c.events;
        CHECK(x.f.live(11)->get_parent() == nullptr);
        CHECK(x.f.c.events.is_queue_empty());
    }
}

namespace {
using namespace EditorCri;
struct ColliderDrawFixture {
    DesignerViewportFixture v;
    ColliderDrawFixture() { v.f.c.ctx.grid_settings.snap_enabled = false; }
    Entity* add(uint64_t n) {
        auto* e = v.f.c.ctx.preview_realm->create_entity(id(n), "Collider");
        REQUIRE(e);
        auto* c = e->get_element<BoxCollider>();
        REQUIRE(c);
        c->set_visibility(true);
        c->set_size(Vector3(2, 4, 6));
        return e;
    }
    void render() {
        v.f.panel.on_render();
        CHECK(v.f.c.renderer.begins == v.f.c.renderer.ends);
    }
    auto& boxes() { return v.f.c.renderer.wire_boxes; }
};
}
TEST_SUITE("EditorCRI017_RealmDesignerPanel") {
    using namespace EditorCri;

    TEST_CASE("CRI017 RealmDesignerPanel: sync callback enqueues work for the next GUI frame") {
        DesignerViewportFixture v;
        std::vector<int> calls;
        v.f.c.ctx.sync_queue.push_back([&] {
            calls.push_back(1);
            v.f.c.ctx.sync_queue.push_back([&] { calls.push_back(3); });
        });
        v.f.c.ctx.sync_queue.push_back([&] { calls.push_back(2); });
        v.frame();
        CHECK((calls == std::vector<int>{1, 2}));
        REQUIRE(v.f.c.ctx.sync_queue.size() == 1);
        v.frame();
        CHECK((calls == std::vector<int>{1, 2, 3}));
        CHECK(v.f.c.ctx.sync_queue.empty());
    }

    TEST_CASE("CRI017 RealmDesignerPanel: sync callback can append a large next-frame batch") {
        DesignerViewportFixture v;
        int calls = 0;
        v.f.c.ctx.sync_queue.push_back([&] {
            for (int i = 0; i < 64; ++i)
                v.f.c.ctx.sync_queue.push_back([&] { ++calls; });
        });
        v.frame();
        CHECK(calls == 0);
        REQUIRE(v.f.c.ctx.sync_queue.size() == 64);
        v.frame();
        CHECK(calls == 64);
        CHECK(v.f.c.ctx.sync_queue.empty());
    }

    TEST_CASE("CRI017 RealmDesignerPanel: clearing newly queued sync work preserves current batch") {
        DesignerViewportFixture v;
        int calls = 0;
        v.f.c.ctx.sync_queue.push_back([&] {
            v.f.c.ctx.sync_queue.push_back([&] { calls += 100; });
            v.f.c.ctx.sync_queue.clear();
        });
        v.f.c.ctx.sync_queue.push_back([&] { ++calls; });
        v.frame();
        CHECK(calls == 1);
        v.frame();
        CHECK(calls == 1);
        CHECK(v.f.c.ctx.sync_queue.empty());
    }

    TEST_CASE("CRI017 RealmDesignerPanel: nested sync waits while data mode is live") {
        DesignerViewportFixture v;
        int calls = 0;
        v.f.c.ctx.sync_queue.push_back([&] {
            ++calls;
            v.f.c.ctx.sync_queue.push_back([&] { ++calls; });
        });
        v.frame();
        REQUIRE(calls == 1);
        v.f.c.ctx.data_mode = EditorDataMode::Live;
        v.frame();
        CHECK(calls == 1);
        REQUIRE(v.f.c.ctx.sync_queue.size() == 1);
        v.f.c.ctx.data_mode = EditorDataMode::Yaml;
        v.frame();
        CHECK(calls == 2);
        CHECK(v.f.c.ctx.sync_queue.empty());
    }

    TEST_CASE("CRI017 RealmDesignerPanel: visible collider submits full size and green debug color") {
        ColliderDrawFixture f;
        f.add(21);
        f.render();
        REQUIRE(f.boxes().size() == 1);
        const auto& b = f.boxes()[0];
        CHECK(b.matrix[0][0] == doctest::Approx(2));
        CHECK(b.matrix[1][1] == doctest::Approx(4));
        CHECK(b.matrix[2][2] == doctest::Approx(6));
        CHECK(b.color == Color(0, 1, 0, 1));
    }

    TEST_CASE("CRI017 RealmDesignerPanel: collider box composes world translation and scale") {
        ColliderDrawFixture f;
        auto* t = f.add(21)->get_transform();
        REQUIRE(t);
        t->set_position(3, 4, 5);
        t->set_scale(2, 3, 4);
        f.render();
        REQUIRE(f.boxes().size() == 1);
        const auto& m = f.boxes()[0].matrix;
        CHECK(m[0][0] == doctest::Approx(4));
        CHECK(m[1][1] == doctest::Approx(12));
        CHECK(m[2][2] == doctest::Approx(24));
        CHECK(m[3][0] == doctest::Approx(3));
        CHECK(m[3][1] == doctest::Approx(4));
        CHECK(m[3][2] == doctest::Approx(5));
    }

    TEST_CASE("CRI017 RealmDesignerPanel: hidden collider is excluded from debug drawing") {
        ColliderDrawFixture f;
        f.add(21)->get_element<BoxCollider>()->set_visibility(false);
        f.add(22)->get_transform()->set_position(7, 0, 0);
        f.render();
        REQUIRE(f.boxes().size() == 1);
        CHECK(f.boxes()[0].matrix[3][0] == doctest::Approx(7));
    }

    TEST_CASE("CRI017 RealmDesignerPanel: purged preview entity submits no collider box") {
        ColliderDrawFixture f;
        f.add(21)->simple_purge();
        f.add(22)->get_transform()->set_position(8, 0, 0);
        f.render();
        REQUIRE(f.boxes().size() == 1);
        CHECK(f.boxes()[0].matrix[3][0] == doctest::Approx(8));
    }

    TEST_CASE("CRI017 RealmDesignerPanel: live mode does not draw authored preview collider boxes") {
        ColliderDrawFixture f;
        f.add(21);
        Realm live("Live");
        f.v.f.c.ctx.active_realm = &live;
        f.v.f.c.ctx.data_mode = EditorDataMode::Live;
        f.render();
        CHECK(f.boxes().empty());
        CHECK(f.v.f.c.renderer.begins == 1);
        f.v.f.c.ctx.active_realm = nullptr;
    }

    TEST_CASE("CRI017 RealmDesignerPanel: collider debug drawing uses updated live size on next render") {
        ColliderDrawFixture f;
        auto* c = f.add(21)->get_element<BoxCollider>();
        f.render();
        REQUIRE(f.boxes().size() == 1);
        f.boxes().clear();
        c->set_size(Vector3(1, 3, 5));
        f.render();
        REQUIRE(f.boxes().size() == 1);
        CHECK(f.boxes()[0].matrix[0][0] == doctest::Approx(1));
        CHECK(f.boxes()[0].matrix[1][1] == doctest::Approx(3));
        CHECK(f.boxes()[0].matrix[2][2] == doctest::Approx(5));
    }
}

#include <Editor/events/EntitySelectedEvent.h>
namespace {
using namespace EditorCri;
struct DesignerInputFixture : PickingFixture {
    const bool previous_block = EntitySelectedEvent::block_selection;
    DesignerInputFixture() {
        v.camera.set_projection_mode(ProjectionMode::Orthographic);
        v.f.c.renderer.framebuffer_texture = ImTextureID(77);
        EntitySelectedEvent::block_selection = false;
    }
    ~DesignerInputFixture() { EntitySelectedEvent::block_selection = previous_block; }
    void show() {
        v.frame(); v.frame();
        auto* w = ImGui::FindWindowByName("Realm Designer");
        REQUIRE(w);
        ImGui::FocusWindow(w);
        v.frame();
        REQUIRE(contains_designer_texture(77));
    }
    void focus_key() {
        ImGui::GetIO().AddKeyEvent(ImGuiKey_F, true);
        v.frame();
        ImGui::GetIO().AddKeyEvent(ImGuiKey_F, false);
        v.frame();
    }
    glm::vec3 position() { return v.camera.get_transform()->get_position().to_glm(); }
    void select(uint64_t n) { v.f.c.ctx.selected_entity_id = id(n); }
    void hover_image() {
        auto* w = ImGui::FindWindowByName("Realm Designer");
        REQUIRE(w);
        const auto size = v.f.panel.get_viewport_size();
        auto p = w->DC.CursorMaxPos;
        ImGui::GetIO().AddMousePosEvent(p.x - size.x * .5f, p.y - size.y * .5f);
        v.frame();
        REQUIRE(v.gui.current->HoveredWindow == w);
    }
};
}
TEST_SUITE("EditorCRI018_RealmDesignerPanel") {
    using namespace EditorCri;

    TEST_CASE("CRI018 RealmDesignerPanel: actual focus shortcut places camera three units from selected preview entity") {
        DesignerInputFixture f;
        f.add(11, 2, 3, 0);
        f.select(11);
        f.show();
        f.focus_key();
        auto target = f.v.f.live(11)->get_transform()->get_world_position().to_glm();
        CHECK(glm::length(f.position() - target) == doctest::Approx(3));
        CHECK(f.selected() == id(11));
    }

    TEST_CASE("CRI018 RealmDesignerPanel: focus shortcut suppressed while editing a property") {
        DesignerInputFixture f;
        f.add(11, 2, 3, 0);
        f.select(11);
        f.v.f.c.ctx.is_editing_property = true;
        f.show();
        const auto before = f.position();
        f.focus_key();
        CHECK(f.position() == before);
    }

    TEST_CASE("CRI018 RealmDesignerPanel: focus shortcut suppressed while locked") {
        DesignerInputFixture f;
        f.add(11, 2, 3, 0);
        f.select(11);
        f.v.f.panel.lock();
        f.show();
        const auto before = f.position();
        f.focus_key();
        CHECK(f.position() == before);
    }

    TEST_CASE("CRI018 RealmDesignerPanel: focus shortcut with missing selected preview leaves camera intact") {
        DesignerInputFixture f;
        f.v.f.c.ctx.editor_realm_manager->add_entity(entity(11, "Missing preview"));
        f.select(11);
        f.show();
        const auto before = f.position();
        f.focus_key();
        CHECK(f.position() == before);
        CHECK(f.selected() == id(11));
    }

    TEST_CASE("CRI018 RealmDesignerPanel: actual viewport left click picks central authored collider") {
        DesignerInputFixture f;
        f.add(11, 0, 0, 0);
        f.show();
        f.hover_image();
        ImGui::GetIO().AddMouseButtonEvent(0, true);
        f.v.frame();
        CHECK(f.selected() == id(11));
        CHECK(f.v.f.c.ctx.selected_entity == f.v.f.live(11));
        ImGui::GetIO().AddMouseButtonEvent(0, false);
        f.v.frame();
    }

    TEST_CASE("CRI018 RealmDesignerPanel: locked viewport releases previous gizmo selection block") {
        DesignerInputFixture f;
        f.v.f.panel.lock();
        EntitySelectedEvent::block_selection = true;
        f.v.frame();
        CHECK_FALSE(EntitySelectedEvent::block_selection);
    }

    TEST_CASE("CRI018 RealmDesignerPanel: hidden viewport releases previous gizmo selection block") {
        DesignerInputFixture f;
        f.v.f.panel.set_visibility(false);
        EntitySelectedEvent::block_selection = true;
        f.v.frame();
        CHECK_FALSE(EntitySelectedEvent::block_selection);
    }

    TEST_CASE("CRI018 RealmDesignerPanel: missing selected archetype releases previous gizmo selection block") {
        DesignerInputFixture f;
        f.select(99);
        EntitySelectedEvent::block_selection = true;
        f.v.frame();
        CHECK_FALSE(EntitySelectedEvent::block_selection);
    }

    TEST_CASE("CRI018 RealmDesignerPanel: missing framebuffer texture releases previous gizmo selection block") {
        DesignerInputFixture f;
        f.v.f.c.renderer.framebuffer_texture = 0;
        EntitySelectedEvent::block_selection = true;
        f.v.frame();
        CHECK_FALSE(EntitySelectedEvent::block_selection);
    }

    TEST_CASE("CRI018 RealmDesignerPanel: live mode releases authored gizmo selection block") {
        DesignerInputFixture f;
        f.v.f.c.ctx.data_mode = EditorDataMode::Live;
        EntitySelectedEvent::block_selection = true;
        f.v.frame();
        CHECK_FALSE(EntitySelectedEvent::block_selection);
    }

    TEST_CASE("CRI018 RealmDesignerPanel: orthographic collider without transform uses identity placement") {
        PickingFixture f;
        f.add(11, 0, 0, 0);
        auto* a = f.v.f.c.ctx.editor_realm_manager->get_archetype(id(11));
        REQUIRE(a);
        a->elements.erase(a->elements.begin());
        f.pick(ProjectionMode::Orthographic);
        CHECK(f.selected() == id(11));
    }

    TEST_CASE("CRI018 RealmDesignerPanel: perspective collider without transform is skipped") {
        PickingFixture f;
        f.add(11, 0, 0, 0);
        auto* a = f.v.f.c.ctx.editor_realm_manager->get_archetype(id(11));
        REQUIRE(a);
        a->elements.erase(a->elements.begin());
        f.pick(ProjectionMode::Perspective);
        CHECK_FALSE(f.selected().is_valid());
    }
}

namespace {
using namespace EditorCri;
void activate_designer_lock(DesignerInputFixture& f) {
    auto* window = ImGui::FindWindowByName("Realm Designer");
    REQUIRE(window);
    ImGui::ActivateItemByID(window->GetID("##PanelLockBtn"));
    f.v.frame();
}
}
TEST_SUITE("EditorCRI018_RealmDesignerToolbar") {
    TEST_CASE("CRI018 RealmDesignerPanel: actual toolbar lock button locks viewport") {
        DesignerInputFixture f;
        f.show();
        CHECK_FALSE(f.v.f.panel.is_locked());
        activate_designer_lock(f);
        CHECK(f.v.f.panel.is_locked());
        CHECK(f.v.f.panel.get_visibility());
    }
    TEST_CASE("CRI018 RealmDesignerPanel: actual toolbar lock button unlocks viewport") {
        DesignerInputFixture f;
        f.v.f.panel.lock();
        f.show();
        CHECK(f.v.f.panel.is_locked());
        activate_designer_lock(f);
        CHECK_FALSE(f.v.f.panel.is_locked());
        CHECK(f.v.f.panel.get_visibility());
    }
}

#include <ImGuizmo/ImGuizmo.h>
namespace {
struct DesignerToolbarFixture : DesignerInputFixture {
    void activate(const char* label) {
        auto* w=ImGui::FindWindowByName("Realm Designer");
        REQUIRE(w);
        ImGui::ActivateItemByID(w->GetID(label));
        v.frame();
    }
    int operation() const { return v.f.panel.get_gizmo_operation_for_testing(); }
    void shortcut(ImGuiKey key) {
        ImGui::GetIO().AddKeyEvent(key,true);v.frame();
        ImGui::GetIO().AddKeyEvent(key,false);v.frame();
    }
};
}
TEST_SUITE("EditorCRI022_RealmDesignerPanel") {
    TEST_CASE("CRI022 RealmDesignerPanel: actual Rotate radio button selects rotation gizmo") {
        DesignerToolbarFixture f;f.show();
        REQUIRE(f.operation()==ImGuizmo::TRANSLATE);
        f.activate("Rotate");
        CHECK(f.operation()==ImGuizmo::ROTATE);
        CHECK_FALSE(f.v.f.panel.is_locked());
    }
    TEST_CASE("CRI022 RealmDesignerPanel: actual Scale radio button selects scale gizmo") {
        DesignerToolbarFixture f;f.show();
        f.activate("Scale");
        CHECK(f.operation()==ImGuizmo::SCALE);
    }
    TEST_CASE("CRI022 RealmDesignerPanel: actual Translate radio button restores translation gizmo") {
        DesignerToolbarFixture f;f.show();
        f.activate("Rotate");REQUIRE(f.operation()==ImGuizmo::ROTATE);
        f.activate("Translate");
        CHECK(f.operation()==ImGuizmo::TRANSLATE);
    }
    TEST_CASE("CRI022 RealmDesignerPanel: hovered viewport keyboard shortcuts select all three operations") {
        DesignerToolbarFixture f;f.show();f.hover_image();
        f.shortcut(ImGuiKey_E);CHECK(f.operation()==ImGuizmo::ROTATE);
        f.shortcut(ImGuiKey_R);CHECK(f.operation()==ImGuizmo::SCALE);
        f.shortcut(ImGuiKey_W);CHECK(f.operation()==ImGuizmo::TRANSLATE);
    }
    TEST_CASE("CRI022 RealmDesignerPanel: keyboard shortcuts cannot change gizmo while editing property") {
        DesignerToolbarFixture f;f.show();f.hover_image();
        f.v.f.c.ctx.is_editing_property=true;
        f.shortcut(ImGuiKey_E);CHECK(f.operation()==ImGuizmo::TRANSLATE);
    }
    TEST_CASE("CRI022 RealmDesignerPanel: locked viewport rejects gizmo keyboard shortcut") {
        DesignerToolbarFixture f;f.show();f.hover_image();
        f.v.f.panel.lock();
        f.shortcut(ImGuiKey_R);CHECK(f.operation()==ImGuizmo::TRANSLATE);
    }
}
