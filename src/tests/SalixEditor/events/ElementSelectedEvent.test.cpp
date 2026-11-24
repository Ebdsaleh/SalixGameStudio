// Tests for Editor/events/ElementSelectedEvent.h.
#include "../fixtures/EditorEventFixtures.h"
#include <Editor/events/ElementSelectedEvent.h>
#include <Editor/panels/WorldTreePanel.h>
#include <Editor/panels/ScryingMirrorPanel.h>
#include <Editor/events/EntitySelectedEvent.h>

TEST_SUITE("EditorCRI014_ElementSelectedEvent") {
    using namespace EditorCri;
    using Capture=EditorEventCapture<ElementSelectedEvent>;
    using Fixture=EditorEventFixture<ElementSelectedEvent>;
    TEST_CASE("CRI014 ElementSelectedEvent: stable owner and component identities survive without live element") {
        ElementSelectedEvent e(id(0x100000001ULL),id(0x200000002ULL));CHECK(e.selected_id==id(0x100000001ULL));CHECK(e.owner_id==id(0x200000002ULL));CHECK(e.element==nullptr);
    }

    TEST_CASE("CRI014 ElementSelectedEvent: live element pointer is retained alongside authored identity") {
        Realm r;auto* a=r.create_entity(id(11),"Owner");auto* t=a->get_transform();REQUIRE(t);ElementSelectedEvent e(id(21),id(11),t);CHECK(e.element==t);CHECK(e.owner_id==id(11));CHECK(e.selected_id==id(21));
    }

    TEST_CASE("CRI014 ElementSelectedEvent: clone preserves pointer IDs and handled status") {
        Realm r;auto* t=r.create_entity(id(11),"Owner")->get_transform();REQUIRE(t);ElementSelectedEvent e(id(21),id(11),t);e.handled=true;auto clone=e.clone();auto* c=dynamic_cast<ElementSelectedEvent*>(clone.get());REQUIRE(c);CHECK(c->element==t);CHECK(c->selected_id==id(21));CHECK(c->owner_id==id(11));CHECK(c->handled);
    }

    TEST_CASE("CRI014 ElementSelectedEvent: metadata identifies Editor selection rather than input") {
        ElementSelectedEvent e(id(21),id(11));CHECK(e.get_event_type()==EventType::EditorElementSelected);CHECK(std::string(e.get_name())=="EditorElementSelected");CHECK(e.is_in_category(EventCategory::Editor));CHECK_FALSE(e.is_mouse_event());
    }

    TEST_CASE("CRI014 ElementSelectedEvent: queued clone retains old selection after original changes") {
        Fixture f;ElementSelectedEvent e(id(21),id(11));f.context.events.dispatch(e);e.owner_id=id(12);e.selected_id=id(22);f.drain();REQUIRE(f.capture.count()==1);CHECK(f.capture.at(0).selected_id==id(21));CHECK(f.capture.at(0).owner_id==id(11));
    }

    TEST_CASE("CRI014 ElementSelectedEvent: successive selection events retain order and owner identity") {
        Fixture f;f.context.events.dispatch(ElementSelectedEvent(id(21),id(11)));f.context.events.dispatch(ElementSelectedEvent(id(22),id(12)));f.drain();REQUIRE(f.capture.count()==2);CHECK(f.capture.at(0).selected_id==id(21));CHECK(f.capture.at(1).owner_id==id(12));
    }

    TEST_CASE("CRI014 ElementSelectedEvent: queued selection synchronizes tree and inspector") {
        PanelContext c;WorldTreePanel tree;ScryingMirrorPanel mirror;tree.initialize(&c.ctx);mirror.initialize(&c.ctx);c.events.dispatch(ElementSelectedEvent(id(21),id(11)));c.events.process_queue();CHECK(c.ctx.selected_entity_id==id(11));CHECK(c.ctx.selected_element_id==id(21));CHECK(mirror.get_selected_entity_id_for_testing()==id(11));CHECK(mirror.get_selected_element_id_for_testing()==id(21));
    }

    TEST_CASE("CRI014 ElementSelectedEvent: invalid selected and owner IDs clear panel selections") {
        PanelContext c;WorldTreePanel tree;ScryingMirrorPanel mirror;tree.initialize(&c.ctx);mirror.initialize(&c.ctx);c.events.dispatch(ElementSelectedEvent(id(21),id(11)));c.events.dispatch(ElementSelectedEvent(SimpleGuid::invalid(),SimpleGuid::invalid()));c.events.process_queue();CHECK_FALSE(c.ctx.selected_entity_id.is_valid());CHECK_FALSE(c.ctx.selected_element_id.is_valid());CHECK_FALSE(mirror.get_selected_element_id_for_testing().is_valid());
    }

    TEST_CASE("CRI014 ElementSelectedEvent: manager duplicate emits selection for new component identity") {
        Fixture f;f.root(11);auto a=element("Sprite2D",21);a.owner_id=id(11);f.manager().add_element_to_entity(id(11),a);f.drain();SimpleGuid::update_next_id(300000);f.manager().duplicate_element(id(11),id(21));f.drain();REQUIRE(f.capture.count()==1);auto& e=f.capture.at(0);CHECK(e.owner_id==id(11));CHECK(e.selected_id!=id(21));REQUIRE(f.manager().get_archetype(id(11))->get_element_by_id(e.selected_id));CHECK(e.element==nullptr);
    }

    TEST_CASE("CRI014 ElementSelectedEvent: absent source component cannot emit a duplicate selection") {
        Fixture f;f.root(11);f.drain();f.manager().duplicate_element(id(11),id(99));f.drain();CHECK(f.capture.count()==0);CHECK(f.manager().get_archetype(id(11))->elements.empty());
    }
}
