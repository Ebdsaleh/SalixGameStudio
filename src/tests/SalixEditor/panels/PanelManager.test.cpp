// Tests for Editor/panels/PanelManager.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../fixtures/ArchetypeFixtures.h"
#include <Editor/panels/PanelManager.h>
#include <Editor/panels/IPanel.h>

namespace {
using namespace EditorCri;
struct ProbePanel : IPanel {
    std::string name; bool visible=true, locked=false; int updates=0, renders=0;
    int* destroyed=nullptr;
    ~ProbePanel() override { if(destroyed) ++*destroyed; }
    void initialize(EditorContext*) override {}
    void on_gui_update() override { ++updates; }
    void on_gui_render() override { ++renders; }
    void on_render() override {}
    void on_event(IEvent&) override {}
    void set_visibility(bool v) override { visible=v; }
    bool get_visibility() const override { return visible; }
    void set_name(const std::string& v) override { name=v; }
    const std::string& get_name() override { return name; }
    bool is_locked() override { return locked; }
    void lock() override { locked=true; }
    void unlock() override { locked=false; }
};
ProbePanel* add(PanelManager& m,const char* n) { auto p=std::make_unique<ProbePanel>(); auto* raw=p.get(); m.register_panel(std::move(p),n); return raw; }
}
TEST_SUITE("EditorCRI001_PanelManager") {
    TEST_CASE("CRI001 PanelManager: empty manager lookup returns null") {
        PanelManager m; CHECK(m.get_panel("Missing")==nullptr);
    }

    TEST_CASE("CRI001 PanelManager: null registration is ignored") {
        PanelManager m; m.register_panel(nullptr,"Ghost"); CHECK(m.get_panel("Ghost")==nullptr); int count=0; m.for_each_visible([&](IPanel&){++count;}); CHECK(count==0);
    }

    TEST_CASE("CRI001 PanelManager: registration assigns name and returns same instance") {
        PanelManager m; auto* p=add(m,"Inspector"); CHECK(m.get_panel("Inspector")==p); CHECK(p->get_name()=="Inspector"); CHECK(m.get_panel("inspector")==nullptr);
    }

    TEST_CASE("CRI001 PanelManager: manager owns and destroys registered panels") {
        int destroyed=0; { PanelManager m; auto* a=add(m,"A"); auto* b=add(m,"B"); a->destroyed=&destroyed; b->destroyed=&destroyed; CHECK(destroyed==0); } CHECK(destroyed==2);
    }

    TEST_CASE("CRI001 PanelManager: updates routed once per panel per invocation") {
        PanelManager m; auto* a=add(m,"A"); auto* b=add(m,"B"); m.update_all_panels(); m.update_all_panels(); CHECK(a->updates==2); CHECK(b->updates==2); CHECK(a->renders==0);
    }

    TEST_CASE("CRI001 PanelManager: render dispatched independently of updates") {
        PanelManager m; auto* a=add(m,"A"); auto* b=add(m,"B"); m.render_all_panels(); CHECK(a->renders==1); CHECK(b->renders==1); CHECK(a->updates==0);
    }

    TEST_CASE("CRI001 PanelManager: visible callback excludes hidden panels and retains order") {
        PanelManager m; add(m,"A"); add(m,"B")->set_visibility(false); add(m,"C"); std::vector<std::string> names; m.for_each_visible([&](IPanel& p){names.push_back(p.get_name());}); REQUIRE(names.size()==2); CHECK(names[0]=="A"); CHECK(names[1]=="C");
    }

    TEST_CASE("CRI001 PanelManager: visibility changes affect the next iteration") {
        PanelManager m; auto* a=add(m,"A"); int count=0; a->set_visibility(false); m.for_each_visible([&](IPanel&){++count;}); CHECK(count==0); a->set_visibility(true); m.for_each_visible([&](IPanel&){++count;}); CHECK(count==1);
    }

    TEST_CASE("CRI001 PanelManager: lock and unlock route to named panel only") {
        PanelManager m; auto* a=add(m,"A"); auto* b=add(m,"B"); std::string key="A"; m.lock_panel(key); CHECK(a->is_locked()); CHECK_FALSE(b->is_locked()); m.unlock_panel(key); CHECK_FALSE(a->is_locked());
    }

    TEST_CASE("CRI001 PanelManager: missing panel lock commands leave registered panel unchanged") {
        PanelManager m; auto* a=add(m,"A"); std::string missing="Missing"; m.lock_panel(missing); CHECK_FALSE(a->is_locked()); a->lock(); m.unlock_panel(missing); CHECK(a->is_locked());
    }
}

#include <Editor/events/EntitySelectedEvent.h>
namespace {
using namespace EditorCri;
// A panel that deliberately opts out of locking and inherits the interface
// defaults. PanelManager must support it alongside lockable panels.
struct NonLockablePanel : IPanel {
    std::string name;
    bool visible=true;
    int updates=0, renders=0;
    void initialize(EditorContext*) override {}
    void on_gui_update() override {++updates;}
    void on_gui_render() override {++renders;}
    void on_render() override {}
    void on_event(IEvent& event) override {EditorBehaviour::on_event(event);}
    void set_visibility(bool value) override {visible=value;}
    bool get_visibility() const override {return visible;}
    void set_name(const std::string& value) override {name=value;}
    const std::string& get_name() override {return name;}
};
}
TEST_SUITE("EditorCRI027_PanelManager") {
    using namespace EditorCri;
    TEST_CASE("CRI027 PanelManager: optional lock interface leaves non-lockable panel operational") {
        PanelManager manager;
        auto plain=std::make_unique<NonLockablePanel>();
        auto* observed=plain.get();
        std::string key="NonLockable";
        manager.register_panel(std::move(plain),key);
        auto* lockable=add(manager,"Lockable");
        std::string lock_key="Lockable";
        manager.lock_panel(key);
        CHECK_FALSE(observed->is_locked());
        manager.lock_panel(lock_key);
        CHECK(lockable->is_locked());
        manager.update_all_panels();
        manager.render_all_panels();
        CHECK(observed->updates==1);
        CHECK(observed->renders==1);
        manager.unlock_panel(key);
        CHECK_FALSE(observed->is_locked());
        CHECK(lockable->is_locked());
        CHECK(observed->get_visibility());
        CHECK(observed->get_name()==key);
    }
    TEST_CASE("CRI027 PanelManager: optional editor behaviour lifecycle leaves unhandled event intact") {
        PanelManager manager;
        auto plain=std::make_unique<NonLockablePanel>();
        auto* observed=plain.get();
        manager.register_panel(std::move(plain),"Lifecycle");
        EditorBehaviour& behaviour=*observed;
        EntitySelectedEvent event(id(11));
        behaviour.on_start();
        behaviour.on_update(.016f);
        behaviour.on_event(event);
        CHECK_FALSE(event.handled);
        CHECK(event.selected_id==id(11));
        CHECK(observed->updates==0);
        CHECK(observed->renders==0);
        manager.update_all_panels();
        CHECK(observed->updates==1);
        CHECK(observed->get_name()=="Lifecycle");
    }
}
