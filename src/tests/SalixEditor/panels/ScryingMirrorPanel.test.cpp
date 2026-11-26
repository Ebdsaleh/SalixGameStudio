// Tests for Editor/panels/ScryingMirrorPanel.
#include "../fixtures/HeadlessGuiFixtures.h"
#include <Editor/panels/ScryingMirrorPanel.h>
#include <Editor/events/EntitySelectedEvent.h>
#include <Editor/events/ElementSelectedEvent.h>
#include <Salix/events/sdl/SDLEvent.h>
namespace {
using namespace EditorCri;
struct MirrorProbe:ScryingMirrorPanel {void content(){on_panel_gui_update();} ImGuiWindowFlags flags()const{return get_window_flags();}};
struct MirrorFixture {PanelContext c; MirrorProbe panel; TempRealm file;
    MirrorFixture(){c.ctx.editor_realm_manager->load_realm_from_file(file.write("Entities: [{name: Root, id: 11, elements: {Transform: {id: 12, name: Local}}}]"));panel.initialize(&c.ctx);}
    void select(uint64_t n){EntitySelectedEvent e(id(n));panel.on_event(e);}
    void select_element(uint64_t n,uint64_t owner){ElementSelectedEvent e(id(n),id(owner));panel.on_event(e);}
};
}
TEST_SUITE("EditorCRI002_ScryingMirrorPanel") {
    TEST_CASE("CRI002 ScryingMirrorPanel: constructor identity and derived flags") {
        MirrorProbe p; CHECK(p.get_name()=="Scrying Mirror Panel"); CHECK(p.get_title()=="Scrying Mirror"); CHECK(p.is_panel_derived()); CHECK(p.flags()==ImGuiWindowFlags_None);
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: null context leaves hidden state unchanged") {
        MirrorProbe p; p.set_visibility(false); p.initialize(nullptr); CHECK_FALSE(p.get_visibility());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: initialize subscribes and restores visibility") {
        MirrorFixture f; CHECK(f.panel.get_visibility()); EntitySelectedEvent e(id(11)); f.c.events.dispatch(e); f.c.events.process_queue(); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11));
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: entity selection clears prior element") {
        MirrorFixture f; f.select_element(12,11); f.select(11); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11)); CHECK_FALSE(f.panel.get_selected_element_id_for_testing().is_valid());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: element selection keeps owner and element") {
        MirrorFixture f; f.select_element(12,11); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11)); CHECK(f.panel.get_selected_element_id_for_testing()==id(12));
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: entity deselection clears both IDs") {
        MirrorFixture f; f.select_element(12,11); f.select(0); CHECK_FALSE(f.panel.get_selected_entity_id_for_testing().is_valid()); CHECK_FALSE(f.panel.get_selected_element_id_for_testing().is_valid());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: element deselection clears both IDs") {
        MirrorFixture f; f.select_element(12,11); f.select_element(0,11); CHECK_FALSE(f.panel.get_selected_entity_id_for_testing().is_valid()); CHECK_FALSE(f.panel.get_selected_element_id_for_testing().is_valid());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: unrelated event leaves selection intact") {
        MirrorFixture f; f.select_element(12,11); WindowResizeEvent e(9,9); f.panel.on_event(e); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11)); CHECK(f.panel.get_selected_element_id_for_testing()==id(12));
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: unselected inspector draws without changing realm") {
        GuiFrame g; MirrorFixture f; CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.deferred_commands.empty()); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI002 ScryingMirrorPanel: selected transform inspector renders without input mutation") {
        GuiFrame g; MirrorFixture f; f.select_element(12,11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(g.draw([&]{f.panel.content();})>0); auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a); CHECK(a->elements[0].data["name"].as<std::string>()=="Local"); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }
}

TEST_SUITE("EditorCRI005_ScryingMirrorPanel") {
    TEST_CASE("CRI005 ScryingMirrorPanel: unregistered component can be inspected without changing data") {
        GuiFrame gui; MirrorFixture f;
        auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a);
        a->elements.push_back(element("CRI005_Unknown",13,"{name: Unsupported}"));
        f.select_element(13,11); auto before=YAML::Dump(a->elements.back().data);
        CHECK(gui.draw([&]{f.panel.content();})>0);
        CHECK(YAML::Dump(a->elements.back().data)==before);
        CHECK(f.c.ctx.deferred_commands.empty());
    }
    TEST_CASE("CRI005 ScryingMirrorPanel: absent realm manager suppresses inspector safely") {
        GuiFrame gui; MirrorFixture f; f.c.ctx.editor_realm_manager.reset();
        f.select(11); CHECK(gui.draw([&]{f.panel.content();})==0);
        CHECK(f.c.ctx.deferred_commands.empty());
    }
}



TEST_SUITE("EditorCRI006_ScryingMirrorPanel") {
    TEST_CASE("CRI006 ScryingMirrorPanel: whole entity inspector preserves selection and source") {
        GuiFrame g; MirrorFixture f; f.select(11); auto before=YAML::Dump(f.c.ctx.editor_realm_manager->get_archetype(id(11))->elements[0].data); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11)); CHECK(YAML::Dump(f.c.ctx.editor_realm_manager->get_archetype(id(11))->elements[0].data)==before);
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: stale entity selection renders fallback") {
        GuiFrame g; MirrorFixture f; f.select(99); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.deferred_commands.empty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: stale element selection does not edit other element") {
        GuiFrame g; MirrorFixture f; f.select_element(99,11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: live mode avoids YAML inspector mutations") {
        GuiFrame g; MirrorFixture f; f.select(11); f.c.ctx.data_mode=EditorDataMode::Live; CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.deferred_commands.empty()); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: hidden content is suppressed") {
        GuiFrame g; MirrorFixture f; f.select(11); f.panel.set_visibility(false); CHECK(g.draw([&]{f.panel.content();})==0); CHECK(f.panel.get_selected_entity_id_for_testing()==id(11));
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: box collider inspector without sprite remains unchanged") {
        GuiFrame g; MirrorFixture f; auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a); a->elements.push_back(element("BoxCollider",13,"{name: Bounds}")); a->elements.back().owner_id=a->id; f.c.ctx.editor_realm_manager->take_snapshot(); f.select_element(13,11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: collider with authored sprite dimensions draws resize controls") {
        GuiFrame g; MirrorFixture f; auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a); a->elements.push_back(element("BoxCollider",13,"{name: Bounds}")); a->elements.back().owner_id=a->id; a->elements.push_back(element("Sprite2D",14,"{name: Image, width: 64, height: 32}")); a->elements.back().owner_id=a->id; f.select_element(13,11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.deferred_commands.empty()); CHECK(a->elements.back().data["width"].as<int>()==64);
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: whole inspector skips unknown components and continues") {
        GuiFrame g; MirrorFixture f; auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a); a->elements.insert(a->elements.begin(),element("CRI006Unknown",13)); f.select(11); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(a->elements.size()==2); CHECK(f.c.ctx.deferred_commands.empty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: locked inspector renders without writing") {
        GuiFrame g; MirrorFixture f; f.select(11); f.panel.lock(); for(int i=0;i<2;++i)g.frame([&]{ImGui::SetNextWindowSize(ImVec2(600,400),ImGuiCond_Always); f.panel.on_gui_update();}); CHECK(f.panel.is_locked()); CHECK(g.current->DisabledStackSize==0); CHECK_FALSE(f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI006 ScryingMirrorPanel: switching selection between entities preserves both archetypes") {
        GuiFrame g; MirrorFixture f; auto a=entity(20,"Second"); a.elements.push_back(element("Transform",21,"{name: Other}")); f.c.ctx.editor_realm_manager->add_entity(a); f.select(11); CHECK(g.draw([&]{f.panel.content();})>0); f.select_element(21,20); CHECK(g.draw([&]{f.panel.content();})>0); CHECK(f.c.ctx.editor_realm_manager->get_archetype(id(20))->elements[0].name=="Transform"); CHECK(f.panel.get_selected_element_id_for_testing()==id(21));
    }
}

#include <Salix/gui/DialogBox.h>
#include <Salix/gui/IGui.h>
#include <Salix/reflection/PropertyHandleYaml.h>
#include <Editor/events/PropertyValueChangedEvent.h>
namespace {
using namespace EditorCri;
struct MediaFixture {
    MirrorFixture f; TempRealm temp;
    std::filesystem::path old_root=Salix::g_project_root_path;
    struct Listener:IEventListener {
        int calls=0; SimpleGuid owner=SimpleGuid::invalid(),component=SimpleGuid::invalid();
        PropertyValue value=0; bool reload=false; std::string name;
        void on_event(IEvent& event) override {
            if(event.get_event_type()!=EventType::EditorPropertyValueChanged)return;
            auto& e=static_cast<PropertyValueChangedEvent&>(event);
            ++calls; owner=e.entity_id; component=e.element_id; value=e.new_value; reload=e.requires_reload;name=e.property_name;
        }
    } listener;
    MediaFixture(){
        Salix::g_project_root_path=std::filesystem::path(temp.path()).parent_path();
        auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11)); REQUIRE(a);
        auto e=element("Sprite2D",21,"{texture_path: old.png, width: 80, height: 40}");e.owner_id=a->id;a->elements.push_back(e);
        f.c.events.subscribe(EventCategory::Editor,&listener); f.select_element(21,11);
        for(std::string key:{"SelectImageFile","SelectAudioFile","SelectSourceFile","SelectFile"}){
            std::string title=key; REQUIRE(f.c.gui.create_dialog(key,title,Salix::DialogType::File,false));
        }
    }
    ~MediaFixture(){Salix::g_project_root_path=old_root;}
    ElementArchetype* sprite(){auto* a=f.c.ctx.editor_realm_manager->get_archetype(id(11));return a?a->get_element_by_id(id(21)):nullptr;}
    Salix::DialogBox* choose(UIHint hint,const char* key){
        auto* s=sprite();REQUIRE(s);Property p{};p.name="texture_path";p.type=PropertyType::String;p.hint=hint;
        PropertyHandleYaml h(p,&s->data);
        f.panel.select_media_for_testing(h,s,ByteMirror::get_type_info_by_name("Sprite2D"));
        CHECK(f.c.gui.shown_dialog==key);auto* d=f.c.gui.get_dialog(key);REQUIRE(d);return d;
    }
    void result(Salix::DialogBox* d,bool ok){Salix::FileDialogResult r{};r.is_ok=ok;r.file_path_name=temp.write("fixture");d->get_callback()(r);}
    void drain(){auto& q=f.c.ctx.deferred_commands;for(auto& cmd:q)cmd();q.clear();f.c.events.process_queue();}
};
}
TEST_SUITE("EditorCRI009_ScryingMirrorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI009 ScryingMirrorPanel: image selector configures image filters") {
        MediaFixture m; auto* d=m.choose(UIHint::ImageFile,"SelectImageFile"); CHECK(d->get_filters().find(".png")!=std::string::npos); CHECK(d->get_default_path()==Salix::g_project_root_path.string());
    }

    TEST_CASE("CRI009 ScryingMirrorPanel: audio selector configures audio filters") {
        MediaFixture m; auto* d=m.choose(UIHint::AudioFile,"SelectAudioFile"); CHECK(d->get_filters().find(".wav")!=std::string::npos);
    }

    TEST_CASE("CRI009 ScryingMirrorPanel: source selector configures source filters") {
        MediaFixture m; auto* d=m.choose(UIHint::SourceFile,"SelectSourceFile"); CHECK(d->get_filters().find(".cpp")!=std::string::npos);
    }

    TEST_CASE("CRI009 ScryingMirrorPanel: generic selector configures all files") {
        MediaFixture m; auto* d=m.choose(UIHint::FilePath,"SelectFile"); CHECK(d->get_filters().find("All Files")!=std::string::npos);
    }

    TEST_CASE("CRI009 ScryingMirrorPanel: cancelled dialog does not queue mutation") {
        MediaFixture m; auto* d=m.choose(UIHint::ImageFile,"SelectImageFile"); m.result(d,false); CHECK(m.f.c.ctx.deferred_commands.empty()); CHECK(m.listener.calls==0);
    }

    TEST_CASE("CRI009 ScryingMirrorPanel: accepted file applies to original owner despite changed selection") {
        MediaFixture m; auto* d=m.choose(UIHint::ImageFile,"SelectImageFile"); m.f.select(99); m.result(d,true); REQUIRE(m.f.c.ctx.deferred_commands.size()==1); m.drain(); CHECK(m.listener.calls==1); CHECK(m.listener.owner==id(11)); CHECK(m.listener.component==id(21)); CHECK(m.listener.reload); CHECK(m.listener.name=="texture_path"); CHECK(std::get<std::string>(m.listener.value)=="fixture.realm");
    }

    TEST_CASE("CRI009 ScryingMirrorPanel: removed component invalidates deferred file selection safely") {
        MediaFixture m; auto* d=m.choose(UIHint::ImageFile,"SelectImageFile"); m.result(d,true); m.f.c.ctx.editor_realm_manager->purge_element(id(11),id(21)); m.drain(); CHECK(m.listener.calls==0); CHECK(m.sprite()==nullptr);
    }

    TEST_CASE("CRI009 ScryingMirrorPanel: collider resize computes world units and preserves depth") {
        MediaFixture m; auto c=element("BoxCollider",22,"{size: {x: 1,y: 1,z: 2}}"); c.owner_id=id(11); m.f.panel.resize_collider_for_testing(&c,m.sprite()); m.f.c.events.process_queue(); REQUIRE(m.listener.calls==1); auto v=std::get<Vector3>(m.listener.value); CHECK(v.x==doctest::Approx(.8)); CHECK(v.y==doctest::Approx(.4)); CHECK(v.z==2); CHECK(m.listener.component==id(22));
    }

    TEST_CASE("CRI009 ScryingMirrorPanel: collider resize falls back for invalid pixel scale and clamps depth") {
        MediaFixture m; m.f.c.renderer.ppu=0; auto c=element("BoxCollider",22,"{size: {x: 1,y: 1,z: 0}}"); m.f.panel.resize_collider_for_testing(&c,m.sprite()); m.f.c.events.process_queue(); REQUIRE(m.listener.calls==1); auto v=std::get<Vector3>(m.listener.value); CHECK(v.x==doctest::Approx(.8)); CHECK(v.z==doctest::Approx(.1));
    }

    TEST_CASE("CRI009 ScryingMirrorPanel: collider resize ignores missing or zero dimensions") {
        MediaFixture m; auto c=element("BoxCollider",22,"{size: {x: 1,y: 1,z: 1}}"); auto s=element("Sprite2D",21,"{width: 0, height: 40}"); m.f.panel.resize_collider_for_testing(&c,&s); m.f.panel.resize_collider_for_testing(&c,nullptr); m.f.c.events.process_queue(); CHECK(m.listener.calls==0);
    }
}

#include <Salix/ecs/Camera.h>
namespace {
using namespace EditorCri;
struct InspectorCameraFixture {
    MirrorFixture f;
    InspectorCameraFixture(){f.c.ctx.preview_realm=std::make_unique<Realm>("Preview");}
    Camera* camera(uint64_t n){auto* e=f.c.ctx.preview_realm->create_entity(id(n),"Camera");auto* c=e->add_element<Camera>();REQUIRE(c);return c;}
    void activate(uint64_t n,bool value=true){auto e=element("Camera",201,"{active: false}");e.owner_id=id(n);e.data["active"]=value;Property p{};p.name="active";p.type=PropertyType::Bool;PropertyHandleYaml h(p,&e.data);f.panel.activate_camera_for_testing(e,h);}
    SimpleGuid main(){return f.c.ctx.preview_realm->get_main_camera_entity_id();}
};
}
TEST_SUITE("EditorCRI011_ScryingMirrorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI011 ScryingMirrorPanel: activation sets main live camera identity") {
        InspectorCameraFixture f;auto* c=f.camera(11);c->deactivate();f.activate(11);CHECK(f.main()==id(11));CHECK(c->get_is_active());
    }

    TEST_CASE("CRI011 ScryingMirrorPanel: activating second camera deactivates first") {
        InspectorCameraFixture f;auto* a=f.camera(11);auto* b=f.camera(12);f.activate(11);f.activate(12);CHECK_FALSE(a->get_is_active());CHECK(b->get_is_active());CHECK(f.main()==id(12));
    }

    TEST_CASE("CRI011 ScryingMirrorPanel: activation ignores entities without camera") {
        InspectorCameraFixture f;auto* a=f.camera(11);f.activate(11);f.f.c.ctx.preview_realm->create_entity(id(12),"Other");f.activate(12);CHECK(f.main()==id(11));CHECK(a->get_is_active());
    }

    TEST_CASE("CRI011 ScryingMirrorPanel: activation ignores missing live owner") {
        InspectorCameraFixture f;auto* a=f.camera(11);f.activate(11);f.activate(99);CHECK(f.main()==id(11));CHECK(a->get_is_active());
    }

    TEST_CASE("CRI011 ScryingMirrorPanel: false activation does not replace existing main camera") {
        InspectorCameraFixture f;auto* a=f.camera(11);f.camera(12);f.activate(11);f.activate(12,false);CHECK(f.main()==id(11));CHECK(a->get_is_active());
    }

    TEST_CASE("CRI011 ScryingMirrorPanel: absent preview realm is safe") {
        InspectorCameraFixture f;f.f.c.ctx.preview_realm.reset();CHECK_NOTHROW(f.activate(11));CHECK(f.f.c.ctx.preview_realm==nullptr);
    }

    TEST_CASE("CRI011 ScryingMirrorPanel: camera activation deactivates additional active cameras") {
        InspectorCameraFixture f;auto* a=f.camera(11);auto* b=f.camera(12);auto* c=f.camera(13);a->activate();b->activate();f.activate(13);CHECK_FALSE(a->get_is_active());CHECK_FALSE(b->get_is_active());CHECK(c->get_is_active());
    }

    TEST_CASE("CRI011 ScryingMirrorPanel: same camera can be selected twice without losing active state") {
        InspectorCameraFixture f;auto* a=f.camera(11);f.activate(11);f.activate(11);CHECK(a->get_is_active());CHECK(f.main()==id(11));
    }

    TEST_CASE("CRI011 ScryingMirrorPanel: dialog result after panel destruction never queues an update") {
        MediaFixture m;auto p=std::make_unique<ScryingMirrorPanel>();p->initialize(&m.f.c.ctx);Property prop{};prop.name="texture_path";prop.type=PropertyType::String;prop.hint=UIHint::ImageFile;PropertyHandleYaml h(prop,&m.sprite()->data);p->select_media_for_testing(h,m.sprite(),ByteMirror::get_type_info_by_name("Sprite2D"));auto* d=m.f.c.gui.get_dialog("SelectImageFile");REQUIRE(d);p.reset();m.result(d,true);CHECK(m.f.c.ctx.deferred_commands.empty());
    }

    TEST_CASE("CRI011 ScryingMirrorPanel: deferred media callback after panel destruction is inert") {
        MediaFixture m;auto p=std::make_unique<ScryingMirrorPanel>();p->initialize(&m.f.c.ctx);Property prop{};prop.name="texture_path";prop.type=PropertyType::String;prop.hint=UIHint::ImageFile;PropertyHandleYaml h(prop,&m.sprite()->data);p->select_media_for_testing(h,m.sprite(),ByteMirror::get_type_info_by_name("Sprite2D"));auto* d=m.f.c.gui.get_dialog("SelectImageFile");REQUIRE(d);m.result(d,true);REQUIRE(m.f.c.ctx.deferred_commands.size()==1);p.reset();m.drain();CHECK(m.listener.calls==0);
    }
}

namespace {
using namespace EditorCri;
struct VisibilityFixture {
    MirrorFixture f;
    VisibilityFixture(){f.c.ctx.preview_realm=std::make_unique<Realm>("Preview");f.c.ctx.preview_realm->create_entity(id(11),"Root");}
    auto* model(){return f.c.ctx.editor_realm_manager->get_archetype(id(11));}
    auto* live(){return f.c.ctx.preview_realm->get_entity_by_id(id(11));}
    void queue(bool visible){f.panel.queue_visibility_for_testing(id(11),visible);}
    void drain(){auto q=std::move(f.c.ctx.deferred_commands);f.c.ctx.deferred_commands.clear();for(auto& command:q)command();}
};
}
TEST_SUITE("EditorCRI013_ScryingMirrorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI013 ScryingMirrorPanel: visibility command is deferred until frame processing") {
        VisibilityFixture v;v.queue(false);REQUIRE(v.f.c.ctx.deferred_commands.size()==1);CHECK(v.model()->is_visible);CHECK(v.live()->is_visible());v.drain();CHECK_FALSE(v.model()->is_visible);CHECK_FALSE(v.live()->is_visible());
    }

    TEST_CASE("CRI013 ScryingMirrorPanel: visibility toggle marks saved entity modified") {
        VisibilityFixture v;v.queue(false);v.drain();CHECK(v.model()->state==ArchetypeState::Modified);CHECK(v.f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI013 ScryingMirrorPanel: restored visibility clears dirty state") {
        VisibilityFixture v;v.queue(false);v.drain();v.queue(true);v.drain();CHECK(v.model()->is_visible);CHECK(v.live()->is_visible());CHECK_FALSE(v.f.c.ctx.editor_realm_manager->is_dirty());
    }

    TEST_CASE("CRI013 ScryingMirrorPanel: visibility of unsaved entity preserves New state") {
        VisibilityFixture v;v.model()->state=ArchetypeState::New;v.queue(false);v.drain();CHECK(v.model()->state==ArchetypeState::New);CHECK_FALSE(v.model()->is_visible);
    }

    TEST_CASE("CRI013 ScryingMirrorPanel: reallocated archetype vector does not invalidate queued visibility") {
        VisibilityFixture v;v.queue(false);for(uint64_t n=50;n<90;++n)v.f.c.ctx.editor_realm_manager->add_entity(entity(n));v.drain();CHECK_FALSE(v.model()->is_visible);CHECK_FALSE(v.live()->is_visible());
    }

    TEST_CASE("CRI013 ScryingMirrorPanel: removed owner cannot receive queued visibility update") {
        VisibilityFixture v;v.queue(false);v.f.c.ctx.editor_realm_manager->purge_entity(id(11));v.drain();CHECK(v.model()==nullptr);
    }

    TEST_CASE("CRI013 ScryingMirrorPanel: missing preview realm still updates authored visibility") {
        VisibilityFixture v;v.f.c.ctx.preview_realm.reset();v.queue(false);v.drain();CHECK_FALSE(v.model()->is_visible);
    }

    TEST_CASE("CRI013 ScryingMirrorPanel: changed selection cannot redirect queued visibility") {
        VisibilityFixture v;v.f.c.ctx.editor_realm_manager->add_entity(entity(21));v.queue(false);v.f.select(21);v.drain();CHECK_FALSE(v.model()->is_visible);CHECK(v.f.c.ctx.editor_realm_manager->get_archetype(id(21))->is_visible);
    }

    TEST_CASE("CRI013 ScryingMirrorPanel: destroyed panel cannot execute queued visibility callback") {
        VisibilityFixture v;auto p=std::make_unique<ScryingMirrorPanel>();p->initialize(&v.f.c.ctx);p->queue_visibility_for_testing(id(11),false);REQUIRE(v.f.c.ctx.deferred_commands.size()==1);p.reset();v.drain();CHECK(v.model()->is_visible);CHECK(v.live()->is_visible());
    }

    TEST_CASE("CRI013 ScryingMirrorPanel: uninitialized inspector cannot queue visibility command") {
        ScryingMirrorPanel p;CHECK_NOTHROW(p.queue_visibility_for_testing(id(11),false));
    }
}

namespace {
using namespace EditorCri;
struct ResizeQueueFixture {
    MediaFixture m;
    ResizeQueueFixture(){auto* owner=m.f.c.ctx.editor_realm_manager->get_archetype(id(11));REQUIRE(owner);auto c=element("BoxCollider",22,"{size: {x: 1,y: 1,z: 2}}");c.owner_id=id(11);owner->elements.push_back(c);}
    void queue(){m.f.panel.queue_collider_resize_for_testing(id(11),id(22),id(21));}
    auto* owner(){return m.f.c.ctx.editor_realm_manager->get_archetype(id(11));}
    void drain(){m.drain();}
};
}
TEST_SUITE("EditorCRI014_ScryingMirrorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI014 ScryingMirrorPanel: queued collider resize dispatches only after deferred execution") {
        ResizeQueueFixture x;x.queue();REQUIRE(x.m.f.c.ctx.deferred_commands.size()==1);CHECK(x.m.listener.calls==0);x.drain();REQUIRE(x.m.listener.calls==1);CHECK(x.m.listener.component==id(22));auto v=std::get<Vector3>(x.m.listener.value);CHECK(v.x==doctest::Approx(.8));CHECK(v.y==doctest::Approx(.4));CHECK(v.z==2);
    }

    TEST_CASE("CRI014 ScryingMirrorPanel: collider resize reacquires pointers after component vector grows") {
        ResizeQueueFixture x;x.queue();for(uint64_t n=100;n<150;++n)x.owner()->elements.push_back(element("Transform",n));x.drain();REQUIRE(x.m.listener.calls==1);CHECK(x.m.listener.component==id(22));
    }

    TEST_CASE("CRI014 ScryingMirrorPanel: collider resize reacquires owner after realm vector grows") {
        ResizeQueueFixture x;x.queue();for(uint64_t n=100;n<150;++n)x.m.f.c.ctx.editor_realm_manager->add_entity(entity(n));x.drain();CHECK(x.m.listener.calls==1);CHECK(x.m.listener.owner==id(11));
    }

    TEST_CASE("CRI014 ScryingMirrorPanel: removed target collider cancels queued resize") {
        ResizeQueueFixture x;x.queue();x.m.f.c.ctx.editor_realm_manager->purge_element(id(11),id(22));x.drain();CHECK(x.m.listener.calls==0);
    }

    TEST_CASE("CRI014 ScryingMirrorPanel: removed texture cancels queued resize") {
        ResizeQueueFixture x;x.queue();x.m.f.c.ctx.editor_realm_manager->purge_element(id(11),id(21));x.drain();CHECK(x.m.listener.calls==0);
    }

    TEST_CASE("CRI014 ScryingMirrorPanel: removed owner cancels queued resize") {
        ResizeQueueFixture x;x.queue();x.m.f.c.ctx.editor_realm_manager->purge_entity(id(11));x.drain();CHECK(x.m.listener.calls==0);
    }

    TEST_CASE("CRI014 ScryingMirrorPanel: changed selection cannot redirect resize") {
        ResizeQueueFixture x;x.queue();x.m.f.select(99);x.drain();CHECK(x.m.listener.calls==1);CHECK(x.m.listener.owner==id(11));
    }

    TEST_CASE("CRI014 ScryingMirrorPanel: queued resize uses current texture dimensions") {
        ResizeQueueFixture x;x.queue();x.m.sprite()->data["width"]=200;x.m.sprite()->data["height"]=50;x.drain();REQUIRE(x.m.listener.calls==1);auto v=std::get<Vector3>(x.m.listener.value);CHECK(v.x==doctest::Approx(2));CHECK(v.y==doctest::Approx(.5));
    }

    TEST_CASE("CRI014 ScryingMirrorPanel: non collider target rejects queued resize") {
        ResizeQueueFixture x;x.queue();x.owner()->get_element_by_id(id(22))->type_name="Transform";x.drain();CHECK(x.m.listener.calls==0);
    }

    TEST_CASE("CRI014 ScryingMirrorPanel: uninitialized panel cannot queue resize") {
        ScryingMirrorPanel p;CHECK_NOTHROW(p.queue_collider_resize_for_testing(id(11),id(22),id(21)));
    }
}

namespace {
using namespace EditorCri;
struct MirrorInputFixture {
    GuiFrame gui;
    MirrorFixture& f;
    explicit MirrorInputFixture(MirrorFixture& fixture):f(fixture){}
    void frame(){gui.draw([&]{f.panel.content();});}
    ImGuiWindow* window(){
        auto* w=ImGui::FindWindowByName("CRI002 offscreen host");
        REQUIRE(w);return w;
    }
    void show(){frame();frame();ImGui::FocusWindow(window());frame();}
    void activate(ImGuiID id){ImGui::ActivateItemByID(id);frame();}
    ImGuiID component_seed(ElementArchetype* e){
        REQUIRE(e);
        return window()->GetID(static_cast<void*>(e));
    }
    void resize(ElementArchetype* collider){
        const auto seed=component_seed(collider);
        int n=static_cast<int>(collider->id.get_value());
        activate(ImHashStr("Apply Resize",0,ImHashData(&n,sizeof(n),seed)));
    }
};
}
TEST_SUITE("EditorCRI020_ScryingMirrorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI020 ScryingMirrorPanel: actual visibility checkbox queues authored and preview update"){
        VisibilityFixture v;v.f.select(11);
        MirrorInputFixture ui(v.f);ui.show();
        ui.activate(ui.window()->GetID("Visible"));
        REQUIRE(v.f.c.ctx.deferred_commands.size()==1);
        CHECK(v.model()->is_visible);CHECK(v.live()->is_visible());
        v.drain();
        CHECK_FALSE(v.model()->is_visible);CHECK_FALSE(v.live()->is_visible());
        CHECK(v.f.c.ctx.editor_realm_manager->is_dirty());
    }
    TEST_CASE("CRI020 ScryingMirrorPanel: second actual visibility toggle restores clean snapshot"){
        VisibilityFixture v;v.f.select(11);
        MirrorInputFixture ui(v.f);ui.show();
        auto id=ui.window()->GetID("Visible");
        ui.activate(id);REQUIRE(v.f.c.ctx.deferred_commands.size()==1);v.drain();
        CHECK_FALSE(v.model()->is_visible);
        ui.frame();ui.activate(id);REQUIRE(v.f.c.ctx.deferred_commands.size()==1);v.drain();
        CHECK(v.model()->is_visible);CHECK(v.live()->is_visible());
        CHECK_FALSE(v.f.c.ctx.editor_realm_manager->is_dirty());
    }
    TEST_CASE("CRI020 ScryingMirrorPanel: element-only view cannot queue whole entity visibility"){
        VisibilityFixture v;v.f.select_element(12,11);
        MirrorInputFixture ui(v.f);ui.show();
        ui.activate(ui.window()->GetID("Visible"));
        CHECK(v.f.c.ctx.deferred_commands.empty());
        CHECK(v.model()->is_visible);CHECK(v.live()->is_visible());
    }
    TEST_CASE("CRI020 ScryingMirrorPanel: actual Apply Resize button defers texture-based collider update"){
        ResizeQueueFixture x;x.m.f.select_element(22,11);
        MirrorInputFixture ui(x.m.f);ui.show();
        ui.resize(x.owner()->get_element_by_id(id(22)));
        REQUIRE(x.m.f.c.ctx.deferred_commands.size()==1);
        CHECK(x.m.listener.calls==0);
        x.drain();REQUIRE(x.m.listener.calls==1);
        CHECK(x.m.listener.owner==id(11));CHECK(x.m.listener.component==id(22));
        auto v=std::get<Vector3>(x.m.listener.value);
        CHECK(v.x==doctest::Approx(.8));CHECK(v.y==doctest::Approx(.4));CHECK(v.z==2);
    }
    TEST_CASE("CRI020 ScryingMirrorPanel: no texture sibling means actual resize control cannot queue work"){
        ResizeQueueFixture x;
        x.m.f.c.ctx.editor_realm_manager->purge_element(id(11),id(21));
        x.m.f.select_element(22,11);
        MirrorInputFixture ui(x.m.f);ui.show();
        ui.resize(x.owner()->get_element_by_id(id(22)));
        CHECK(x.m.f.c.ctx.deferred_commands.empty());
        CHECK(x.m.listener.calls==0);
    }
}

namespace {
using namespace EditorCri;
ImGuiID inspector_widget(MirrorInputFixture& ui,ElementArchetype* e,const char* label) {
    REQUIRE(e);
    return ImHashStr(label,0,ImHashStr(e->type_name.c_str(),0,ui.component_seed(e)));
}
}
TEST_SUITE("EditorCRI022_ScryingMirrorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI022 ScryingMirrorPanel: actual reflected visibility checkbox emits component property event") {
        MediaFixture f;
        auto* e=f.sprite();REQUIRE(e);
        e->data["visible"]=true;
        MirrorInputFixture ui(f.f);ui.show();
        ui.activate(inspector_widget(ui,e,"##visible"));
        CHECK_FALSE(e->data["visible"].as<bool>());
        f.f.c.events.process_queue();
        REQUIRE(f.listener.calls==1);
        CHECK(f.listener.owner==id(11));
        CHECK(f.listener.component==id(21));
        CHECK(f.listener.name=="visible");
        CHECK_FALSE(std::get<bool>(f.listener.value));
    }
    TEST_CASE("CRI022 ScryingMirrorPanel: actual media browse widget opens image dialog for selected sprite") {
        MediaFixture f;
        MirrorInputFixture ui(f.f);ui.show();
        ui.activate(inspector_widget(ui,f.sprite(),"...####texture_path"));
        CHECK(f.f.c.gui.shown_dialog=="SelectImageFile");
        auto* d=f.f.c.gui.get_dialog("SelectImageFile");
        REQUIRE(d);
        REQUIRE(d->get_callback());
        CHECK(f.f.c.ctx.deferred_commands.empty());
        CHECK(f.sprite()->data["texture_path"].as<std::string>()=="old.png");
    }
    TEST_CASE("CRI022 ScryingMirrorPanel: actual camera active checkbox selects corresponding preview camera") {
        InspectorCameraFixture f;
        auto* camera=f.camera(11);camera->deactivate();
        auto* owner=f.f.c.ctx.editor_realm_manager->get_archetype(id(11));REQUIRE(owner);
        auto e=element("Camera",21,"{active: false}");
        e.owner_id=id(11);owner->elements.push_back(e);
        f.f.select_element(21,11);
        MirrorInputFixture ui(f.f);ui.show();
        ui.activate(inspector_widget(ui,owner->get_element_by_id(id(21)),"##active"));
        CHECK(owner->get_element_by_id(id(21))->data["active"].as<bool>());
        CHECK(f.main()==id(11));
        CHECK(camera->get_is_active());
        CHECK_FALSE(f.f.c.events.is_queue_empty());
    }
}

TEST_SUITE("EditorCRI023_ScryingMirrorPanel") {
    using namespace EditorCri;
    TEST_CASE("CRI023 ScryingMirrorPanel: removing selected texture resets resize selector to surviving sibling") {
        ResizeQueueFixture x;
        x.m.sprite()->name="First Texture";
        auto second=element("Sprite2D",23,"{width: 150, height: 90}");
        second.owner_id=id(11);second.name="Second Texture";
        x.owner()->elements.push_back(second);
        x.m.f.select_element(22,11);
        MirrorInputFixture ui(x.m.f);ui.show();
        auto* collider=x.owner()->get_element_by_id(id(22));
        REQUIRE(collider);
        int n=22;
        const auto seed=ImHashData(&n,sizeof(n),ui.component_seed(collider));
        ui.activate(ImHashStr("Select Texture",0,seed));ui.frame();
        REQUIRE(ui.gui.current->OpenPopupStack.Size==1);
        auto* popup=ui.gui.current->OpenPopupStack.back().Window;
        REQUIRE(popup);
        ui.activate(ImHashStr("Second Texture",0,popup->GetID(1)));
        REQUIRE(ui.gui.current->OpenPopupStack.Size==0);
        ui.resize(collider);
        REQUIRE(x.m.f.c.ctx.deferred_commands.size()==1);
        x.drain();
        REQUIRE(x.m.listener.calls==1);
        auto first_result=std::get<Vector3>(x.m.listener.value);
        CHECK(first_result.x==doctest::Approx(1.5));
        CHECK(first_result.y==doctest::Approx(.9));

        x.m.f.c.ctx.editor_realm_manager->purge_element(id(11),id(23));
        ui.frame();ui.frame();
        collider=x.owner()->get_element_by_id(id(22));
        REQUIRE(collider);
        ui.resize(collider);
        REQUIRE(x.m.f.c.ctx.deferred_commands.size()==1);
        x.drain();
        REQUIRE(x.m.listener.calls==2);
        auto remaining=std::get<Vector3>(x.m.listener.value);
        CHECK(remaining.x==doctest::Approx(.8));
        CHECK(remaining.y==doctest::Approx(.4));
        CHECK(remaining.z==2);
    }
}
