// =================================================================================
// Filename:    src/Tests/SalixEngine/events/EventManager.test.cpp
// Description: Contains unit tests for the EventManager.
// ================================================================================= 

#include <doctest.h>
#include <Salix/events/EventManager.h>
// Include the new reusable mock header file.
#include <Tests/SalixEngine/mocking/events/MockEventSystem.h>

namespace Salix {

    TEST_SUITE("Salix::events::EventManager") {

        TEST_CASE("a subscribed listener receives a dispatched event") {
            // ARRANGE
            EventManager event_manager;
            MockListener listener;
            auto mock_event = std::make_unique<MockEvent>();

            // ACT
            event_manager.subscribe(EventCategory::Application, &listener);
            event_manager.dispatch(std::move(mock_event));
            event_manager.process_queue();

            // ASSERT
            CHECK(listener.event_received == true);
        }

        TEST_CASE("an unsubscribed listener does not receive events") {
            // ARRANGE
            EventManager event_manager;
            MockListener listener;
            auto mock_event = std::make_unique<MockEvent>();

            event_manager.subscribe(EventCategory::Application, &listener);
            
            // ACT
            event_manager.unsubscribe(EventCategory::Application, &listener);
            event_manager.dispatch(std::move(mock_event));
            event_manager.process_queue();

            // ASSERT
            CHECK(listener.event_received == false);
        }

        TEST_CASE("a handled event is not dispatched to other listeners") {
            // ARRANGE
            EventManager event_manager;
            auto mock_event = std::make_unique<MockEvent>();
            MockListener listener1;
            MockListener listener2;

            // --- CORRECTED LOGIC ---
            // Assign a lambda to the handler variable, not the function itself.
            listener1.on_event_handler = [&](IEvent& event) {
                event.handled = true;
                listener1.event_received = true;
            };

            // ACT
            event_manager.subscribe(EventCategory::Application, &listener1);
            event_manager.subscribe(EventCategory::Application, &listener2);
            event_manager.dispatch(std::move(mock_event));
            event_manager.process_queue();

            // ASSERT
            CHECK(listener1.event_received == true);
            CHECK(listener2.event_received == false);
        }

        TEST_CASE("helper methods and overloads work correctly") {
            // ARRANGE
            EventManager event_manager;
            MockListener listener;
            MockEvent mock_event_on_stack;

            // ASSERT
            CHECK(event_manager.is_queue_empty() == true);

            // ACT
            event_manager.subscribe(EventCategory::Application, &listener);
            event_manager.dispatch(mock_event_on_stack);

            // ASSERT
            CHECK(event_manager.is_queue_empty() == false);
            event_manager.process_queue();
            CHECK(event_manager.is_queue_empty() == true);
            CHECK(listener.event_received == true);
        }
        TEST_CASE("a blocking event is not added to the queue") {
            // ARRANGE
            EventManager event_manager;
            MockListener listener;
            auto blocking_event = std::make_unique<MockBlockingEvent>();

            // ACT
            event_manager.subscribe(EventCategory::Application, &listener);
            event_manager.dispatch(std::move(blocking_event)); // Dispatch the blocking event

            // ASSERT
            // The queue should still be empty because the event was blocked before being added.
            CHECK(event_manager.is_queue_empty() == true);

            // ACT (continued)
            event_manager.process_queue(); // Process the (empty) queue

            // ASSERT
            // The listener should not have received the event.
            CHECK(listener.event_received == false);
        }
    }

} // namespace Salix

// Cross-category and multi-listener queue contracts (no Editor components required).
namespace {
    class AdditionalMultiCategoryEvent final : public Salix::IEvent {
    public:
        Salix::EventType get_event_type() const override { return Salix::EventType::AppTick; }
        const char* get_name() const override { return "AdditionalMultiCategoryEvent"; }
        int get_category_flags() const override { return Salix::EventCategory::Application | Salix::EventCategory::Editor; }
        std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<AdditionalMultiCategoryEvent>(*this); }
    };
}

TEST_SUITE("Salix::events::EventManagerAdditional") {
    TEST_CASE("event manager two listeners in one category both receive an unhandled event") {
        Salix::EventManager manager;
        Salix::MockListener a, b;
        int ac = 0, bc = 0;
        a.on_event_handler = [&](Salix::IEvent&) { ++ac; };
        b.on_event_handler = [&](Salix::IEvent&) { ++bc; };
        manager.subscribe(Salix::EventCategory::Application, &a);
        manager.subscribe(Salix::EventCategory::Application, &b);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(ac == 1);
        CHECK(bc == 1);
    }

    TEST_CASE("event manager two queued events each reach both eligible listeners") {
        Salix::EventManager manager;
        Salix::MockListener a, b;
        int ac = 0, bc = 0;
        a.on_event_handler = [&](Salix::IEvent&) { ++ac; };
        b.on_event_handler = [&](Salix::IEvent&) { ++bc; };
        manager.subscribe(Salix::EventCategory::Application, &a);
        manager.subscribe(Salix::EventCategory::Application, &b);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(ac == 2);
        CHECK(bc == 2);
    }

    TEST_CASE("event manager unsubscribing an unknown listener does not remove another subscriber") {
        Salix::EventManager manager;
        Salix::MockListener subscribed, stranger;
        manager.subscribe(Salix::EventCategory::Application, &subscribed);
        manager.unsubscribe(Salix::EventCategory::Application, &stranger);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(subscribed.event_received);
        CHECK_FALSE(stranger.event_received);
    }

    TEST_CASE("event manager unsubscribing an unknown category preserves application subscription") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.unsubscribe(Salix::EventCategory::Editor, &listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(listener.event_received);
    }

    TEST_CASE("event manager multi-category event reaches one listener for each category subscription") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int received = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++received; };
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Editor, &listener);
        manager.dispatch(std::make_unique<AdditionalMultiCategoryEvent>());
        manager.process_queue();
        CHECK(received == 2);
    }

    TEST_CASE("event manager handled multi-category event does not leak into later categories") {
        Salix::EventManager manager;
        Salix::MockListener first, later;
        int later_received = 0;
        first.on_event_handler = [&](Salix::IEvent& e) { e.handled = true; };
        later.on_event_handler = [&](Salix::IEvent&) { ++later_received; };
        manager.subscribe(Salix::EventCategory::Application, &first);
        manager.subscribe(Salix::EventCategory::Editor, &later);
        manager.dispatch(std::make_unique<AdditionalMultiCategoryEvent>());
        manager.process_queue();
        CHECK(later_received == 0);
    }

    TEST_CASE("event manager handling first queued event leaves later queued events independently handled") {
        Salix::EventManager manager;
        Salix::MockListener handler, observer;
        int count = 0;
        handler.on_event_handler = [&](Salix::IEvent& e) { if (++count == 1) e.handled = true; };
        int observers = 0;
        observer.on_event_handler = [&](Salix::IEvent&) { ++observers; };
        manager.subscribe(Salix::EventCategory::Application, &handler);
        manager.subscribe(Salix::EventCategory::Application, &observer);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(count == 2);
        CHECK(observers == 1);
    }

    TEST_CASE("event manager resubscribing after unsubscribe restores exactly one delivery") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.unsubscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(calls == 1);
    }

    TEST_CASE("event manager separate managers never deliver an event through the other manager") {
        Salix::EventManager alpha, beta;
        Salix::MockListener a, b;
        alpha.subscribe(Salix::EventCategory::Application, &a);
        beta.subscribe(Salix::EventCategory::Application, &b);
        alpha.dispatch(std::make_unique<Salix::MockEvent>());
        alpha.process_queue();
        CHECK(a.event_received);
        CHECK_FALSE(b.event_received);
        CHECK(beta.is_queue_empty());
    }

    TEST_CASE("event manager empty process and subsequently added events preserve queue readiness") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.process_queue();
        CHECK(manager.is_queue_empty());
        CHECK_FALSE(listener.event_received);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK_FALSE(manager.is_queue_empty());
        manager.process_queue();
        CHECK(manager.is_queue_empty());
        CHECK(listener.event_received);
    }
}

// Queue timing and listener-ownership regressions without live Editor objects.
TEST_SUITE("Salix::events::EventManagerAdditional20261019") {
    TEST_CASE("events extended large queue remains deferred until processing and then drains entirely") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int count=0;
        listener.on_event_handler=[&](Salix::IEvent&){ ++count; };
        manager.subscribe(Salix::EventCategory::Application,&listener);
        for(int i=0;i<32;++i) manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK(count==0);
        CHECK_FALSE(manager.is_queue_empty());
        manager.process_queue();
        CHECK(count==32);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("events extended second process without new dispatch must not replay delivered events") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int count=0;
        listener.on_event_handler=[&](Salix::IEvent&){ ++count; };
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        manager.process_queue();
        CHECK(count==1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("events extended unsubscribing before queued delivery prevents any notifications") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.unsubscribe(Salix::EventCategory::Application,&listener);
        manager.process_queue();
        CHECK_FALSE(listener.event_received);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("events extended subscribing after an event is queued makes it visible at processing time") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.process_queue();
        CHECK(listener.event_received);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("events extended duplicating a subscription cannot multiply event deliveries") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int count=0;
        listener.on_event_handler=[&](Salix::IEvent&){ ++count; };
        for(int i=0;i<5;++i)manager.subscribe(Salix::EventCategory::Application,&listener);
        for(int i=0;i<3;++i)manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(count==3);
    }

    TEST_CASE("events extended self unsubscription during delivery applies to later queued events") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int count=0;
        listener.on_event_handler=[&](Salix::IEvent&){ ++count; manager.unsubscribe(Salix::EventCategory::Application,&listener); };
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(count==1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("events extended two distinct managers keep deferred queues independent") {
        Salix::EventManager first,second;
        Salix::MockListener listener_a,listener_b;
        first.subscribe(Salix::EventCategory::Application,&listener_a);
        second.subscribe(Salix::EventCategory::Application,&listener_b);
        first.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK(second.is_queue_empty());
        second.process_queue();
        CHECK_FALSE(listener_b.event_received);
        first.process_queue();
        CHECK(listener_a.event_received);
    }

    TEST_CASE("events extended blocking a queued event does not prevent later normal delivery") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int count=0;
        listener.on_event_handler=[&](Salix::IEvent&){ ++count; };
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.dispatch(std::make_unique<Salix::MockBlockingEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(count==1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("events extended a handled event stops later listeners but the next event still arrives") {
        Salix::EventManager manager;
        Salix::MockListener first,second;
        int first_calls=0,second_calls=0;
        first.on_event_handler=[&](Salix::IEvent& event){ ++first_calls; if(first_calls==1)event.handled=true; };
        second.on_event_handler=[&](Salix::IEvent&){ ++second_calls; };
        manager.subscribe(Salix::EventCategory::Application,&first);
        manager.subscribe(Salix::EventCategory::Application,&second);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(first_calls==2);
        CHECK(second_calls==1);
    }

    TEST_CASE("events extended dispatching by reference clones an event before stack lifetime ends") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int count=0;
        listener.on_event_handler=[&](Salix::IEvent&){ ++count; };
        manager.subscribe(Salix::EventCategory::Application,&listener);
        { const Salix::MockEvent local; manager.dispatch(local); }
        CHECK(count==0);
        manager.process_queue();
        CHECK(count==1);
        CHECK(manager.is_queue_empty());
    }
}

// Additional diagnostic and deferred-delivery contracts.
#include <vector>
namespace { class Batch647NumberedEvent final : public Salix::IEvent {
public:
    explicit Batch647NumberedEvent(int n):value(n){}
    Salix::EventType get_event_type() const override{return Salix::EventType::AppTick;}
    const char* get_name() const override{return "Batch647NumberedEvent";}
    int get_category_flags() const override{return static_cast<int>(Salix::EventCategory::Application);}
    std::unique_ptr<Salix::IEvent> clone() const override{return std::make_unique<Batch647NumberedEvent>(*this);}
    int value;
}; }
TEST_SUITE("Salix::events::EventManagerBatch647") {
    TEST_CASE("an unhandled event is delivered to all eligible listeners in order") {
        Salix::EventManager manager; Salix::MockListener first,second,third;
        std::vector<int> calls;
        first.on_event_handler=[&](Salix::IEvent&){calls.push_back(1);};
        second.on_event_handler=[&](Salix::IEvent&){calls.push_back(2);};
        third.on_event_handler=[&](Salix::IEvent&){calls.push_back(3);};
        manager.subscribe(Salix::EventCategory::Application,&first);
        manager.subscribe(Salix::EventCategory::Application,&second);
        manager.subscribe(Salix::EventCategory::Application,&third);
        manager.dispatch(std::make_unique<Salix::MockEvent>());manager.process_queue();
        CHECK((calls==std::vector<int>{1,2,3}));
    }

    TEST_CASE("a handler in the middle stops later listeners for the same event") {
        Salix::EventManager manager; Salix::MockListener first,second,third;
        int a=0,b=0,c=0;
        first.on_event_handler=[&](Salix::IEvent&){++a;};
        second.on_event_handler=[&](Salix::IEvent& e){++b;e.handled=true;};
        third.on_event_handler=[&](Salix::IEvent&){++c;};
        manager.subscribe(Salix::EventCategory::Application,&first);
        manager.subscribe(Salix::EventCategory::Application,&second);
        manager.subscribe(Salix::EventCategory::Application,&third);
        manager.dispatch(std::make_unique<Salix::MockEvent>());manager.process_queue();
        CHECK(a==1);CHECK(b==1);CHECK(c==0);
    }

    TEST_CASE("unsubscribing a later listener while processing preserves current snapshot") {
        Salix::EventManager manager; Salix::MockListener first,later;
        int calls=0;
        first.on_event_handler=[&](Salix::IEvent&){manager.unsubscribe(Salix::EventCategory::Application,&later);};
        later.on_event_handler=[&](Salix::IEvent&){++calls;};
        manager.subscribe(Salix::EventCategory::Application,&first);
        manager.subscribe(Salix::EventCategory::Application,&later);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(calls==1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("a newly subscribed listener receives later queued events not the current snapshot") {
        Salix::EventManager manager;Salix::MockListener first,late;
        int late_calls=0;
        first.on_event_handler=[&](Salix::IEvent&){manager.subscribe(Salix::EventCategory::Application,&late);};
        late.on_event_handler=[&](Salix::IEvent&){++late_calls;};
        manager.subscribe(Salix::EventCategory::Application,&first);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(late_calls==1);
    }

    TEST_CASE("a queue with no subscribers drains every pending event") {
        Salix::EventManager manager;
        for(int i=0;i<16;++i)manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK_FALSE(manager.is_queue_empty());
        manager.process_queue();
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("removing all listeners leaves later events queued then harmlessly consumed") {
        Salix::EventManager manager;Salix::MockListener first,second;
        manager.subscribe(Salix::EventCategory::Application,&first);
        manager.subscribe(Salix::EventCategory::Application,&second);
        manager.unsubscribe(Salix::EventCategory::Application,&first);
        manager.unsubscribe(Salix::EventCategory::Application,&second);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK_FALSE(first.event_received);CHECK_FALSE(second.event_received);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("the same listener can unsubscribe then subscribe and receive only once") {
        Salix::EventManager manager;Salix::MockListener listener;
        int deliveries=0;listener.on_event_handler=[&](Salix::IEvent&){++deliveries;};
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.unsubscribe(Salix::EventCategory::Application,&listener);
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(deliveries==1);
    }

    TEST_CASE("blocking is event-local and never discards the pending normal event") {
        Salix::EventManager manager;Salix::MockListener listener;
        int count=0;listener.on_event_handler=[&](Salix::IEvent&){++count;};
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockBlockingEvent>());
        CHECK_FALSE(manager.is_queue_empty());
        manager.process_queue();
        CHECK(count==1);CHECK(manager.is_queue_empty());
    }

    TEST_CASE("queued events are received in the exact order they were dispatched") {
        Salix::EventManager manager;Salix::MockListener listener;
        std::vector<int> observed;
        listener.on_event_handler=[&](Salix::IEvent& e){observed.push_back(static_cast<Batch647NumberedEvent&>(e).value);};
        manager.subscribe(Salix::EventCategory::Application,&listener);
        for(int value:{7,1,42,-3})manager.dispatch(std::make_unique<Batch647NumberedEvent>(value));
        manager.process_queue();
        CHECK((observed==std::vector<int>{7,1,42,-3}));
    }

    TEST_CASE("stack event dispatch clones payload before its original scope ends") {
        Salix::EventManager manager;Salix::MockListener listener;
        int seen=-1;
        listener.on_event_handler=[&](Salix::IEvent& e){seen=static_cast<Batch647NumberedEvent&>(e).value;};
        manager.subscribe(Salix::EventCategory::Application,&listener);
        {const Batch647NumberedEvent temporary(902);manager.dispatch(temporary);}
        CHECK(seen==-1);
        manager.process_queue();
        CHECK(seen==902);
    }
}

namespace {
class Batch947EditorOnlyEvent final : public Salix::IEvent {
public:
    Salix::EventType get_event_type() const override { return Salix::EventType::EditorOnHierarchyChanged; }
    const char* get_name() const override { return "Batch947EditorOnlyEvent"; }
    int get_category_flags() const override { return static_cast<int>(Salix::EventCategory::Editor); }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch947EditorOnlyEvent>(*this); }
};

class Batch947DualCategoryEvent final : public Salix::IEvent {
public:
    Salix::EventType get_event_type() const override { return Salix::EventType::AppUpdate; }
    const char* get_name() const override { return "Batch947DualCategoryEvent"; }
    int get_category_flags() const override { return Salix::EventCategory::Application | Salix::EventCategory::Editor; }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch947DualCategoryEvent>(*this); }
};
}

TEST_SUITE("Salix::events::EventManagerBatch947") {
    TEST_CASE("one listener subscribed to both matching categories receives a dual-category event twice") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Editor, &listener);
        manager.dispatch(std::make_unique<Batch947DualCategoryEvent>());
        manager.process_queue();
        CHECK(calls == 2);
    }

    TEST_CASE("unsubscribing one matching category leaves the other dual-category delivery intact") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Editor, &listener);
        manager.unsubscribe(Salix::EventCategory::Application, &listener);
        manager.dispatch(std::make_unique<Batch947DualCategoryEvent>());
        manager.process_queue();
        CHECK(calls == 1);
    }

    TEST_CASE("duplicate subscriptions remain deduplicated independently in each category") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        for (int i = 0; i < 4; ++i) {
            manager.subscribe(Salix::EventCategory::Application, &listener);
            manager.subscribe(Salix::EventCategory::Editor, &listener);
        }
        manager.dispatch(std::make_unique<Batch947DualCategoryEvent>());
        manager.process_queue();
        CHECK(calls == 2);
    }

    TEST_CASE("application listener ignores an editor-only event") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.dispatch(std::make_unique<Batch947EditorOnlyEvent>());
        manager.process_queue();
        CHECK_FALSE(listener.event_received);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("editor listener receives editor-only event without application subscription") {
        Salix::EventManager manager;
        Salix::MockListener editor;
        Salix::MockListener application;
        manager.subscribe(Salix::EventCategory::Editor, &editor);
        manager.subscribe(Salix::EventCategory::Application, &application);
        manager.dispatch(std::make_unique<Batch947EditorOnlyEvent>());
        manager.process_queue();
        CHECK(editor.event_received);
        CHECK_FALSE(application.event_received);
    }

    TEST_CASE("mixed category queue routes each event only to its eligible listener") {
        Salix::EventManager manager;
        Salix::MockListener editor, application;
        int editor_calls = 0, application_calls = 0;
        editor.on_event_handler = [&](Salix::IEvent&) { ++editor_calls; };
        application.on_event_handler = [&](Salix::IEvent&) { ++application_calls; };
        manager.subscribe(Salix::EventCategory::Editor, &editor);
        manager.subscribe(Salix::EventCategory::Application, &application);
        manager.dispatch(std::make_unique<Batch947EditorOnlyEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(editor_calls == 1);
        CHECK(application_calls == 1);
    }

    TEST_CASE("unsubscribing an unknown application listener cannot remove editor subscription") {
        Salix::EventManager manager;
        Salix::MockListener subscribed, unknown;
        int calls = 0;
        subscribed.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        manager.subscribe(Salix::EventCategory::Editor, &subscribed);
        manager.unsubscribe(Salix::EventCategory::Application, &unknown);
        manager.dispatch(std::make_unique<Batch947EditorOnlyEvent>());
        manager.process_queue();
        CHECK(calls == 1);
    }

    TEST_CASE("stack blocking dispatch marks only its clone and leaves source state unchanged") {
        Salix::EventManager manager;
        Salix::MockBlockingEvent source;
        REQUIRE_FALSE(source.handled);
        manager.dispatch(source);
        CHECK_FALSE(source.handled);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("blocking event queued first cannot prevent a later normal event from delivery") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.dispatch(std::make_unique<Salix::MockBlockingEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(calls == 1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("repeated empty processing preserves subscriptions for future delivery") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int calls = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++calls; };
        manager.subscribe(Salix::EventCategory::Application, &listener);
        for (int i = 0; i < 5; ++i) {
            manager.process_queue();
            CHECK(manager.is_queue_empty());
        }
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();
        CHECK(calls == 1);
    }
}

// ---- Relocated from src/tests/SalixEngine/events/EventManagerQueue.test.cpp; canonical tests mirror production .cpp files ----
// EventManagerQueue.test.cpp - deferred event queue behavioural regressions.
// No application windows, project files, assets or graphics backends are used.
#include <doctest.h>
#include <Salix/events/EventManager.h>
#include <Tests/SalixEngine/mocking/events/MockEventSystem.h>

#include <memory>
#include <vector>

namespace {
    // Encodes ordering explicitly: different events of the same type can be
    // distinguished without relying on pointer identity or execution timing.
    class NumberedApplicationEvent final : public Salix::IEvent {
    public:
        explicit NumberedApplicationEvent(int number) : sequence(number) {}

        Salix::EventType get_event_type() const override {
            return Salix::EventType::AppTick;
        }
        const char* get_name() const override { return "NumberedApplicationEvent"; }
        int get_category_flags() const override {
            return static_cast<int>(Salix::EventCategory::Application);
        }
        std::unique_ptr<Salix::IEvent> clone() const override {
            return std::make_unique<NumberedApplicationEvent>(*this);
        }

        int sequence;
    };
}

TEST_SUITE("Salix::events::EventManagerQueue") {

    TEST_CASE("duplicate subscription delivers each event just once to that listener") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };

        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();

        CHECK(deliveries == 1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("dispatch defers delivery until process_queue and drains pending events") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        CHECK(manager.is_queue_empty());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK_FALSE(manager.is_queue_empty());
        CHECK(deliveries == 0);

        manager.process_queue();
        CHECK(manager.is_queue_empty());
        CHECK(deliveries == 1);

        manager.process_queue();
        CHECK(deliveries == 1);
    }

    TEST_CASE("queued events reach subscribers in FIFO order") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        std::vector<int> received;
        listener.on_event_handler = [&](Salix::IEvent& event) {
            received.push_back(static_cast<NumberedApplicationEvent&>(event).sequence);
        };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        manager.dispatch(std::make_unique<NumberedApplicationEvent>(41));
        manager.dispatch(std::make_unique<NumberedApplicationEvent>(7));
        manager.dispatch(std::make_unique<NumberedApplicationEvent>(99));
        CHECK(received.empty());

        manager.process_queue();
        REQUIRE(received.size() == 3);
        CHECK(received[0] == 41);
        CHECK(received[1] == 7);
        CHECK(received[2] == 99);
    }

    TEST_CASE("unsubscribing before processing prevents delivery of queued events") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.unsubscribe(Salix::EventCategory::Application, &listener);
        manager.process_queue();

        CHECK(deliveries == 0);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("a listener subscribed after dispatch receives the still-queued event") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };

        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.subscribe(Salix::EventCategory::Application, &listener);
        manager.process_queue();

        CHECK(deliveries == 1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("unrelated event categories are not delivered to subscribers") {
        Salix::EventManager manager;
        Salix::MockListener editor_listener;
        Salix::MockListener application_listener;
        int editor_calls = 0;
        int application_calls = 0;
        editor_listener.on_event_handler = [&](Salix::IEvent&) { ++editor_calls; };
        application_listener.on_event_handler = [&](Salix::IEvent&) { ++application_calls; };
        manager.subscribe(Salix::EventCategory::Editor, &editor_listener);
        manager.subscribe(Salix::EventCategory::Application, &application_listener);

        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();

        CHECK(editor_calls == 0);
        CHECK(application_calls == 1);
    }

    TEST_CASE("handled events stop notifying later listeners but not later events") {
        Salix::EventManager manager;
        Salix::MockListener first;
        Salix::MockListener second;
        int first_calls = 0;
        int second_calls = 0;
        first.on_event_handler = [&](Salix::IEvent& event) {
            ++first_calls;
            event.handled = true;
        };
        second.on_event_handler = [&](Salix::IEvent&) { ++second_calls; };
        manager.subscribe(Salix::EventCategory::Application, &first);
        manager.subscribe(Salix::EventCategory::Application, &second);

        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        manager.process_queue();

        CHECK(first_calls == 2);
        CHECK(second_calls == 0);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("blocking an event does not block subsequent normal events") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        int deliveries = 0;
        listener.on_event_handler = [&](Salix::IEvent&) { ++deliveries; };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        manager.dispatch(std::make_unique<Salix::MockBlockingEvent>());
        CHECK(manager.is_queue_empty());
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK_FALSE(manager.is_queue_empty());
        manager.process_queue();

        CHECK(deliveries == 1);
        CHECK(manager.is_queue_empty());
    }

    TEST_CASE("dispatching by const reference clones the event for later delivery") {
        Salix::EventManager manager;
        Salix::MockListener listener;
        std::vector<int> received;
        listener.on_event_handler = [&](Salix::IEvent& event) {
            received.push_back(static_cast<NumberedApplicationEvent&>(event).sequence);
        };
        manager.subscribe(Salix::EventCategory::Application, &listener);

        {
            const NumberedApplicationEvent stack_event(1234);
            manager.dispatch(stack_event);
        }

        CHECK(received.empty());
        manager.process_queue();
        REQUIRE(received.size() == 1);
        CHECK(received.front() == 1234);
    }

    TEST_CASE("processing an event without listeners still drains the queue") {
        Salix::EventManager manager;
        manager.dispatch(std::make_unique<Salix::MockEvent>());
        CHECK_FALSE(manager.is_queue_empty());

        manager.process_queue();
        CHECK(manager.is_queue_empty());
    }
}


// Integration boundaries for the 847+ coverage batch.
TEST_SUITE("Salix::events::EventManagerQueueBatch847") {
    TEST_CASE("processing an already empty queue is idempotent") {
        Salix::EventManager manager; CHECK(manager.is_queue_empty()); manager.process_queue(); CHECK(manager.is_queue_empty()); manager.process_queue(); CHECK(manager.is_queue_empty());
    }

    TEST_CASE("unsubscribing an unknown listener from an unused category is harmless") {
        Salix::EventManager manager; Salix::MockListener listener; CHECK_NOTHROW(manager.unsubscribe(Salix::EventCategory::Editor,&listener)); CHECK(manager.is_queue_empty());
    }

    TEST_CASE("two listeners receive an unhandled event in subscription order") {
        Salix::EventManager manager; Salix::MockListener a,b; std::vector<int> order; a.on_event_handler=[&](Salix::IEvent&){order.push_back(1);}; b.on_event_handler=[&](Salix::IEvent&){order.push_back(2);}; manager.subscribe(Salix::EventCategory::Application,&a); manager.subscribe(Salix::EventCategory::Application,&b); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); REQUIRE(order.size()==2); CHECK(order[0]==1); CHECK(order[1]==2);
    }

    TEST_CASE("a listener can unsubscribe itself during delivery and misses the next queued event") {
        Salix::EventManager manager; Salix::MockListener self; int calls=0; self.on_event_handler=[&](Salix::IEvent&){++calls; manager.unsubscribe(Salix::EventCategory::Application,&self);}; manager.subscribe(Salix::EventCategory::Application,&self); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(calls==1); CHECK(manager.is_queue_empty());
    }

    TEST_CASE("unsubscribing a later listener during delivery affects the next event not the current copy") {
        Salix::EventManager manager; Salix::MockListener first,second; int a=0,b=0; first.on_event_handler=[&](Salix::IEvent&){++a; manager.unsubscribe(Salix::EventCategory::Application,&second);}; second.on_event_handler=[&](Salix::IEvent&){++b;}; manager.subscribe(Salix::EventCategory::Application,&first); manager.subscribe(Salix::EventCategory::Application,&second); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(a==2); CHECK(b==1);
    }

    TEST_CASE("subscribing a new listener during delivery begins with the next queued event") {
        Salix::EventManager manager; Salix::MockListener first,late; int first_calls=0,late_calls=0; late.on_event_handler=[&](Salix::IEvent&){++late_calls;}; first.on_event_handler=[&](Salix::IEvent&){++first_calls; manager.subscribe(Salix::EventCategory::Application,&late);}; manager.subscribe(Salix::EventCategory::Application,&first); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(first_calls==2); CHECK(late_calls==1);
    }

    TEST_CASE("blocked events interleaved with queued events never disturb FIFO payload order") {
        Salix::EventManager manager; Salix::MockListener listener; std::vector<int> values; listener.on_event_handler=[&](Salix::IEvent& e){values.push_back(static_cast<NumberedApplicationEvent&>(e).sequence);}; manager.subscribe(Salix::EventCategory::Application,&listener); manager.dispatch(std::make_unique<NumberedApplicationEvent>(1)); manager.dispatch(std::make_unique<Salix::MockBlockingEvent>()); manager.dispatch(std::make_unique<NumberedApplicationEvent>(2)); manager.process_queue(); REQUIRE(values.size()==2); CHECK(values[0]==1); CHECK(values[1]==2);
    }

    TEST_CASE("const reference dispatch preserves the source object for reuse") {
        Salix::EventManager manager; Salix::MockListener listener; int sum=0; listener.on_event_handler=[&](Salix::IEvent& e){sum+=static_cast<NumberedApplicationEvent&>(e).sequence;}; manager.subscribe(Salix::EventCategory::Application,&listener); const NumberedApplicationEvent event(9); manager.dispatch(event); manager.dispatch(event); CHECK(event.sequence==9); manager.process_queue(); CHECK(sum==18);
    }

    TEST_CASE("removing and resubscribing a listener does not duplicate future delivery") {
        Salix::EventManager manager; Salix::MockListener listener; int calls=0; listener.on_event_handler=[&](Salix::IEvent&){++calls;}; manager.subscribe(Salix::EventCategory::Application,&listener); manager.unsubscribe(Salix::EventCategory::Application,&listener); manager.subscribe(Salix::EventCategory::Application,&listener); manager.subscribe(Salix::EventCategory::Application,&listener); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(calls==1);
    }

    TEST_CASE("handled state on one queued event does not leak into the next event object") {
        Salix::EventManager manager; Salix::MockListener first,second; int first_calls=0,second_calls=0; first.on_event_handler=[&](Salix::IEvent& e){++first_calls; if(first_calls==1)e.handled=true;}; second.on_event_handler=[&](Salix::IEvent&){++second_calls;}; manager.subscribe(Salix::EventCategory::Application,&first); manager.subscribe(Salix::EventCategory::Application,&second); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(first_calls==2); CHECK(second_calls==1);
    }
}


// ---- Relocated from src/tests/SalixEngine/events/IEvent.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/events/IEvent.h>
#include <Salix/events/BeforeElementPurgedEvent.h>
#include <Salix/events/BeforeEntityPurgedEvent.h>
#include <Salix/ecs/Entity.h>
#include <Salix/core/SimpleGuid.h>
#include <memory>
#include <string>
#include <utility>

namespace {
class Batch1347Event : public Salix::IEvent {
public:
    Batch1347Event(
        Salix::EventType type = Salix::EventType::AppTick,
        std::string name = "Batch1347Event",
        int categories = static_cast<int>(Salix::EventCategory::Application))
        : type_(type), name_(std::move(name)), categories_(categories) {}

    Salix::EventType get_event_type() const override { return type_; }
    const char* get_name() const override { return name_.c_str(); }
    int get_category_flags() const override { return categories_; }
    void* get_native_handle() override { return native_handle_; }
    bool should_block() const override { return block_; }
    void set_block(bool block) override { block_ = block; }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch1347Event>(*this); }

    void set_native_handle(void* handle) { native_handle_ = handle; }

private:
    Salix::EventType type_;
    std::string name_;
    int categories_;
    void* native_handle_ = nullptr;
    bool block_ = false;
};

class Batch1347DefaultEvent : public Salix::IEvent {
public:
    Salix::EventType get_event_type() const override { return Salix::EventType::AppUpdate; }
    const char* get_name() const override { return "DefaultEvent"; }
    int get_category_flags() const override { return static_cast<int>(Salix::EventCategory::Application); }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch1347DefaultEvent>(*this); }
};
}

TEST_SUITE("Salix::events::IEventBatch1347") {
    TEST_CASE("event default to_string delegates to event name") {
        Batch1347DefaultEvent event;
        CHECK(event.to_string() == "DefaultEvent");
    }

    TEST_CASE("event category bitwise helpers combine strongly typed flags") {
        const int flags = Salix::EventCategory::Input | Salix::EventCategory::Keyboard | Salix::EventCategory::Editor;
        CHECK((flags & static_cast<int>(Salix::EventCategory::Input)) != 0);
        CHECK((flags & static_cast<int>(Salix::EventCategory::Keyboard)) != 0);
        CHECK((flags & static_cast<int>(Salix::EventCategory::Editor)) != 0);
    }

    TEST_CASE("event application category helper reports configured membership") {
        Batch1347Event event;
        CHECK(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
    }

    TEST_CASE("event input and keyboard helpers can both be true") {
        const int categories = Salix::EventCategory::Input | Salix::EventCategory::Keyboard;
        Batch1347Event event(Salix::EventType::KeyPressed, "KeyPressed", categories);
        CHECK(event.is_input_event());
        CHECK(event.is_keyboard_event());
        CHECK_FALSE(event.is_mouse_event());
    }

    TEST_CASE("event mouse and mouse button helpers distinguish pointer categories") {
        const int categories = Salix::EventCategory::Input | Salix::EventCategory::Mouse | Salix::EventCategory::MouseButton;
        Batch1347Event event(Salix::EventType::MouseButtonPressed, "MouseButtonPressed", categories);
        CHECK(event.is_input_event());
        CHECK(event.is_mouse_event());
        CHECK(event.is_mouse_button_event());
        CHECK_FALSE(event.is_keyboard_event());
    }

    TEST_CASE("event none category is not reported as any concrete category") {
        Batch1347Event event(Salix::EventType::None, "None", static_cast<int>(Salix::EventCategory::None));
        CHECK_FALSE(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
        CHECK_FALSE(event.is_keyboard_event());
        CHECK_FALSE(event.is_mouse_event());
    }

    TEST_CASE("event interface default native handle and blocking behavior are neutral") {
        Batch1347DefaultEvent event;
        CHECK(event.get_native_handle() == nullptr);
        CHECK_FALSE(event.should_block());
        event.set_block(true);
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("event implementation can expose native handle and mutable blocking state") {
        Batch1347Event event;
        int native = 42;
        event.set_native_handle(&native);
        event.set_block(true);
        CHECK(event.get_native_handle() == &native);
        CHECK(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("event handled flag is independent of category and block state") {
        Batch1347Event event;
        CHECK_FALSE(event.handled);
        event.handled = true;
        CHECK(event.handled);
        CHECK(event.is_application_event());
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("event clone preserves dynamic contract and handled state") {
        Batch1347Event event(Salix::EventType::WindowClose, "CloseProbe",
            static_cast<int>(Salix::EventCategory::Application));
        event.handled = true;
        event.set_block(true);
        auto clone = event.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->get_event_type() == Salix::EventType::WindowClose);
        CHECK(std::string(clone->get_name()) == "CloseProbe");
        CHECK(clone->handled);
        CHECK(clone->should_block());
    }
}

namespace {
class Batch1847Event : public Salix::IEvent {
public:
    Batch1847Event(int flags, std::string name = "Batch1847Event") : flags_(flags), name_(std::move(name)) {}
    Salix::EventType get_event_type() const override { return Salix::EventType::AppUpdate; }
    const char* get_name() const override { return name_.c_str(); }
    int get_category_flags() const override { return flags_; }
    std::unique_ptr<Salix::IEvent> clone() const override { return std::make_unique<Batch1847Event>(*this); }
private:
    int flags_;
    std::string name_;
};
}

TEST_SUITE("Salix::events::IEventBatch1847") {
    TEST_CASE("two strongly typed categories combine into the expected integer mask") {
        const int flags = Salix::EventCategory::Application | Salix::EventCategory::Input;
        CHECK(flags == (static_cast<int>(Salix::EventCategory::Application) | static_cast<int>(Salix::EventCategory::Input)));
    }

    TEST_CASE("integer-left category operator supports chaining a third category") {
        const int first = Salix::EventCategory::Input | Salix::EventCategory::Keyboard;
        const int flags = first | Salix::EventCategory::Editor;
        CHECK((flags & static_cast<int>(Salix::EventCategory::Editor)) != 0);
        CHECK((flags & static_cast<int>(Salix::EventCategory::Keyboard)) != 0);
    }

    TEST_CASE("mouse-axis membership is queryable through generic category helper") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::MouseAxis));
        CHECK(event.is_in_category(Salix::EventCategory::MouseAxis));
        CHECK_FALSE(event.is_mouse_event());
    }

    TEST_CASE("editor and input categories can coexist without implying keyboard input") {
        const int flags = Salix::EventCategory::Input | Salix::EventCategory::Editor;
        Batch1847Event event(flags);
        CHECK(event.is_input_event());
        CHECK(event.is_in_category(Salix::EventCategory::Editor));
        CHECK_FALSE(event.is_keyboard_event());
    }

    TEST_CASE("mouse category alone does not imply mouse-button category") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::Mouse));
        CHECK(event.is_mouse_event());
        CHECK_FALSE(event.is_mouse_button_event());
    }

    TEST_CASE("combined concrete categories report each requested membership") {
        const int flags = Salix::EventCategory::Application | Salix::EventCategory::Input |
            Salix::EventCategory::Keyboard | Salix::EventCategory::Mouse | Salix::EventCategory::MouseButton;
        Batch1847Event event(flags);
        CHECK(event.is_application_event()); CHECK(event.is_input_event()); CHECK(event.is_keyboard_event());
        CHECK(event.is_mouse_event()); CHECK(event.is_mouse_button_event());
    }

    TEST_CASE("base blocking setter remains an intentional no-op") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::Application));
        CHECK_FALSE(event.should_block());
        event.set_block(true);
        CHECK_FALSE(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("base native handle remains null across unrelated state mutations") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::Application));
        event.handled = true;
        event.set_block(true);
        CHECK(event.get_native_handle() == nullptr);
    }

    TEST_CASE("clone preserves name category membership and handled flag") {
        Batch1847Event event(Salix::EventCategory::Input | Salix::EventCategory::Editor, "CloneProbe");
        event.handled = true;
        auto clone = event.clone();
        REQUIRE(clone != nullptr);
        CHECK(std::string(clone->get_name()) == "CloneProbe");
        CHECK(clone->is_input_event());
        CHECK(clone->is_in_category(Salix::EventCategory::Editor));
        CHECK(clone->handled);
    }

    TEST_CASE("cloned handled state is independent from the source after cloning") {
        Batch1847Event event(static_cast<int>(Salix::EventCategory::Application));
        auto clone = event.clone();
        REQUIRE(clone != nullptr);
        clone->handled = true;
        CHECK_FALSE(event.handled);
        CHECK(clone->handled);
    }
}

TEST_SUITE("Salix::events::BeforeElementPurgedEventBatch1147") {
    TEST_CASE("event preserves element and parent identifiers exactly") {
        const auto element = Salix::SimpleGuid::from_value(101);
        const auto parent = Salix::SimpleGuid::from_value(202);
        Salix::BeforeElementPurgedEvent event(element, parent);
        CHECK(event.element_id == element);
        CHECK(event.parent_id == parent);
    }

    TEST_CASE("event accepts invalid element identifier without altering valid parent identity") {
        const auto parent = Salix::SimpleGuid::from_value(22);
        Salix::BeforeElementPurgedEvent event(Salix::SimpleGuid::invalid(), parent);
        CHECK_FALSE(event.element_id.is_valid());
        CHECK(event.parent_id == parent);
    }

    TEST_CASE("event accepts invalid parent identifier without altering valid element identity") {
        const auto element = Salix::SimpleGuid::from_value(33);
        Salix::BeforeElementPurgedEvent event(element, Salix::SimpleGuid::invalid());
        CHECK(event.element_id == element);
        CHECK_FALSE(event.parent_id.is_valid());
    }

    TEST_CASE("static and virtual event type identities agree") {
        Salix::BeforeElementPurgedEvent event({}, {});
        CHECK(Salix::BeforeElementPurgedEvent::get_static_type() == Salix::EventType::BeforeElementPurged);
        CHECK(event.get_event_type() == Salix::BeforeElementPurgedEvent::get_static_type());
    }

    TEST_CASE("event name and default string representation identify BeforeElementPurged") {
        Salix::BeforeElementPurgedEvent event({}, {});
        CHECK(std::string(event.get_name()) == "BeforeElementPurged");
        CHECK(event.to_string() == "BeforeElementPurged");
    }

    TEST_CASE("purge notification exposes exactly the editor category") {
        Salix::BeforeElementPurgedEvent event({}, {});
        CHECK(event.get_category_flags() == static_cast<int>(Salix::EventCategory::Editor));
        CHECK(event.is_in_category(Salix::EventCategory::Editor));
        CHECK_FALSE(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
    }

    TEST_CASE("clone preserves both identifiers and concrete event type") {
        const auto element = Salix::SimpleGuid::from_value(77);
        const auto parent = Salix::SimpleGuid::from_value(88);
        Salix::BeforeElementPurgedEvent source(element, parent);
        std::unique_ptr<Salix::IEvent> clone = source.clone();
        REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::BeforeElementPurgedEvent*>(clone.get());
        REQUIRE(concrete != nullptr);
        CHECK(concrete->element_id == element);
        CHECK(concrete->parent_id == parent);
    }

    TEST_CASE("clone copies handled state while remaining independently mutable") {
        Salix::BeforeElementPurgedEvent source({}, {});
        source.handled = true;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->handled);
        clone->handled = false;
        CHECK(source.handled);
        CHECK_FALSE(clone->handled);
    }

    TEST_CASE("editor purge event uses default null native handle") {
        Salix::BeforeElementPurgedEvent event({}, {});
        Salix::IEvent* base = &event;
        CHECK(base->get_native_handle() == nullptr);
    }

    TEST_CASE("editor purge event retains default nonblocking contract") {
        Salix::BeforeElementPurgedEvent event({}, {});
        CHECK_FALSE(event.should_block());
        event.set_block(true);
        CHECK_FALSE(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }
}


TEST_SUITE("Salix::events::BeforeElementPurgedEventBatch2047") {
    TEST_CASE("event payload identifiers can be retargeted independently after construction") {
        Salix::BeforeElementPurgedEvent event(Salix::SimpleGuid::from_value(1), Salix::SimpleGuid::from_value(2));
        event.element_id = Salix::SimpleGuid::from_value(3);
        CHECK(event.element_id == Salix::SimpleGuid::from_value(3));
        CHECK(event.parent_id == Salix::SimpleGuid::from_value(2));
    }
    TEST_CASE("copy construction preserves payload and handled state") {
        Salix::BeforeElementPurgedEvent source(Salix::SimpleGuid::from_value(10), Salix::SimpleGuid::from_value(20));
        source.handled = true;
        Salix::BeforeElementPurgedEvent copy(source);
        CHECK(copy.element_id == source.element_id); CHECK(copy.parent_id == source.parent_id); CHECK(copy.handled);
    }
    TEST_CASE("copy assignment replaces both identifiers and handled state") {
        Salix::BeforeElementPurgedEvent source(Salix::SimpleGuid::from_value(30), Salix::SimpleGuid::from_value(40));
        source.handled = true;
        Salix::BeforeElementPurgedEvent destination(Salix::SimpleGuid::from_value(1), Salix::SimpleGuid::from_value(2));
        destination = source;
        CHECK(destination.element_id == Salix::SimpleGuid::from_value(30));
        CHECK(destination.parent_id == Salix::SimpleGuid::from_value(40)); CHECK(destination.handled);
    }
    TEST_CASE("clone preserves two invalid payload identifiers") {
        Salix::BeforeElementPurgedEvent source(Salix::SimpleGuid::invalid(), Salix::SimpleGuid::invalid());
        auto clone = source.clone(); REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::BeforeElementPurgedEvent*>(clone.get()); REQUIRE(concrete != nullptr);
        CHECK_FALSE(concrete->element_id.is_valid()); CHECK_FALSE(concrete->parent_id.is_valid());
    }
    TEST_CASE("mutating cloned identifiers never changes source payload") {
        Salix::BeforeElementPurgedEvent source(Salix::SimpleGuid::from_value(50), Salix::SimpleGuid::from_value(60));
        auto clone = source.clone(); auto* concrete = dynamic_cast<Salix::BeforeElementPurgedEvent*>(clone.get()); REQUIRE(concrete != nullptr);
        concrete->element_id = Salix::SimpleGuid::from_value(500); concrete->parent_id = Salix::SimpleGuid::from_value(600);
        CHECK(source.element_id == Salix::SimpleGuid::from_value(50)); CHECK(source.parent_id == Salix::SimpleGuid::from_value(60));
    }
    TEST_CASE("base reference dispatch preserves purge type name and editor category") {
        Salix::BeforeElementPurgedEvent event({}, {}); Salix::IEvent& base = event;
        CHECK(base.get_event_type() == Salix::EventType::BeforeElementPurged);
        CHECK(std::string(base.get_name()) == "BeforeElementPurged");
        CHECK(base.get_category_flags() == static_cast<int>(Salix::EventCategory::Editor));
    }
    TEST_CASE("handled state changes do not mutate purge payload identifiers") {
        Salix::BeforeElementPurgedEvent event(Salix::SimpleGuid::from_value(70), Salix::SimpleGuid::from_value(80));
        event.handled = true; event.handled = false;
        CHECK(event.element_id == Salix::SimpleGuid::from_value(70)); CHECK(event.parent_id == Salix::SimpleGuid::from_value(80));
    }
    TEST_CASE("string representation remains stable after payload retargeting") {
        Salix::BeforeElementPurgedEvent event({}, {});
        event.element_id = Salix::SimpleGuid::from_value(90); event.parent_id = Salix::SimpleGuid::from_value(91);
        CHECK(event.to_string() == "BeforeElementPurged");
    }
    TEST_CASE("separate purge events own independent identifier values") {
        Salix::BeforeElementPurgedEvent first(Salix::SimpleGuid::from_value(100), Salix::SimpleGuid::from_value(200));
        Salix::BeforeElementPurgedEvent second = first; second.element_id = Salix::SimpleGuid::from_value(101);
        CHECK(first.element_id == Salix::SimpleGuid::from_value(100)); CHECK(second.element_id == Salix::SimpleGuid::from_value(101));
        CHECK(first.parent_id == second.parent_id);
    }
    TEST_CASE("self assignment preserves purge payload and handled state") {
        Salix::BeforeElementPurgedEvent event(Salix::SimpleGuid::from_value(111), Salix::SimpleGuid::from_value(222));
        event.handled = true; event = event;
        CHECK(event.element_id == Salix::SimpleGuid::from_value(111)); CHECK(event.parent_id == Salix::SimpleGuid::from_value(222)); CHECK(event.handled);
    }
}

TEST_SUITE("Salix::events::BeforeEntityPurgedEventBatch1047") {
    TEST_CASE("event preserves the exact entity pointer supplied at construction") {
        Salix::Entity entity;
        Salix::BeforeEntityPurgedEvent event(&entity);
        CHECK(event.entity == &entity);
    }

    TEST_CASE("event deliberately supports a null entity payload without changing metadata") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(event.entity == nullptr);
        CHECK(event.get_event_type() == Salix::EventType::BeforeEntityPurged);
        CHECK(std::string(event.get_name()) == "BeforeEntityPurged");
    }

    TEST_CASE("static and virtual event type identities agree") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(Salix::BeforeEntityPurgedEvent::get_static_type() == Salix::EventType::BeforeEntityPurged);
        CHECK(event.get_event_type() == Salix::BeforeEntityPurgedEvent::get_static_type());
    }

    TEST_CASE("default event string representation uses the concrete event name") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(std::string(event.get_name()) == "BeforeEntityPurged");
        CHECK(event.to_string() == "BeforeEntityPurged");
    }

    TEST_CASE("event exposes exactly the editor category flag") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(event.get_category_flags() == static_cast<int>(Salix::EventCategory::Editor));
    }

    TEST_CASE("category helpers classify purge notification as editor only") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK(event.is_in_category(Salix::EventCategory::Editor));
        CHECK_FALSE(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
        CHECK_FALSE(event.is_keyboard_event());
        CHECK_FALSE(event.is_mouse_event());
        CHECK_FALSE(event.is_mouse_button_event());
    }

    TEST_CASE("clone preserves concrete type and entity pointer payload") {
        Salix::Entity entity;
        Salix::BeforeEntityPurgedEvent source(&entity);
        std::unique_ptr<Salix::IEvent> clone = source.clone();
        REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::BeforeEntityPurgedEvent*>(clone.get());
        REQUIRE(concrete != nullptr);
        CHECK(concrete->entity == &entity);
        CHECK(concrete->get_event_type() == Salix::EventType::BeforeEntityPurged);
    }

    TEST_CASE("clone copies handled state but remains independently mutable") {
        Salix::BeforeEntityPurgedEvent source(nullptr);
        source.handled = true;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->handled);
        clone->handled = false;
        CHECK(source.handled);
        CHECK_FALSE(clone->handled);
    }

    TEST_CASE("event inherits null native handle contract for non-native editor events") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        Salix::IEvent* base = &event;
        CHECK(base->get_native_handle() == nullptr);
    }

    TEST_CASE("editor purge event uses default nonblocking contract") {
        Salix::BeforeEntityPurgedEvent event(nullptr);
        CHECK_FALSE(event.should_block());
        event.set_block(true);
        CHECK_FALSE(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }
}


TEST_SUITE("Salix::events::BeforeEntityPurgedEventBatch2047") {
    TEST_CASE("entity payload pointer can be retargeted after construction") {
        Salix::Entity first; Salix::Entity second; Salix::BeforeEntityPurgedEvent event(&first); event.entity = &second; CHECK(event.entity == &second);
    }
    TEST_CASE("copy construction preserves entity pointer and handled state") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent source(&entity); source.handled = true;
        Salix::BeforeEntityPurgedEvent copy(source); CHECK(copy.entity == &entity); CHECK(copy.handled);
    }
    TEST_CASE("copy assignment replaces entity pointer and handled state") {
        Salix::Entity first; Salix::Entity second; Salix::BeforeEntityPurgedEvent source(&first); source.handled = true;
        Salix::BeforeEntityPurgedEvent destination(&second); destination = source; CHECK(destination.entity == &first); CHECK(destination.handled);
    }
    TEST_CASE("clone preserves an explicit null entity payload") {
        Salix::BeforeEntityPurgedEvent source(nullptr); auto clone = source.clone(); REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::BeforeEntityPurgedEvent*>(clone.get()); REQUIRE(concrete != nullptr); CHECK(concrete->entity == nullptr);
    }
    TEST_CASE("retargeting cloned entity pointer does not alter source event") {
        Salix::Entity first; Salix::Entity second; Salix::BeforeEntityPurgedEvent source(&first);
        auto clone = source.clone(); auto* concrete = dynamic_cast<Salix::BeforeEntityPurgedEvent*>(clone.get()); REQUIRE(concrete != nullptr);
        concrete->entity = &second; CHECK(source.entity == &first); CHECK(concrete->entity == &second);
    }
    TEST_CASE("base reference dispatch preserves entity purge metadata") {
        Salix::BeforeEntityPurgedEvent event(nullptr); Salix::IEvent& base = event;
        CHECK(base.get_event_type() == Salix::EventType::BeforeEntityPurged); CHECK(std::string(base.get_name()) == "BeforeEntityPurged");
        CHECK(base.get_category_flags() == static_cast<int>(Salix::EventCategory::Editor));
    }
    TEST_CASE("handled state remains independent from entity pointer payload") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent event(&entity); event.handled = true; CHECK(event.entity == &entity);
        event.handled = false; CHECK(event.entity == &entity);
    }
    TEST_CASE("two purge events may reference the same entity while tracking handled state separately") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent first(&entity); Salix::BeforeEntityPurgedEvent second(&entity);
        first.handled = true; CHECK(first.entity == second.entity); CHECK(first.handled); CHECK_FALSE(second.handled);
    }
    TEST_CASE("self assignment preserves entity pointer and handled state") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent event(&entity); event.handled = true; event = event; CHECK(event.entity == &entity); CHECK(event.handled);
    }
    TEST_CASE("string representation remains stable after entity pointer changes") {
        Salix::Entity entity; Salix::BeforeEntityPurgedEvent event(nullptr); event.entity = &entity; CHECK(event.to_string() == "BeforeEntityPurged");
        event.entity = nullptr; CHECK(event.to_string() == "BeforeEntityPurged");
    }
}


// ---- Relocated from src/tests/SalixEngine/events/ImGuiInputEvent.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/events/ImGuiInputEvent.h>
#include <Salix/events/IEvent.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::events::ImGuiInputEventBatch1047") {
    TEST_CASE("no-capture event reports neither mouse nor keyboard capture") {
        Salix::ImGuiInputEvent event(false, false);
        CHECK_FALSE(event.is_mouse_captured());
        CHECK_FALSE(event.is_keyboard_captured());
    }

    TEST_CASE("mouse-only capture remains independent from keyboard capture") {
        Salix::ImGuiInputEvent event(true, false);
        CHECK(event.is_mouse_captured());
        CHECK_FALSE(event.is_keyboard_captured());
    }

    TEST_CASE("keyboard-only capture remains independent from mouse capture") {
        Salix::ImGuiInputEvent event(false, true);
        CHECK_FALSE(event.is_mouse_captured());
        CHECK(event.is_keyboard_captured());
    }

    TEST_CASE("simultaneous capture reports both channels") {
        Salix::ImGuiInputEvent event(true, true);
        CHECK(event.is_mouse_captured());
        CHECK(event.is_keyboard_captured());
    }

    TEST_CASE("string representation reports disabled capture channels as zero") {
        Salix::ImGuiInputEvent event(false, false);
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 0, Keyboard Captured = 0");
    }

    TEST_CASE("string representation reports enabled capture channels as one") {
        Salix::ImGuiInputEvent event(true, true);
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 1, Keyboard Captured = 1");
    }

    TEST_CASE("static type virtual type and event name remain consistent") {
        Salix::ImGuiInputEvent event(false, true);
        CHECK(Salix::ImGuiInputEvent::get_static_type() == Salix::EventType::ImGuiInput);
        CHECK(event.get_event_type() == Salix::EventType::ImGuiInput);
        CHECK(std::string(event.get_name()) == "ImGuiInput");
    }

    TEST_CASE("ImGui capture event is classified as input but not unrelated categories") {
        Salix::ImGuiInputEvent event(true, false);
        CHECK(event.get_category_flags() == static_cast<int>(Salix::EventCategory::Input));
        CHECK(event.is_input_event());
        CHECK_FALSE(event.is_application_event());
        CHECK_FALSE(event.is_keyboard_event());
        CHECK_FALSE(event.is_mouse_event());
        CHECK_FALSE(event.is_mouse_button_event());
    }

    TEST_CASE("clone preserves capture flags concrete type and handled state") {
        Salix::ImGuiInputEvent source(true, false);
        source.handled = true;
        std::unique_ptr<Salix::IEvent> clone = source.clone();
        REQUIRE(clone != nullptr);
        auto* concrete = dynamic_cast<Salix::ImGuiInputEvent*>(clone.get());
        REQUIRE(concrete != nullptr);
        CHECK(concrete->is_mouse_captured());
        CHECK_FALSE(concrete->is_keyboard_captured());
        CHECK(concrete->handled);
        concrete->handled = false;
        CHECK(source.handled);
    }

    TEST_CASE("non-native ImGui capture event retains default blocking and native-handle contracts") {
        Salix::ImGuiInputEvent event(false, true);
        Salix::IEvent* base = &event;
        CHECK(base->get_native_handle() == nullptr);
        CHECK_FALSE(base->should_block());
        base->set_block(true);
        CHECK_FALSE(base->should_block());
        CHECK(base->get_event_type() == Salix::EventType::ImGuiInput);
    }
}

TEST_SUITE("Salix::events::ImGuiInputEventBatch1847") {
    TEST_CASE("mouse-only string form records one and zero capture flags") {
        Salix::ImGuiInputEvent event(true, false);
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 1, Keyboard Captured = 0");
    }

    TEST_CASE("keyboard-only string form records zero and one capture flags") {
        Salix::ImGuiInputEvent event(false, true);
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 0, Keyboard Captured = 1");
    }

    TEST_CASE("fresh ImGui input event starts unhandled independently of capture state") {
        Salix::ImGuiInputEvent event(true, true);
        CHECK_FALSE(event.handled);
        CHECK(event.is_mouse_captured());
        CHECK(event.is_keyboard_captured());
    }

    TEST_CASE("marking event handled never changes immutable capture flags") {
        Salix::ImGuiInputEvent event(true, false);
        event.handled = true;
        CHECK(event.handled);
        CHECK(event.is_mouse_captured());
        CHECK_FALSE(event.is_keyboard_captured());
    }

    TEST_CASE("clone of no-capture event remains independently unhandled") {
        Salix::ImGuiInputEvent source(false, false);
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        clone->handled = true;
        CHECK_FALSE(source.handled);
        CHECK(clone->handled);
        auto* concrete = dynamic_cast<Salix::ImGuiInputEvent*>(clone.get());
        REQUIRE(concrete != nullptr);
        CHECK_FALSE(concrete->is_mouse_captured());
        CHECK_FALSE(concrete->is_keyboard_captured());
    }

    TEST_CASE("clone of full-capture event preserves full string representation") {
        Salix::ImGuiInputEvent source(true, true);
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->to_string() == source.to_string());
    }

    TEST_CASE("repeated string conversion is stable and side-effect free") {
        Salix::ImGuiInputEvent event(true, false);
        const auto first = event.to_string();
        const auto second = event.to_string();
        CHECK(first == second);
        CHECK(event.is_mouse_captured());
        CHECK_FALSE(event.is_keyboard_captured());
    }

    TEST_CASE("base interface virtual to_string dispatches to ImGui-specific representation") {
        Salix::ImGuiInputEvent concrete(false, true);
        Salix::IEvent& event = concrete;
        CHECK(event.to_string() == "ImGuiInputEvent: Mouse Captured = 0, Keyboard Captured = 1");
        CHECK(event.get_event_type() == Salix::EventType::ImGuiInput);
    }

    TEST_CASE("input classification does not leak into editor or mouse-axis categories") {
        Salix::ImGuiInputEvent event(true, true);
        CHECK(event.is_input_event());
        CHECK_FALSE(event.is_in_category(Salix::EventCategory::Editor));
        CHECK_FALSE(event.is_in_category(Salix::EventCategory::MouseAxis));
    }

    TEST_CASE("default blocking contract remains false through base interface and clone") {
        Salix::ImGuiInputEvent source(true, false);
        Salix::IEvent& base = source;
        base.set_block(true);
        CHECK_FALSE(base.should_block());
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK_FALSE(clone->should_block());
    }
}
