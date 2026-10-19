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
    ImGuiContext* previous = ImGui::GetCurrentContext();
    ImGuiContext* owned = ImGui::CreateContext();
    ~InputManagerContext() {
        ImGui::DestroyContext(owned);
        ImGui::SetCurrentContext(previous);
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
