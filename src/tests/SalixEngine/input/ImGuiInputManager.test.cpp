// =================================================================================
// Filename:    src/Tests/SalixEngine/input/ImGuiInputManager.test.cpp
// Description: Contains unit tests for the ImGuiInputManager.
// ================================================================================= 

#include <doctest.h>
#include <Salix/input/ImGuiInputManager.h>
#include <Tests/TestFixtures.h> // <-- Include the updated fixtures file
#include <imgui.h>
#include <SDL.h>

// The ImGuiTestFixture struct definition is now removed from this file.

TEST_SUITE("Salix::input::ImGuiInputManager") {

    TEST_CASE("Minimal ImGui Context Creation") {
        // This is the most basic ImGui initialization.
        // If this fails, the issue is definitely in the build configuration.
        ImGuiContext* ctx = ImGui::CreateContext();
        CHECK(ctx != nullptr);
        ImGui::DestroyContext(ctx);
    }
    
}

// Pure ImGui input context: no SDL window or graphics backend required.
#include <vector>
#include <Tests/SalixEngine/mocking/events/MockEventSystem.h>
namespace {
struct InputManagerContext {
    ImGuiContext* previous_test = ImGui::GetCurrentContext();
    ImGuiContext* previous_engine = Salix::ImGuiInputManager::get_context_for_testing();
    ImGuiContext* owned = ImGui::CreateContext();

    InputManagerContext() {
        // A copy of ImGui's global current-context pointer may live in
        // SalixEngine.dll when it is statically linked into both modules.
        // Ensure the exported input manager sees this same owned context.
        Salix::ImGuiInputManager::set_context_for_testing(owned);
    }

    ~InputManagerContext() {
        // Restore the engine's context before destroying the owned context:
        // no dangling DLL-side GImGui survives between CTest cases.
        Salix::ImGuiInputManager::set_context_for_testing(previous_engine);
        ImGui::DestroyContext(owned);
        ImGui::SetCurrentContext(previous_test);
    }
};
}
TEST_SUITE("Salix::input::ImGuiInputManagerExtended") {
    TEST_CASE("imgui input default scroll state does not report wheel activity") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        CHECK(manager.get_mouse_scroll_delta() == 0.f);
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Forward));
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Backward));
    }

    TEST_CASE("imgui input positive scroll is classified as forward only") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MouseWheel = 1.5f;
        CHECK(manager.did_scroll(Salix::MouseScroll::Forward));
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Backward));
        CHECK(manager.get_mouse_scroll_delta() == doctest::Approx(1.5f));
    }

    TEST_CASE("imgui input negative scroll is classified as backward only") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MouseWheel = -2.25f;
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Forward));
        CHECK(manager.did_scroll(Salix::MouseScroll::Backward));
        CHECK(manager.get_mouse_scroll_delta() == doctest::Approx(-2.25f));
    }

    TEST_CASE("imgui input changing scroll sign immediately changes reported direction") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MouseWheel = 3.f;
        CHECK(manager.did_scroll(Salix::MouseScroll::Forward));
        ImGui::GetIO().MouseWheel = -1.f;
        CHECK(manager.did_scroll(Salix::MouseScroll::Backward));
        ImGui::GetIO().MouseWheel = 0.f;
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Forward));
    }

    TEST_CASE("imgui input mouse coordinates convert floating point position toward zero") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MousePos = ImVec2(117.9f, -23.6f);
        int x = 0, y = 0;
        manager.get_mouse_position(&x, &y);
        CHECK(x == 117);
        CHECK(y == -23);
    }

    TEST_CASE("imgui input mouse position query permits a missing X output pointer") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MousePos = ImVec2(10.f, 25.f);
        int y = -1;
        manager.get_mouse_position(nullptr, &y);
        CHECK(y == 25);
    }

    TEST_CASE("imgui input mouse position query permits a missing Y output pointer") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MousePos = ImVec2(-11.f, 9.f);
        int x = 0;
        manager.get_mouse_position(&x, nullptr);
        CHECK(x == -11);
        CHECK_NOTHROW(manager.get_mouse_position(nullptr, nullptr));
    }

    TEST_CASE("imgui input raw engine events do not mutate UI scroll state or request quit") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MouseWheel = 0.75f;
        Salix::MockEvent event;
        manager.process_event(event);
        manager.update(0.25f);
        CHECK(manager.get_mouse_scroll_delta() == doctest::Approx(0.75f));
        CHECK_FALSE(manager.wants_to_quit());
    }

    TEST_CASE("imgui input empty keyboard chords follow universal and existential semantics") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        const std::vector<Salix::KeyCode> none;
        CHECK(manager.multiple_are_down(none));
        CHECK(manager.multiple_are_held_down(none));
        CHECK(manager.multiple_are_held_down_for(none, 0.5f));
        CHECK(manager.multiple_were_released(none));
        CHECK(manager.multiple_are_up(none));
        CHECK_FALSE(manager.any_of_combo_was_released(none));
    }

    TEST_CASE("imgui input empty mouse chords follow universal and existential semantics") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        const std::vector<Salix::MouseButton> none;
        CHECK(manager.multiple_are_down(none));
        CHECK(manager.multiple_are_held_down(none));
        CHECK(manager.multiple_are_held_down_for(none, 0.5f));
        CHECK(manager.multiple_were_released(none));
        CHECK(manager.multiple_are_up(none));
        CHECK_FALSE(manager.any_of_combo_was_released(none));
    }
}

// Isolated virtual-dispatch and GUI-input state behavioral contracts.
TEST_SUITE("Salix::Batch747::ImGuiInputBatch747") {
    TEST_CASE("imgui test context bridge points to the owned context throughout a fixture lifetime") {
        InputManagerContext fixture;CHECK(Salix::ImGuiInputManager::get_context_for_testing()==fixture.owned);CHECK(ImGui::GetCurrentContext()==fixture.owned);
    }

    TEST_CASE("imgui scroll sign detection handles positive subunit wheel deltas") {
        InputManagerContext fixture;Salix::ImGuiInputManager m;ImGui::GetIO().MouseWheel=0.125f;CHECK(m.did_scroll(Salix::MouseScroll::Forward));CHECK_FALSE(m.did_scroll(Salix::MouseScroll::Backward));CHECK(m.get_mouse_scroll_delta()==0.125f);
    }

    TEST_CASE("imgui scroll sign detection handles negative subunit wheel deltas") {
        InputManagerContext fixture;Salix::ImGuiInputManager m;ImGui::GetIO().MouseWheel=-0.125f;CHECK_FALSE(m.did_scroll(Salix::MouseScroll::Forward));CHECK(m.did_scroll(Salix::MouseScroll::Backward));CHECK(m.get_mouse_scroll_delta()==-0.125f);
    }

    TEST_CASE("imgui changing wheel delta back to zero clears both direction flags") {
        InputManagerContext fixture;Salix::ImGuiInputManager m;ImGui::GetIO().MouseWheel=2.f;CHECK(m.did_scroll(Salix::MouseScroll::Forward));ImGui::GetIO().MouseWheel=0.f;CHECK_FALSE(m.did_scroll(Salix::MouseScroll::Forward));CHECK_FALSE(m.did_scroll(Salix::MouseScroll::Backward));
    }

    TEST_CASE("imgui update does not consume a recorded fractional scroll signal") {
        InputManagerContext fixture;Salix::ImGuiInputManager m;ImGui::GetIO().MouseWheel=0.625f;m.update(1.f/30.f);CHECK(m.get_mouse_scroll_delta()==doctest::Approx(0.625f));m.update(1.f/60.f);CHECK(m.did_scroll(Salix::MouseScroll::Forward));
    }

    TEST_CASE("imgui mouse position rounds negative subpixel offsets toward zero") {
        InputManagerContext fixture;Salix::ImGuiInputManager m;ImGui::GetIO().MousePos=ImVec2(-0.875f,-0.125f);int x=77,y=99;m.get_mouse_position(&x,&y);CHECK(x==0);CHECK(y==0);
    }

    TEST_CASE("imgui mouse coordinates query does not modify their float source position") {
        InputManagerContext fixture;Salix::ImGuiInputManager m;ImGui::GetIO().MousePos=ImVec2(42.75f,-9.5f);int x=0,y=0;m.get_mouse_position(&x,&y);CHECK(x==42);CHECK(y==-9);CHECK(ImGui::GetIO().MousePos.x==doctest::Approx(42.75f));CHECK(ImGui::GetIO().MousePos.y==doctest::Approx(-9.5f));
    }

    TEST_CASE("imgui mouse position may be queried repeatedly after changing the raw coordinates") {
        InputManagerContext fixture;Salix::ImGuiInputManager m;int x=0,y=0;ImGui::GetIO().MousePos=ImVec2(3.f,4.f);m.get_mouse_position(&x,&y);CHECK(x==3);CHECK(y==4);ImGui::GetIO().MousePos=ImVec2(-8.f,20.f);m.get_mouse_position(&x,&y);CHECK(x==-8);CHECK(y==20);
    }

    TEST_CASE("imgui manager update preserves quit state while wheel deltas change") {
        InputManagerContext fixture;Salix::ImGuiInputManager m;for(float wheel:{-1.f,0.f,1.f}){ImGui::GetIO().MouseWheel=wheel;m.update(0.125f);CHECK_FALSE(m.wants_to_quit());}
    }

    TEST_CASE("imgui raw engine event does not overwrite the GUI mouse coordinates") {
        InputManagerContext fixture;Salix::ImGuiInputManager m;ImGui::GetIO().MousePos=ImVec2(-24.5f,56.75f);Salix::MockEvent raw;m.process_event(raw);int x=0,y=0;m.get_mouse_position(&x,&y);CHECK(x==-24);CHECK(y==56);CHECK_FALSE(m.wants_to_quit());
    }
}

TEST_SUITE("Salix::input::ImGuiInputManagerBatch947") {
    TEST_CASE("unmapped None key can never satisfy held duration query") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        CHECK_FALSE(manager.is_held_down_for(Salix::KeyCode::None, 0.0f));
        CHECK_FALSE(manager.is_held_down_for(Salix::KeyCode::None, 1.0f));
    }

    TEST_CASE("unmapped Separator key can never satisfy held duration query") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        CHECK_FALSE(manager.is_held_down_for(Salix::KeyCode::Separator, 0.0f));
        CHECK_FALSE(manager.is_held_down_for(Salix::KeyCode::Separator, 5.0f));
    }

    TEST_CASE("unmapped mouse button follows safe inactive semantics") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        CHECK_FALSE(manager.is_down(Salix::MouseButton::None));
        CHECK_FALSE(manager.is_held_down(Salix::MouseButton::None));
        CHECK_FALSE(manager.is_held_down_for(Salix::MouseButton::None, 0.0f));
        CHECK_FALSE(manager.was_released(Salix::MouseButton::None));
        CHECK(manager.is_up(Salix::MouseButton::None));
    }

    TEST_CASE("mouse chord containing only unmapped button is inactive but fully up") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        const std::vector<Salix::MouseButton> buttons{Salix::MouseButton::None};
        CHECK_FALSE(manager.multiple_are_down(buttons));
        CHECK_FALSE(manager.multiple_are_held_down(buttons));
        CHECK_FALSE(manager.multiple_are_held_down_for(buttons, 0.0f));
        CHECK_FALSE(manager.multiple_were_released(buttons));
        CHECK_FALSE(manager.any_of_combo_was_released(buttons));
        CHECK(manager.multiple_are_up(buttons));
    }

    TEST_CASE("MouseScroll None never matches a positive wheel delta") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MouseWheel = 4.5f;
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::None));
        CHECK(manager.did_scroll(Salix::MouseScroll::Forward));
    }

    TEST_CASE("MouseScroll None never matches a negative wheel delta") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MouseWheel = -4.5f;
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::None));
        CHECK(manager.did_scroll(Salix::MouseScroll::Backward));
    }

    TEST_CASE("large fractional wheel values remain unchanged by direction queries") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MouseWheel = 123.75f;
        CHECK(manager.did_scroll(Salix::MouseScroll::Forward));
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Backward));
        CHECK(manager.get_mouse_scroll_delta() == doctest::Approx(123.75f));
    }

    TEST_CASE("mouse coordinate conversion handles large signed fractional positions") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MousePos = ImVec2(32767.875f, -32767.875f);
        int x = 0, y = 0;
        manager.get_mouse_position(&x, &y);
        CHECK(x == 32767);
        CHECK(y == -32767);
    }

    TEST_CASE("multiple raw engine events preserve GUI wheel and mouse position") {
        InputManagerContext fixture;
        Salix::ImGuiInputManager manager;
        ImGui::GetIO().MouseWheel = -0.75f;
        ImGui::GetIO().MousePos = ImVec2(19.5f, -41.25f);
        Salix::MockEvent first, second, third;
        manager.process_event(first);
        manager.process_event(second);
        manager.process_event(third);
        int x = 0, y = 0;
        manager.get_mouse_position(&x, &y);
        CHECK(manager.get_mouse_scroll_delta() == doctest::Approx(-0.75f));
        CHECK(x == 19);
        CHECK(y == -41);
        CHECK_FALSE(manager.wants_to_quit());
    }

    TEST_CASE("manager construction and destruction never replace the owned ImGui context") {
        InputManagerContext fixture;
        REQUIRE(ImGui::GetCurrentContext() == fixture.owned);
        {
            Salix::ImGuiInputManager manager;
            CHECK(ImGui::GetCurrentContext() == fixture.owned);
            CHECK(Salix::ImGuiInputManager::get_context_for_testing() == fixture.owned);
        }
        CHECK(ImGui::GetCurrentContext() == fixture.owned);
        CHECK(Salix::ImGuiInputManager::get_context_for_testing() == fixture.owned);
    }
}
