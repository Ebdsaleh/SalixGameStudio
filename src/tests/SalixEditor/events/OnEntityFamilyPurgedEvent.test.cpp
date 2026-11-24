// Tests for Editor/events/OnEntityFamilyPurgedEvent.h.
#include "../fixtures/EditorEventFixtures.h"
#include <Editor/events/OnEntityFamilyPurgedEvent.h>

TEST_SUITE("EditorCRI013_OnEntityFamilyPurgedEvent") {
    using namespace EditorCri;
    using Capture=EditorEventCapture<OnEntityFamilyPurgedEvent>;
    using Fixture=EditorEventFixture<OnEntityFamilyPurgedEvent>;
    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: empty purge payload has no identities") {
        OnEntityFamilyPurgedEvent e({});CHECK(e.entity_ids.empty());
    }

    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: purge payload preserves order and full-width IDs") {
        OnEntityFamilyPurgedEvent e({id(11),id(0x100000001ULL),id(13)});REQUIRE(e.entity_ids.size()==3);CHECK(e.entity_ids[0]==id(11));CHECK(e.entity_ids[1]==id(0x100000001ULL));CHECK(e.entity_ids[2]==id(13));
    }

    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: event owns vector independently of caller") {
        std::vector<SimpleGuid> source{id(11),id(12)};OnEntityFamilyPurgedEvent e(source);source.clear();REQUIRE(e.entity_ids.size()==2);CHECK(e.entity_ids[1]==id(12));
    }

    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: clone owns independent removed-ID list") {
        OnEntityFamilyPurgedEvent e({id(11),id(12)});e.handled=true;auto c=e.clone();auto* p=dynamic_cast<OnEntityFamilyPurgedEvent*>(c.get());REQUIRE(p);CHECK(p->handled);p->entity_ids.clear();CHECK(e.entity_ids.size()==2);
    }

    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: queued purge payload survives source destruction") {
        Fixture f;{OnEntityFamilyPurgedEvent e({id(11),id(12)});f.context.events.dispatch(e);}f.drain();REQUIRE(f.capture.count()==1);CHECK((f.capture.at(0).entity_ids==std::vector<SimpleGuid>{id(11),id(12)}));
    }

    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: subtree purge emits exactly removed family identities") {
        Fixture f;f.root(11);f.child(12,11);f.child(13,12);f.root(14);f.drain();f.manager().purge_entity_and_family(id(12));f.drain();REQUIRE(f.capture.count()==1);CHECK((f.capture.at(0).entity_ids==std::vector<SimpleGuid>{id(12),id(13)}));CHECK(f.manager().get_archetype(id(11))!=nullptr);CHECK(f.manager().get_archetype(id(14))!=nullptr);
    }

    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: bloodline purge reports ancestors and descendants") {
        Fixture f;f.root(11);f.child(12,11);f.child(13,12);f.root(14);f.drain();f.manager().purge_entity_bloodline(id(13));f.drain();REQUIRE(f.capture.count()==1);CHECK((f.capture.at(0).entity_ids==std::vector<SimpleGuid>{id(11),id(12),id(13)}));CHECK(f.manager().get_realm_size()==1);
    }

    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: descendant purge excludes retained parent from notification") {
        Fixture f;f.root(11);f.child(12,11);f.child(13,12);f.drain();f.manager().purge_entity_descendants(id(11));f.drain();REQUIRE(f.capture.count()==1);CHECK((f.capture.at(0).entity_ids==std::vector<SimpleGuid>{id(12),id(13)}));CHECK(f.manager().get_archetype(id(11))!=nullptr);
    }

    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: successive independent purges retain separate event payloads") {
        Fixture f;f.root(11);f.root(12);f.drain();f.manager().purge_entity_and_family(id(11));f.manager().purge_entity_and_family(id(12));f.drain();REQUIRE(f.capture.count()==2);CHECK((f.capture.at(0).entity_ids==std::vector<SimpleGuid>{id(11)}));CHECK((f.capture.at(1).entity_ids==std::vector<SimpleGuid>{id(12)}));
    }

    TEST_CASE("CRI013 OnEntityFamilyPurgedEvent: family purge event has Editor metadata and no input routing") {
        OnEntityFamilyPurgedEvent e({id(11)});CHECK(e.get_event_type()==EventType::EditorOnEntityFamilyPurged);CHECK(std::string(e.get_name())=="EditorOnEntityFamilyPurged");CHECK(e.is_in_category(EventCategory::Editor));CHECK_FALSE(e.is_input_event());Capture c;EventManager m;m.subscribe(EventCategory::Input,&c);m.dispatch(e);m.process_queue();CHECK(c.count()==0);
    }
}
