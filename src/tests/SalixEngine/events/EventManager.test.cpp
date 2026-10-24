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

TEST_SUITE("Salix::events::EventManagerBatch2347") {
    TEST_CASE("batch2347 duplicate subscription in one category delivers exactly once") {
        Salix::EventManager manager; Salix::MockListener listener; int calls=0; listener.on_event_handler=[&](Salix::IEvent&){++calls;};
        manager.subscribe(Salix::EventCategory::Application,&listener); manager.subscribe(Salix::EventCategory::Application,&listener); manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(calls==1);
    }
    TEST_CASE("batch2347 unsubscribe removes listener from category until explicitly resubscribed") {
        Salix::EventManager manager; Salix::MockListener listener; int calls=0; listener.on_event_handler=[&](Salix::IEvent&){++calls;};
        manager.subscribe(Salix::EventCategory::Application,&listener); manager.unsubscribe(Salix::EventCategory::Application,&listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(calls==0);
        manager.subscribe(Salix::EventCategory::Application,&listener); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(calls==1);
    }
    TEST_CASE("batch2347 empty queue processing remains empty") {
        Salix::EventManager manager; CHECK(manager.is_queue_empty()); CHECK_NOTHROW(manager.process_queue()); CHECK(manager.is_queue_empty());
    }
    TEST_CASE("batch2347 const reference dispatch queues a clone rather than delivering synchronously") {
        Salix::EventManager manager; Salix::MockListener listener; int calls=0; listener.on_event_handler=[&](Salix::IEvent&){++calls;}; manager.subscribe(Salix::EventCategory::Application,&listener);
        Salix::MockEvent event; manager.dispatch(event); CHECK(calls==0); CHECK_FALSE(manager.is_queue_empty()); manager.process_queue(); CHECK(calls==1); CHECK(manager.is_queue_empty());
    }
    TEST_CASE("batch2347 handled first listener prevents second listener in same category") {
        Salix::EventManager manager; Salix::MockListener first,second; int first_calls=0,second_calls=0;
        first.on_event_handler=[&](Salix::IEvent& e){++first_calls; e.handled=true;}; second.on_event_handler=[&](Salix::IEvent&){++second_calls;};
        manager.subscribe(Salix::EventCategory::Application,&first); manager.subscribe(Salix::EventCategory::Application,&second);
        manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(first_calls==1); CHECK(second_calls==0);
    }
    TEST_CASE("batch2347 one handled event does not prevent later queued event from processing") {
        Salix::EventManager manager; Salix::MockListener listener; int calls=0;
        listener.on_event_handler=[&](Salix::IEvent& e){++calls; if(calls==1)e.handled=true;};
        manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue();
        CHECK(calls==2); CHECK(manager.is_queue_empty());
    }
    TEST_CASE("batch2347 blocking unique event is discarded immediately and queue remains empty") {
        Salix::EventManager manager; auto event=std::make_unique<Salix::MockBlockingEvent>(); manager.dispatch(std::move(event)); CHECK(manager.is_queue_empty());
    }
    TEST_CASE("batch2347 unsubscribing nonexistent category from fresh manager is harmless") {
        Salix::EventManager manager; Salix::MockListener listener; CHECK_NOTHROW(manager.unsubscribe(Salix::EventCategory::Editor,&listener)); CHECK(manager.is_queue_empty());
    }
    TEST_CASE("batch2347 subscription survives repeated empty process cycles") {
        Salix::EventManager manager; Salix::MockListener listener; int calls=0; listener.on_event_handler=[&](Salix::IEvent&){++calls;}; manager.subscribe(Salix::EventCategory::Application,&listener);
        manager.process_queue(); manager.process_queue(); manager.process_queue(); manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(calls==1);
    }
    TEST_CASE("batch2347 listener can unsubscribe itself during notification without invalidating dispatch") {
        Salix::EventManager manager; Salix::MockListener self,other; int self_calls=0,other_calls=0;
        self.on_event_handler=[&](Salix::IEvent&){++self_calls; manager.unsubscribe(Salix::EventCategory::Application,&self);};
        other.on_event_handler=[&](Salix::IEvent&){++other_calls;};
        manager.subscribe(Salix::EventCategory::Application,&self); manager.subscribe(Salix::EventCategory::Application,&other);
        manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(self_calls==1); CHECK(other_calls==1);
        manager.dispatch(std::make_unique<Salix::MockEvent>()); manager.process_queue(); CHECK(self_calls==1); CHECK(other_calls==2);
    }
}

