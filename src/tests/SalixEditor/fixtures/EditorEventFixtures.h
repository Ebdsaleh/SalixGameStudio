#pragma once
#include "HeadlessGuiFixtures.h"
#include <Salix/events/EventManager.h>
#include <Salix/events/IEventListener.h>

namespace EditorCri {
    // Capture real Editor notifications without invoking GUI or mutating the event queue.
    template<class Event> struct EditorEventCapture : IEventListener {
        std::vector<std::unique_ptr<IEvent>> received;
        bool consume=false;
        void on_event(IEvent& e) override {
            if(e.get_event_type()!=Event::get_static_type()) return;
            received.push_back(e.clone());
            if(consume)e.handled=true;
        }
        Event& at(size_t index) {return static_cast<Event&>(*received.at(index));}
        size_t count() const {return received.size();}
    };
    template<class Event> struct EditorEventFixture {
        EditorEventCapture<Event> capture;
        PanelContext context;
        EditorEventFixture(){context.events.subscribe(EventCategory::Editor,&capture);}
        EditorRealmManager& manager(){return *context.ctx.editor_realm_manager;}
        void root(uint64_t n){manager().add_entity(entity(n,"Root"));}
        void child(uint64_t n,uint64_t parent){auto a=entity(n,"Child");a.parent_id=id(parent);manager().add_child_entity(a);}
        void drain(){context.events.process_queue();}
    };
}
