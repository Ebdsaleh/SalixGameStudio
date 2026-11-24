// CRI relay 003: EditorContext; uncovered paths and behavioral regressions.
#include "CriRelay003Fixtures.h"

namespace {
using namespace EditorCri;

}
TEST_SUITE("EditorCRI003_EditorContext") {
    TEST_CASE("CRI003 EditorContext: context starts without borrowed services") {
        EditorContext c;CHECK(c.init_context==nullptr);CHECK(c.renderer==nullptr);CHECK(c.event_manager==nullptr);CHECK(c.preview_realm==nullptr);
    }

    TEST_CASE("CRI003 EditorContext: context starts in YAML mode with no selection") {
        EditorContext c;CHECK(c.data_mode==EditorDataMode::Yaml);CHECK_FALSE(c.selected_entity_id.is_valid());CHECK_FALSE(c.selected_element_id.is_valid());CHECK(c.selected_entity==nullptr);
    }

    TEST_CASE("CRI003 EditorContext: deferred command is stored without immediate execution") {
        EditorContext c;int n=0;c.add_deferred_command([&]{++n;});CHECK(n==0);CHECK_FALSE(c.is_deferred_commands_empty());REQUIRE(c.deferred_commands.size()==1);c.deferred_commands[0]();CHECK(n==1);
    }

    TEST_CASE("CRI003 EditorContext: queued commands retain insertion order") {
        EditorContext c;std::vector<int> seen;c.add_deferred_command([&]{seen.push_back(1);});c.add_deferred_command([&]{seen.push_back(2);});for(auto& cmd:c.deferred_commands)cmd();REQUIRE(seen.size()==2);CHECK(seen[0]==1);CHECK(seen[1]==2);
    }

    TEST_CASE("CRI003 EditorContext: clearing deferred commands does not execute them") {
        EditorContext c;int n=0;c.add_deferred_command([&]{++n;});c.clear_deferred_commands();CHECK(n==0);CHECK(c.is_deferred_commands_empty());
    }

    TEST_CASE("CRI003 EditorContext: clearing deferred queue leaves synchronization queue intact") {
        EditorContext c;c.sync_queue.push_back([]{});c.add_deferred_command([]{});c.clear_deferred_commands();CHECK(c.is_deferred_commands_empty());CHECK(c.sync_queue.size()==1);
    }

    TEST_CASE("CRI003 EditorContext: independent contexts do not share command queues") {
        EditorContext a,b;a.add_deferred_command([]{});CHECK_FALSE(a.is_deferred_commands_empty());CHECK(b.is_deferred_commands_empty());
    }

    TEST_CASE("CRI003 EditorContext: commands retain value captures after original changes") {
        EditorContext c;int original=3,seen=0;c.add_deferred_command([original,&seen]{seen=original;});original=9;c.deferred_commands[0]();CHECK(seen==3);
    }

    TEST_CASE("CRI003 EditorContext: clearing queue releases owned captures") {
        EditorContext c;auto p=std::make_shared<int>(7);std::weak_ptr<int> weak=p;c.add_deferred_command([p]{});p.reset();CHECK_FALSE(weak.expired());c.clear_deferred_commands();CHECK(weak.expired());
    }

    TEST_CASE("CRI003 EditorContext: cleared queue can accept subsequent frame commands") {
        EditorContext c;int n=0;c.add_deferred_command([&]{n=1;});c.clear_deferred_commands();c.add_deferred_command([&]{n=2;});REQUIRE(c.deferred_commands.size()==1);c.deferred_commands[0]();CHECK(n==2);
    }
}
