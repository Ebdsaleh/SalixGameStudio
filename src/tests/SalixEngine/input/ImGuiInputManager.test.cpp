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