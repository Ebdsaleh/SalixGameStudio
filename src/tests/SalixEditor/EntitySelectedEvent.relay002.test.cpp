// CRI relay 002: EntitySelectedEvent; off-screen and observable behavior.
#include "CriRelay002Fixtures.h"
#include <Editor/events/EntitySelectedEvent.h>
#include <Salix/ecs/Entity.h>
#include <Salix/events/IEventListener.h>
namespace {
using namespace EditorCri;
struct BlockGuard {EntitySelectedEvent e{id(1)};bool old=e.should_block();BlockGuard(){e.set_block(false);}~BlockGuard(){e.set_block(old);}};
struct Listener:IEventListener{int calls=0;SimpleGuid selected=SimpleGuid::invalid();void on_event(IEvent& e)override{++calls;selected=static_cast<EntitySelectedEvent&>(e).selected_id;}};
}
TEST_SUITE("EditorCRI002_EntitySelectedEvent") {
    TEST_CASE("CRI002 EntitySelectedEvent: stable ID valid without live pointer") {
        BlockGuard g;EntitySelectedEvent e(id(11));CHECK(e.selected_id==id(11));CHECK(e.entity==nullptr);
    }

    TEST_CASE("CRI002 EntitySelectedEvent: live pointer and supplied ID preserved") {
        BlockGuard g;Entity live;EntitySelectedEvent e(id(11),&live);CHECK(e.entity==&live);CHECK(e.selected_id==id(11));
    }

    TEST_CASE("CRI002 EntitySelectedEvent: invalid ID represents deselection") {
        BlockGuard g;EntitySelectedEvent e(SimpleGuid::invalid());CHECK_FALSE(e.selected_id.is_valid());CHECK(e.entity==nullptr);
    }

    TEST_CASE("CRI002 EntitySelectedEvent: event type name and editor category") {
        BlockGuard g;EntitySelectedEvent e(id(11));CHECK(e.get_event_type()==EventType::EditorEntitySelected);CHECK(std::string(e.get_name())=="EditorEntitySelected");CHECK(e.is_in_category(EventCategory::Editor));CHECK_FALSE(e.is_input_event());
    }

    TEST_CASE("CRI002 EntitySelectedEvent: clone preserves ID pointer and handled flag") {
        BlockGuard g;Entity live;EntitySelectedEvent e(id(11),&live);e.handled=true;auto clone=e.clone();auto* p=dynamic_cast<EntitySelectedEvent*>(clone.get());REQUIRE(p);CHECK(p!=&e);CHECK(p->selected_id==e.selected_id);CHECK(p->entity==&live);CHECK(p->handled);
    }

    TEST_CASE("CRI002 EntitySelectedEvent: block flag shared across existing and new instances") {
        BlockGuard g;EntitySelectedEvent a(id(11)),b(id(12));a.set_block(true);CHECK(b.should_block());EntitySelectedEvent c(id(13));CHECK(c.should_block());b.set_block(false);CHECK_FALSE(a.should_block());CHECK_FALSE(c.should_block());
    }

    TEST_CASE("CRI002 EntitySelectedEvent: clone observes shared block changes") {
        BlockGuard g;EntitySelectedEvent a(id(11));auto b=a.clone();a.set_block(true);CHECK(b->should_block());b->set_block(false);CHECK_FALSE(a.should_block());
    }

    TEST_CASE("CRI002 EntitySelectedEvent: blocked event not queued") {
        BlockGuard g;EventManager m;Listener l;m.subscribe(EventCategory::Editor,&l);EntitySelectedEvent e(id(11));e.set_block(true);m.dispatch(e);CHECK(m.is_queue_empty());m.process_queue();CHECK(l.calls==0);
    }

    TEST_CASE("CRI002 EntitySelectedEvent: unblocked event delivered with correct identity") {
        BlockGuard g;EventManager m;Listener l;m.subscribe(EventCategory::Editor,&l);EntitySelectedEvent e(id(11));m.dispatch(e);CHECK_FALSE(m.is_queue_empty());m.process_queue();CHECK(l.calls==1);CHECK(l.selected==id(11));CHECK(m.is_queue_empty());
    }

    TEST_CASE("CRI002 EntitySelectedEvent: blocking then releasing only affects later dispatch") {
        BlockGuard g;EventManager m;Listener l;m.subscribe(EventCategory::Editor,&l);EntitySelectedEvent a(id(11));a.set_block(true);m.dispatch(a);a.set_block(false);EntitySelectedEvent b(id(12));m.dispatch(b);m.process_queue();CHECK(l.calls==1);CHECK(l.selected==id(12));
    }
}
