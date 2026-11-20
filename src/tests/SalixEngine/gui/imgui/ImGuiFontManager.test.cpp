#include <doctest.h>
#include <cstdlib>
#include <SDL.h>
#include <Tests/SalixEngine/mocking/window/MockIWindow.h>
#include <Salix/events/EventManager.h>
#include <Salix/gui/imgui/opengl/OpenGLImGui.h>
#include <Salix/gui/imgui/sdl/SDLImGui.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Tests/SalixEngine/mocking/gui/MockIGui.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <filesystem>
#include <Salix/gui/IFont.h>
#include <Tests/SalixEngine/mocking/gui/MockIFont.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch1247") {
    TEST_CASE("font manager starts with an empty registry and no active font") {
        Salix::ImGuiFontManager manager;
        CHECK(manager.get_registered_font_names().empty());
        CHECK(manager.get_font_registry().empty());
        CHECK(manager.get_active_font() == nullptr);
    }

    TEST_CASE("font manager rejects null font registration without mutating registry") {
        Salix::ImGuiFontManager manager;
        std::unique_ptr<Salix::IFont> null_font;
        CHECK_FALSE(manager.register_font(std::move(null_font)));
        CHECK(manager.get_font_registry().empty());
    }

    TEST_CASE("font manager registers a font and returns the same owned object") {
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("Body_16px", "Body", 16.0f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        CHECK(manager.get_font("Body_16px") == raw);
        CHECK(manager.get_font_registry().size() == 1);
    }

    TEST_CASE("font manager rejects duplicate names and preserves first registration") {
        Salix::ImGuiFontManager manager;
        auto first = std::make_unique<MockIFont>("SharedName", "FirstFamily", 12.0f);
        MockIFont* first_raw = first.get();
        REQUIRE(manager.register_font(std::move(first)));
        CHECK_FALSE(manager.register_font(std::make_unique<MockIFont>("SharedName", "SecondFamily", 24.0f)));
        CHECK(manager.get_font("SharedName") == first_raw);
        CHECK(manager.get_font("SharedName")->get_family() == "FirstFamily");
    }

    TEST_CASE("font manager registered name list contains each unique font exactly once") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("A_12px", "A", 12.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("B_14px", "B", 14.0f)));
        auto names = manager.get_registered_font_names();
        std::sort(names.begin(), names.end());
        REQUIRE(names.size() == 2);
        CHECK(names[0] == "A_12px");
        CHECK(names[1] == "B_14px");
    }

    TEST_CASE("font manager unique families are deduplicated and returned in sorted order") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Mono_12px", "Mono", 12.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Mono_18px", "Mono", 18.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Sans_16px", "Sans", 16.0f)));
        const auto families = manager.get_unique_font_families();
        REQUIRE(families.size() == 2);
        CHECK(families[0] == "Mono");
        CHECK(families[1] == "Sans");
    }

    TEST_CASE("font manager family sizes are filtered by family and sorted numerically") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Body_24px", "Body", 24.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Body_12px", "Body", 12.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Code_18px", "Code", 18.0f)));
        const auto sizes = manager.get_font_sizes_for_font_family("Body");
        REQUIRE(sizes.size() == 2);
        CHECK(sizes[0] == doctest::Approx(12.0f));
        CHECK(sizes[1] == doctest::Approx(24.0f));
    }

    TEST_CASE("font manager set active rejects missing names and accepts registered names") {
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.set_active_font(""));
        CHECK_FALSE(manager.set_active_font("Missing"));
        auto font = std::make_unique<MockIFont>("Active", "Body", 16.0f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        CHECK(manager.set_active_font("Active"));
        CHECK(manager.get_active_font() == raw);
    }

    TEST_CASE("font manager unregistering the active font clears active pointer safely") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Active", "Body", 16.0f)));
        REQUIRE(manager.set_active_font("Active"));
        REQUIRE(manager.get_active_font() != nullptr);
        CHECK(manager.unregister_font("Active"));
        CHECK(manager.get_active_font() == nullptr);
        CHECK(manager.get_font("Active") == nullptr);
        CHECK_FALSE(manager.unregister_font("Active"));
    }

    TEST_CASE("font manager avoids ImGui context access for missing or unloaded font application and shutdown clears state") {
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.apply_font("Missing"));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("NoAtlasFont", "Body", 16.0f)));
        CHECK_FALSE(manager.apply_font("NoAtlasFont"));
        REQUIRE(manager.set_active_font("NoAtlasFont"));
        manager.shutdown();
        CHECK(manager.get_active_font() == nullptr);
        CHECK(manager.get_registered_font_names().empty());
    }
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch1947") {
    TEST_CASE("font atlas rebuild fails safely before GUI initialization") {
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.rebuild_font_atlas_texture());
    }

    TEST_CASE("stubbed directory loader rejects empty directory path") {
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.load_fonts_from_directory(""));
    }

    TEST_CASE("stubbed directory loader also reports false for a nonempty path") {
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.load_fonts_from_directory("Assets/Fonts"));
    }

    TEST_CASE("stubbed purge rejects empty font names") {
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.purge_font(""));
    }

    TEST_CASE("stubbed purge reports false even for a registered font name") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Registered", "Family", 12.0f)));
        CHECK_FALSE(manager.purge_font("Registered"));
        CHECK(manager.get_font("Registered") != nullptr);
    }

    TEST_CASE("missing font lookup is null without mutating registry") {
        Salix::ImGuiFontManager manager;
        CHECK(manager.get_font("Missing") == nullptr);
        CHECK(manager.get_font_registry().empty());
    }

    TEST_CASE("family-size lookup returns empty vector when family has no matches") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Body", "BodyFamily", 16.0f)));
        CHECK(manager.get_font_sizes_for_font_family("OtherFamily").empty());
    }

    TEST_CASE("empty family name is still deduplicated as a registry family") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("A", "", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("B", "", 12.0f)));
        const auto families = manager.get_unique_font_families();
        REQUIRE(families.size() == 1);
        CHECK(families[0].empty());
    }

    TEST_CASE("empty font name is a valid explicit registry key") {
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("", "Family", 14.0f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        CHECK(manager.get_font("") == raw);
        CHECK_FALSE(manager.set_active_font(""));
    }

    TEST_CASE("shutdown is idempotent after registry and active state have been populated") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Active", "Body", 16.0f)));
        REQUIRE(manager.set_active_font("Active"));
        manager.shutdown();
        CHECK(manager.get_active_font() == nullptr);
        CHECK(manager.get_font_registry().empty());
        CHECK_NOTHROW(manager.shutdown());
        CHECK(manager.get_font_registry().empty());
    }
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch2447") {
    TEST_CASE("batch2447 null initialization fails without mutating registry state") {
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.initialize(nullptr));
        CHECK(manager.get_font_registry().empty());
        CHECK(manager.get_active_font() == nullptr);
    }

    TEST_CASE("batch2447 unregistering a missing font leaves existing registry untouched") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Keep2447", "Family", 12.0f)));
        CHECK_FALSE(manager.unregister_font("Missing2447"));
        CHECK(manager.get_font("Keep2447") != nullptr);
        CHECK(manager.get_font_registry().size() == 1);
    }

    TEST_CASE("batch2447 unregistering inactive font preserves active font selection") {
        Salix::ImGuiFontManager manager;
        auto active = std::make_unique<MockIFont>("Active2447", "Family", 14.0f);
        MockIFont* active_raw = active.get();
        REQUIRE(manager.register_font(std::move(active)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Inactive2447", "Family", 18.0f)));
        REQUIRE(manager.set_active_font("Active2447"));
        REQUIRE(manager.unregister_font("Inactive2447"));
        CHECK(manager.get_active_font() == active_raw);
        CHECK(manager.get_font("Inactive2447") == nullptr);
    }

    TEST_CASE("batch2447 registered name list reflects successful removal exactly") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("A2447", "Family", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("B2447", "Family", 12.0f)));
        REQUIRE(manager.unregister_font("A2447"));
        const auto names = manager.get_registered_font_names();
        REQUIRE(names.size() == 1);
        CHECK(names[0] == "B2447");
    }

    TEST_CASE("batch2447 family size query sorts unsorted registration sizes ascending") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("F24", "Family2447", 24.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("F8", "Family2447", 8.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("F16", "Family2447", 16.0f)));
        const auto sizes = manager.get_font_sizes_for_font_family("Family2447");
        REQUIRE(sizes.size() == 3);
        CHECK(sizes[0] == doctest::Approx(8.0f));
        CHECK(sizes[1] == doctest::Approx(16.0f));
        CHECK(sizes[2] == doctest::Approx(24.0f));
    }

    TEST_CASE("batch2447 unique family query is lexicographically sorted") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("ZuluFont", "Zulu", 12.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("AlphaFont", "Alpha", 12.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("MiddleFont", "Middle", 12.0f)));
        const auto families = manager.get_unique_font_families();
        REQUIRE(families.size() == 3);
        CHECK(families[0] == "Alpha");
        CHECK(families[1] == "Middle");
        CHECK(families[2] == "Zulu");
    }

    TEST_CASE("batch2447 applying a missing font does not clear existing active selection") {
        Salix::ImGuiFontManager manager;
        auto active = std::make_unique<MockIFont>("Active2447Apply", "Body", 16.0f);
        MockIFont* raw = active.get();
        REQUIRE(manager.register_font(std::move(active)));
        REQUIRE(manager.set_active_font("Active2447Apply"));
        CHECK_FALSE(manager.apply_font("Missing2447Apply"));
        CHECK(manager.get_active_font() == raw);
    }

    TEST_CASE("batch2447 font load rejects size below minimum before requiring ImGui context") {
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.load_font_from_file("missing2447.ttf", "TooSmall2447", 5.99f, false));
        CHECK(manager.get_font_registry().empty());
    }

    TEST_CASE("batch2447 font load rejects empty path before requiring ImGui context") {
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.load_font_from_file("", "NoPath2447", 12.0f, false));
        CHECK(manager.get_font_registry().empty());
    }

    TEST_CASE("batch2447 missing font batch source returns without registry mutation") {
        Salix::ImGuiFontManager manager;
        manager.create_font_batch("__batch2447_missing_font__.ttf", "MissingFamily2447", 8.0f, 12.0f);
        CHECK(manager.get_font_registry().empty());
        CHECK(manager.get_active_font() == nullptr);
    }
}
TEST_SUITE("Salix::gui::ImGuiFontManagerBatch3447") {
    TEST_CASE("batch3447 active selection can switch between two registered font identities") {
        Salix::ImGuiFontManager manager;
        auto first = std::make_unique<MockIFont>("First3447", "FamilyA", 12.f);
        auto second = std::make_unique<MockIFont>("Second3447", "FamilyB", 18.f);
        MockIFont* first_raw = first.get();
        MockIFont* second_raw = second.get();
        REQUIRE(manager.register_font(std::move(first)));
        REQUIRE(manager.register_font(std::move(second)));
        REQUIRE(manager.set_active_font("First3447"));
        CHECK(manager.get_active_font() == first_raw);
        REQUIRE(manager.set_active_font("Second3447"));
        CHECK(manager.get_active_font() == second_raw);
    }

    TEST_CASE("batch3447 empty active selection request preserves an existing active font") {
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("Keep3447", "Family", 14.f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        REQUIRE(manager.set_active_font("Keep3447"));
        CHECK_FALSE(manager.set_active_font(""));
        CHECK(manager.get_active_font() == raw);
    }

    TEST_CASE("batch3447 case distinct font names coexist as separate registry keys") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Case3447", "Family", 10.f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("case3447", "Family", 11.f)));
        CHECK(manager.get_font("Case3447") != nullptr);
        CHECK(manager.get_font("case3447") != nullptr);
        CHECK(manager.get_font_registry().size() == 2);
    }

    TEST_CASE("batch3447 embedded null font name remains an exact registry key") {
        Salix::ImGuiFontManager manager;
        const std::string name("Font\0Alt", 8);
        REQUIRE(manager.register_font(std::make_unique<MockIFont>(name, "BinaryFamily", 13.f)));
        REQUIRE(manager.get_font(name) != nullptr);
        CHECK(manager.get_font(name)->get_name() == name);
        CHECK(manager.get_font_registry().count(name) == 1);
    }

    TEST_CASE("batch3447 family size query retains duplicate sizes from distinct registered fonts") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("DupSizeA3447", "DupFamily3447", 16.f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("DupSizeB3447", "DupFamily3447", 16.f)));
        const auto sizes = manager.get_font_sizes_for_font_family("DupFamily3447");
        REQUIRE(sizes.size() == 2);
        CHECK(sizes[0] == doctest::Approx(16.f));
        CHECK(sizes[1] == doctest::Approx(16.f));
    }

    TEST_CASE("batch3447 family size filtering remains case sensitive") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Upper3447", "Mono3447", 12.f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Lower3447", "mono3447", 24.f)));
        const auto upper = manager.get_font_sizes_for_font_family("Mono3447");
        const auto lower = manager.get_font_sizes_for_font_family("mono3447");
        REQUIRE(upper.size() == 1);
        REQUIRE(lower.size() == 1);
        CHECK(upper[0] == doctest::Approx(12.f));
        CHECK(lower[0] == doctest::Approx(24.f));
    }

    TEST_CASE("batch3447 registry reference object remains stable across insertion and removal") {
        Salix::ImGuiFontManager manager;
        const auto* registry = &manager.get_font_registry();
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("StableA3447", "A", 10.f)));
        CHECK(&manager.get_font_registry() == registry);
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("StableB3447", "B", 20.f)));
        CHECK(&manager.get_font_registry() == registry);
        REQUIRE(manager.unregister_font("StableA3447"));
        CHECK(&manager.get_font_registry() == registry);
    }

    TEST_CASE("batch3447 applying registered font with null runtime pointer preserves current active selection") {
        Salix::ImGuiFontManager manager;
        auto active = std::make_unique<MockIFont>("Active3447", "A", 12.f);
        MockIFont* active_raw = active.get();
        REQUIRE(manager.register_font(std::move(active)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("InvalidRuntime3447", "B", 14.f)));
        REQUIRE(manager.set_active_font("Active3447"));
        CHECK_FALSE(manager.apply_font("InvalidRuntime3447"));
        CHECK(manager.get_active_font() == active_raw);
    }

    TEST_CASE("batch3447 active font name can be unregistered then re registered with new identity") {
        Salix::ImGuiFontManager manager;
        auto first = std::make_unique<MockIFont>("Recycle3447", "Old", 12.f);
        REQUIRE(manager.register_font(std::move(first)));
        REQUIRE(manager.set_active_font("Recycle3447"));
        REQUIRE(manager.unregister_font("Recycle3447"));
        CHECK(manager.get_active_font() == nullptr);
        auto second = std::make_unique<MockIFont>("Recycle3447", "New", 22.f);
        MockIFont* second_raw = second.get();
        REQUIRE(manager.register_font(std::move(second)));
        REQUIRE(manager.set_active_font("Recycle3447"));
        CHECK(manager.get_active_font() == second_raw);
        CHECK(manager.get_active_font()->get_family() == "New");
    }

    TEST_CASE("batch3447 shutdown permits clean registry reuse with a new font") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Before3447", "Before", 10.f)));
        REQUIRE(manager.set_active_font("Before3447"));
        manager.shutdown();
        CHECK(manager.get_active_font() == nullptr);
        CHECK(manager.get_font_registry().empty());
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("After3447", "After", 20.f)));
        CHECK(manager.get_font("After3447") != nullptr);
        CHECK(manager.get_font("Before3447") == nullptr);
    }
}
TEST_SUITE("Salix::gui::ImGuiFontManagerBatch3847") {
    TEST_CASE("batch3847 registered font name vector is independent from registry state") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("One3847", "Family", 10.f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Two3847", "Family", 12.f)));
        auto names = manager.get_registered_font_names();
        REQUIRE(names.size() == 2);
        names.clear();
        CHECK(manager.get_registered_font_names().size() == 2);
        CHECK(manager.get_font_registry().size() == 2);
    }

    TEST_CASE("batch3847 embedded null family remains exact during family size lookup") {
        Salix::ImGuiFontManager manager;
        const std::string family("Family\0Alt3847", 14);
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("NullFamily3847", family, 17.f)));
        const auto exact = manager.get_font_sizes_for_font_family(family);
        REQUIRE(exact.size() == 1);
        CHECK(exact[0] == doctest::Approx(17.f));
        CHECK(manager.get_font_sizes_for_font_family("Family").empty());
    }

    TEST_CASE("batch3847 family sizes sort negative zero and positive diagnostic values") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Neg3847", "Diag3847", -2.f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Zero3847", "Diag3847", 0.f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Pos3847", "Diag3847", 4.f)));
        const auto sizes = manager.get_font_sizes_for_font_family("Diag3847");
        REQUIRE(sizes.size() == 3);
        CHECK(sizes[0] == doctest::Approx(-2.f));
        CHECK(sizes[1] == doctest::Approx(0.f));
        CHECK(sizes[2] == doctest::Approx(4.f));
    }

    TEST_CASE("batch3847 duplicate active registration preserves original active object") {
        Salix::ImGuiFontManager manager;
        auto original = std::make_unique<MockIFont>("Active3847", "FirstFamily3847", 12.f);
        MockIFont* raw = original.get();
        REQUIRE(manager.register_font(std::move(original)));
        REQUIRE(manager.set_active_font("Active3847"));
        CHECK_FALSE(manager.register_font(std::make_unique<MockIFont>("Active3847", "SecondFamily3847", 24.f)));
        CHECK(manager.get_active_font() == raw);
        CHECK(manager.get_font("Active3847")->get_family() == "FirstFamily3847");
    }

    TEST_CASE("batch3847 unregister then reregister same name selects replacement metadata") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Replace3847", "Before3847", 11.f)));
        REQUIRE(manager.set_active_font("Replace3847"));
        REQUIRE(manager.unregister_font("Replace3847"));
        CHECK(manager.get_active_font() == nullptr);
        auto replacement = std::make_unique<MockIFont>("Replace3847", "After3847", 29.f);
        MockIFont* raw = replacement.get();
        REQUIRE(manager.register_font(std::move(replacement)));
        REQUIRE(manager.set_active_font("Replace3847"));
        CHECK(manager.get_active_font() == raw);
        CHECK(manager.get_font("Replace3847")->get_family() == "After3847");
    }

    TEST_CASE("batch3847 purge stub preserves registered active font for nonempty name") {
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("Purge3847", "Family3847", 16.f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        REQUIRE(manager.set_active_font("Purge3847"));
        CHECK_FALSE(manager.purge_font("Purge3847"));
        CHECK(manager.get_active_font() == raw);
        CHECK(manager.get_font("Purge3847") == raw);
    }

    TEST_CASE("batch3847 empty directory loader stub fails without registry mutation") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Keep3847", "Family", 14.f)));
        CHECK_FALSE(manager.load_fonts_from_directory(""));
        CHECK(manager.get_font_registry().size() == 1);
        CHECK(manager.get_font("Keep3847") != nullptr);
    }

    TEST_CASE("batch3847 nonempty directory loader stub also leaves registry unchanged") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("KeepNonempty3847", "Family", 15.f)));
        CHECK_FALSE(manager.load_fonts_from_directory("unused_batch3847_fonts"));
        CHECK(manager.get_font_registry().size() == 1);
        CHECK(manager.get_font("KeepNonempty3847") != nullptr);
    }

    TEST_CASE("batch3847 repeated active selection of same font is idempotent") {
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("Same3847", "Family", 18.f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        REQUIRE(manager.set_active_font("Same3847"));
        REQUIRE(manager.set_active_font("Same3847"));
        CHECK(manager.get_active_font() == raw);
    }

    TEST_CASE("batch3847 missing font lookup does not mutate names families or active state") {
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("Stable3847", "StableFamily3847", 20.f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        REQUIRE(manager.set_active_font("Stable3847"));
        const auto names_before = manager.get_registered_font_names();
        const auto families_before = manager.get_unique_font_families();
        CHECK(manager.get_font("Missing3847") == nullptr);
        CHECK(manager.get_registered_font_names().size() == names_before.size());
        CHECK(manager.get_unique_font_families() == families_before);
        CHECK(manager.get_active_font() == raw);
    }
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch4447") {
    TEST_CASE("batch4447 three registered fonts remain independently retrievable") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("A4447", "FamilyA", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("B4447", "FamilyB", 12.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("C4447", "FamilyC", 14.0f)));
        CHECK(manager.get_font("A4447") != nullptr);
        CHECK(manager.get_font("B4447") != nullptr);
        CHECK(manager.get_font("C4447") != nullptr);
        CHECK(manager.get_font_registry().size() == 3);
    }

    TEST_CASE("batch4447 unregistering inactive font preserves current active font") {
        Salix::ImGuiFontManager manager;
        auto active = std::make_unique<MockIFont>("Active4447", "A", 12.0f);
        MockIFont* active_raw = active.get();
        REQUIRE(manager.register_font(std::move(active)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Inactive4447", "B", 14.0f)));
        REQUIRE(manager.set_active_font("Active4447"));
        REQUIRE(manager.unregister_font("Inactive4447"));
        CHECK(manager.get_active_font() == active_raw);
    }

    TEST_CASE("batch4447 null registration attempt preserves established active font") {
        Salix::ImGuiFontManager manager;
        auto active = std::make_unique<MockIFont>("Keep4447", "A", 12.0f);
        MockIFont* raw = active.get();
        REQUIRE(manager.register_font(std::move(active)));
        REQUIRE(manager.set_active_font("Keep4447"));
        std::unique_ptr<Salix::IFont> null_font;
        CHECK_FALSE(manager.register_font(std::move(null_font)));
        CHECK(manager.get_active_font() == raw);
    }

    TEST_CASE("batch4447 duplicate registration never grows registry cardinality") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Duplicate4447", "First", 12.0f)));
        REQUIRE(manager.get_font_registry().size() == 1);
        CHECK_FALSE(manager.register_font(std::make_unique<MockIFont>("Duplicate4447", "Second", 24.0f)));
        CHECK(manager.get_font_registry().size() == 1);
        CHECK(manager.get_font("Duplicate4447")->get_family() == "First");
    }

    TEST_CASE("batch4447 case distinct font names can be selected independently") {
        Salix::ImGuiFontManager manager;
        auto upper = std::make_unique<MockIFont>("Case4447", "Upper", 12.0f);
        auto lower = std::make_unique<MockIFont>("case4447", "Lower", 14.0f);
        MockIFont* upper_raw = upper.get();
        MockIFont* lower_raw = lower.get();
        REQUIRE(manager.register_font(std::move(upper)));
        REQUIRE(manager.register_font(std::move(lower)));
        REQUIRE(manager.set_active_font("Case4447"));
        CHECK(manager.get_active_font() == upper_raw);
        REQUIRE(manager.set_active_font("case4447"));
        CHECK(manager.get_active_font() == lower_raw);
    }

    TEST_CASE("batch4447 nonempty whitespace rich font name is a valid exact registry key") {
        Salix::ImGuiFontManager manager;
        const std::string name = "  Font 4447  ";
        auto font = std::make_unique<MockIFont>(name, "Family", 16.0f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        REQUIRE(manager.set_active_font(name));
        CHECK(manager.get_active_font() == raw);
        CHECK(manager.get_font(name) == raw);
    }

    TEST_CASE("batch4447 family collection preserves case and whitespace distinct values") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("One4447", "Mono4447", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Two4447", " mono4447 ", 12.0f)));
        const auto families = manager.get_unique_font_families();
        CHECK(std::find(families.begin(), families.end(), "Mono4447") != families.end());
        CHECK(std::find(families.begin(), families.end(), " mono4447 ") != families.end());
        CHECK(families.size() == 2);
    }

    TEST_CASE("batch4447 fractional family sizes are returned in numeric order") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("F1", "Fraction4447", 12.75f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("F2", "Fraction4447", 12.25f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("F3", "Fraction4447", 12.5f)));
        const auto sizes = manager.get_font_sizes_for_font_family("Fraction4447");
        REQUIRE(sizes.size() == 3);
        CHECK(sizes[0] == doctest::Approx(12.25f));
        CHECK(sizes[1] == doctest::Approx(12.5f));
        CHECK(sizes[2] == doctest::Approx(12.75f));
    }

    TEST_CASE("batch4447 shutdown permits same name to be registered and selected again") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Reuse4447", "Before", 10.0f)));
        REQUIRE(manager.set_active_font("Reuse4447"));
        manager.shutdown();
        auto replacement = std::make_unique<MockIFont>("Reuse4447", "After", 20.0f);
        MockIFont* raw = replacement.get();
        REQUIRE(manager.register_font(std::move(replacement)));
        REQUIRE(manager.set_active_font("Reuse4447"));
        CHECK(manager.get_active_font() == raw);
        CHECK(manager.get_font("Reuse4447")->get_family() == "After");
    }

    TEST_CASE("batch4447 purge stub leaves active registered font intact") {
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("Purge4447", "Family", 16.0f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        REQUIRE(manager.set_active_font("Purge4447"));
        CHECK_FALSE(manager.purge_font("Purge4447"));
        CHECK(manager.get_active_font() == raw);
        CHECK(manager.get_font("Purge4447") == raw);
    }
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch4947") {
    TEST_CASE("batch4947 registry reference remains stable across registration and unregister") {
        Salix::ImGuiFontManager manager;
        const auto* registry = &manager.get_font_registry();
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("StableRegistry4947", "Family", 12.0f)));
        CHECK(&manager.get_font_registry() == registry);
        REQUIRE(manager.unregister_font("StableRegistry4947"));
        CHECK(&manager.get_font_registry() == registry);
    }

    TEST_CASE("batch4947 empty font name is a valid exact registry key but cannot become active") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("", "EmptyNameFamily4947", 12.0f)));
        CHECK(manager.get_font("") != nullptr);
        CHECK_FALSE(manager.set_active_font(""));
        CHECK(manager.get_active_font() == nullptr);
    }

    TEST_CASE("batch4947 empty family is retained in unique family collection") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("EmptyFamily4947", "", 12.0f)));
        const auto families = manager.get_unique_font_families();
        REQUIRE(families.size() == 1);
        CHECK(families[0].empty());
    }

    TEST_CASE("batch4947 equal numeric sizes from distinct fonts remain duplicated in family size result") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("SameSizeA4947", "SameSizeFamily4947", 14.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("SameSizeB4947", "SameSizeFamily4947", 14.0f)));
        const auto sizes = manager.get_font_sizes_for_font_family("SameSizeFamily4947");
        REQUIRE(sizes.size() == 2);
        CHECK(sizes[0] == doctest::Approx(14.0f));
        CHECK(sizes[1] == doctest::Approx(14.0f));
    }

    TEST_CASE("batch4947 embedded null font name remains retrievable only by exact full string") {
        Salix::ImGuiFontManager manager;
        const std::string name("Font\0Name4947", 13);
        REQUIRE(manager.register_font(std::make_unique<MockIFont>(name, "Family4947", 15.0f)));
        CHECK(manager.get_font(name) != nullptr);
        CHECK(manager.get_font("Font") == nullptr);
    }

    TEST_CASE("batch4947 failed missing active selection preserves established active font") {
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("Active4947", "Family", 16.0f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        REQUIRE(manager.set_active_font("Active4947"));
        CHECK_FALSE(manager.set_active_font("Missing4947"));
        CHECK(manager.get_active_font() == raw);
    }

    TEST_CASE("batch4947 unregistering active font clears selection while retaining inactive peer") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Active4947B", "A", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Peer4947", "B", 20.0f)));
        REQUIRE(manager.set_active_font("Active4947B"));
        REQUIRE(manager.unregister_font("Active4947B"));
        CHECK(manager.get_active_font() == nullptr);
        CHECK(manager.get_font("Peer4947") != nullptr);
        CHECK(manager.get_font_registry().size() == 1);
    }

    TEST_CASE("batch4947 failed apply of registered font with null runtime pointer preserves active peer") {
        Salix::ImGuiFontManager manager;
        auto active = std::make_unique<MockIFont>("KeepActive4947", "A", 10.0f);
        MockIFont* active_raw = active.get();
        REQUIRE(manager.register_font(std::move(active)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("NullRuntime4947", "B", 12.0f)));
        REQUIRE(manager.set_active_font("KeepActive4947"));
        CHECK_FALSE(manager.apply_font("NullRuntime4947"));
        CHECK(manager.get_active_font() == active_raw);
    }

    TEST_CASE("batch4947 rebuild before GUI initialization fails without mutating registry") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Keep4947", "Family", 12.0f)));
        CHECK_FALSE(manager.rebuild_font_atlas_texture());
        CHECK(manager.get_font("Keep4947") != nullptr);
        CHECK(manager.get_font_registry().size() == 1);
    }

    TEST_CASE("batch4947 returned name vector is a value snapshot independent from registry") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("SnapshotA4947", "A", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("SnapshotB4947", "B", 20.0f)));
        auto names = manager.get_registered_font_names();
        REQUIRE(names.size() == 2);
        names.clear();
        CHECK(manager.get_registered_font_names().size() == 2);
        CHECK(manager.get_font_registry().size() == 2);
    }
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch5647") {
    TEST_CASE("batch5647 fresh manager starts with empty snapshots and null active font") {
        Salix::ImGuiFontManager manager;
        CHECK(manager.get_font_registry().empty());
        CHECK(manager.get_registered_font_names().empty());
        CHECK(manager.get_unique_font_families().empty());
        CHECK(manager.get_active_font() == nullptr);
    }

    TEST_CASE("batch5647 null registration leaves registry completely empty") {
        Salix::ImGuiFontManager manager;
        std::unique_ptr<Salix::IFont> empty;
        CHECK_FALSE(manager.register_font(std::move(empty)));
        CHECK(manager.get_font_registry().empty());
    }

    TEST_CASE("batch5647 case distinct font names coexist") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Case5647", "A", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("case5647", "B", 20.0f)));
        CHECK(manager.get_font("Case5647") != nullptr);
        CHECK(manager.get_font("case5647") != nullptr);
        CHECK(manager.get_font_registry().size() == 2);
    }

    TEST_CASE("batch5647 duplicate registration preserves first object and metadata") {
        Salix::ImGuiFontManager manager;
        auto first = std::make_unique<MockIFont>("Shared5647", "First5647", 11.0f);
        MockIFont* raw = first.get();
        REQUIRE(manager.register_font(std::move(first)));
        CHECK_FALSE(manager.register_font(std::make_unique<MockIFont>("Shared5647", "Second5647", 22.0f)));
        CHECK(manager.get_font("Shared5647") == raw);
        CHECK(manager.get_font("Shared5647")->get_family() == "First5647");
    }

    TEST_CASE("batch5647 returned font name vector is independent from registry") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("A5647", "FamilyA", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("B5647", "FamilyB", 20.0f)));
        auto names = manager.get_registered_font_names();
        REQUIRE(names.size() == 2);
        names.clear();
        CHECK(manager.get_font_registry().size() == 2);
        CHECK(manager.get_registered_font_names().size() == 2);
    }

    TEST_CASE("batch5647 unique families remain sorted and deduplicated") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Z1_5647", "Zulu5647", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("A1_5647", "Alpha5647", 12.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("A2_5647", "Alpha5647", 14.0f)));
        const auto families = manager.get_unique_font_families();
        REQUIRE(families.size() == 2);
        CHECK(families[0] == "Alpha5647");
        CHECK(families[1] == "Zulu5647");
    }

    TEST_CASE("batch5647 family sizes sort numerically while retaining duplicates") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("S1_5647", "Sizes5647", 20.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("S2_5647", "Sizes5647", 10.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("S3_5647", "Sizes5647", 20.0f)));
        const auto sizes = manager.get_font_sizes_for_font_family("Sizes5647");
        REQUIRE(sizes.size() == 3);
        CHECK(sizes[0] == doctest::Approx(10.0f));
        CHECK(sizes[1] == doctest::Approx(20.0f));
        CHECK(sizes[2] == doctest::Approx(20.0f));
    }

    TEST_CASE("batch5647 failed active selection preserves established active font") {
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("Active5647", "Family", 16.0f);
        MockIFont* raw = font.get();
        REQUIRE(manager.register_font(std::move(font)));
        REQUIRE(manager.set_active_font("Active5647"));
        CHECK_FALSE(manager.set_active_font("Missing5647"));
        CHECK(manager.get_active_font() == raw);
    }

    TEST_CASE("batch5647 unregistering inactive peer preserves active selection") {
        Salix::ImGuiFontManager manager;
        auto active = std::make_unique<MockIFont>("ActivePeer5647", "A", 12.0f);
        MockIFont* active_raw = active.get();
        REQUIRE(manager.register_font(std::move(active)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("RemovePeer5647", "B", 14.0f)));
        REQUIRE(manager.set_active_font("ActivePeer5647"));
        REQUIRE(manager.unregister_font("RemovePeer5647"));
        CHECK(manager.get_active_font() == active_raw);
        CHECK(manager.get_font("ActivePeer5647") == active_raw);
    }

    TEST_CASE("batch5647 repeated shutdown clears populated registry and remains idempotent") {
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Shutdown5647", "Family", 12.0f)));
        REQUIRE(manager.set_active_font("Shutdown5647"));
        manager.shutdown();
        CHECK(manager.get_active_font() == nullptr);
        CHECK(manager.get_font_registry().empty());
        CHECK_NOTHROW(manager.shutdown());
        CHECK(manager.get_font_registry().empty());
    }
}

#include <Salix/input/ImGuiInputManager.h>
#include <Salix/core/ApplicationConfig.h>
#include <Tests/SalixEngine/mocking/gui/MockIGui.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <imgui.h>
#include <fstream>
#include <filesystem>
#include <atomic>

namespace {
struct Batch6947FontManagerContext {
    ImGuiContext* previous_test = ImGui::GetCurrentContext();
    ImGuiContext* previous_engine = Salix::ImGuiInputManager::get_context_for_testing();
    ImGuiContext* owned = ImGui::CreateContext();
    Batch6947FontManagerContext() {
        ImGui::SetCurrentContext(owned);
        Salix::ImGuiInputManager::set_context_for_testing(owned);
    }
    ~Batch6947FontManagerContext() {
        Salix::ImGuiInputManager::set_context_for_testing(previous_engine);
        ImGui::DestroyContext(owned);
        ImGui::SetCurrentContext(previous_test);
    }
};
std::filesystem::path batch6947_bad_font_file() {
    static std::atomic<unsigned long> counter{0};
    const auto path = std::filesystem::temp_directory_path() /
        ("Salix_Batch6947_BadFont_" + std::to_string(++counter) + ".ttf");
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << "not-a-valid-font-6947";
    return path;
}
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch6947") {
    TEST_CASE("batch6947 SDL initialization rejects missing renderer interface") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.initialize(&gui));
    }

    TEST_CASE("batch6947 SDL initialization rejects null native renderer handle") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        MockIRenderer renderer;
        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, &renderer, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.initialize(&gui));
    }

    TEST_CASE("batch6947 SDL initialization accepts opaque nonnull native handle without using backend") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        MockIRenderer renderer;
        renderer.native_handle = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x6947));
        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, &renderer, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        CHECK(manager.initialize(&gui));
    }

    TEST_CASE("batch6947 OpenGL initialization rejects missing renderer interface") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::OpenGL;
        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.initialize(&gui));
    }

    TEST_CASE("batch6947 OpenGL initialization rejects non OpenGL renderer implementation") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::OpenGL;
        MockIRenderer renderer;
        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, &renderer, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.initialize(&gui));
    }

    TEST_CASE("batch6947 unrecognized renderer type reaches initialization success tail") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;
        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        CHECK(manager.initialize(&gui));
    }

    TEST_CASE("batch6947 rebuild after SDL setup fails safely on non SDLImGui GUI object") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::SDL;
        MockIRenderer renderer;
        renderer.native_handle = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x6947));
        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, &renderer, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));
        CHECK_FALSE(manager.rebuild_font_atlas_texture());
    }

    TEST_CASE("batch6947 rebuild with unrecognized renderer type returns false") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;
        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));
        CHECK_FALSE(manager.rebuild_font_atlas_texture());
    }

    TEST_CASE("batch6947 apply font succeeds with real ImGui font pointer in bare context") {
        Batch6947FontManagerContext context;
        Salix::ImGuiFontManager manager;
        auto font = std::make_unique<MockIFont>("ContextFont6947", "ContextFamily6947", 16.0f);
        ImFont* imgui_font = ImGui::GetIO().Fonts->AddFontDefault();
        REQUIRE(imgui_font != nullptr);
        font->set_imgui_font_ptr(imgui_font);
        REQUIRE(manager.register_font(std::move(font)));
        REQUIRE(manager.apply_font("ContextFont6947"));
        CHECK(manager.get_active_font() != nullptr);
        CHECK(ImGui::GetIO().FontDefault == imgui_font);
    }

    TEST_CASE("batch6947 valid repository TTF loads and registers without native backend") {
        Batch6947FontManagerContext context;
        Salix::ImGuiFontManager manager;
        auto repo_root = std::filesystem::path(__FILE__);
        for (int i = 0; i < 6; ++i) repo_root = repo_root.parent_path();
        const auto font_path = repo_root / "Assets" / "Fonts" / "Roboto-Regular.ttf";
        REQUIRE(std::filesystem::is_regular_file(font_path));
        REQUIRE(manager.load_font_from_file(font_path.string(), "LoadedFont6947", 16.0f, false));
        REQUIRE(manager.get_font("LoadedFont6947") != nullptr);
        CHECK(manager.get_font("LoadedFont6947")->get_imgui_font_ptr() != nullptr);
        CHECK(manager.get_active_font() == nullptr);
    }
}

namespace {
std::filesystem::path batch7047_repo_root() {
    auto path = std::filesystem::path(__FILE__);
    for (int i = 0; i < 6; ++i) path = path.parent_path();
    return path;
}
struct Batch7047CwdGuard {
    std::filesystem::path old = std::filesystem::current_path();
    explicit Batch7047CwdGuard(const std::filesystem::path& next) { std::filesystem::current_path(next); }
    ~Batch7047CwdGuard() { std::error_code ec; std::filesystem::current_path(old, ec); }
};
std::filesystem::path batch7047_font_path() {
    return batch7047_repo_root() / "Assets" / "Fonts" / "Roboto-Regular.ttf";
}
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch7047") {
    TEST_CASE("batch7047 single size font batch registers one real font") {
        Batch6947FontManagerContext context;
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None;
        MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.create_font_batch(batch7047_font_path().string(),"Batch7047Single",8.0f,8.0f);
        CHECK(manager.get_registered_font_names().size()==1);
        CHECK(manager.get_font("Batch7047Single_8px")!=nullptr);
    }

    TEST_CASE("batch7047 three size font batch registers inclusive two point steps") {
        Batch6947FontManagerContext context;
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.create_font_batch(batch7047_font_path().string(),"Batch7047Three",8.0f,12.0f);
        CHECK(manager.get_registered_font_names().size()==3);
        CHECK(manager.get_font("Batch7047Three_10px")!=nullptr);
        CHECK(manager.get_font("Batch7047Three_12px")!=nullptr);
    }

    TEST_CASE("batch7047 reversed font batch range creates no variants") {
        Batch6947FontManagerContext context;
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.create_font_batch(batch7047_font_path().string(),"Batch7047Reverse",12.0f,8.0f);
        CHECK(manager.get_registered_font_names().empty());
    }

    TEST_CASE("batch7047 DPI scaling is reflected in registered font data size") {
        Batch6947FontManagerContext context;
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; config.gui_settings.global_dpi_scaling=2.0f;
        MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.create_font_batch(batch7047_font_path().string(),"Batch7047Dpi",8.0f,8.0f);
        const auto sizes=manager.get_font_sizes_for_font_family("Batch7047Dpi");
        REQUIRE(sizes.size()==1);
        CHECK(sizes[0]==doctest::Approx(16.0f));
    }

    TEST_CASE("batch7047 odd size font batch preserves requested variant names") {
        Batch6947FontManagerContext context;
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.create_font_batch(batch7047_font_path().string(),"Batch7047Odd",9.0f,13.0f);
        CHECK(manager.get_font("Batch7047Odd_9px")!=nullptr);
        CHECK(manager.get_font("Batch7047Odd_11px")!=nullptr);
        CHECK(manager.get_font("Batch7047Odd_13px")!=nullptr);
    }

    TEST_CASE("batch7047 setup default fonts registers every canonical family") {
        Batch6947FontManagerContext context; Batch7047CwdGuard cwd(batch7047_repo_root());
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.setup_default_fonts();
        const auto families=manager.get_unique_font_families();
        CHECK(std::find(families.begin(),families.end(),"Roboto-Regular")!=families.end());
        CHECK(std::find(families.begin(),families.end(),"Roboto-Medium")!=families.end());
        CHECK(std::find(families.begin(),families.end(),"Karla-Regular")!=families.end());
        CHECK(std::find(families.begin(),families.end(),"ProggyClean")!=families.end());
        CHECK(std::find(families.begin(),families.end(),"ProggyTiny")!=families.end());
        CHECK(std::find(families.begin(),families.end(),"DroidSans")!=families.end());
        CHECK(std::find(families.begin(),families.end(),"Cousine-Regular")!=families.end());
    }

    TEST_CASE("batch7047 setup default fonts creates full Roboto Regular size ladder") {
        Batch6947FontManagerContext context; Batch7047CwdGuard cwd(batch7047_repo_root());
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.setup_default_fonts();
        const auto sizes=manager.get_font_sizes_for_font_family("Roboto-Regular");
        CHECK(sizes.size()==33);
        CHECK(sizes.front()==doctest::Approx(8.0f));
        CHECK(sizes.back()==doctest::Approx(72.0f));
    }

    TEST_CASE("batch7047 valid font load with apply immediately sets active font") {
        Batch6947FontManagerContext context;
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Apply7047",16.0f,true));
        REQUIRE(manager.get_active_font()!=nullptr);
        CHECK(manager.get_active_font()->get_name()=="Apply7047");
    }

    TEST_CASE("batch7047 duplicate valid font name reaches registration failure path") {
        Batch6947FontManagerContext context;
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Duplicate7047",16.0f,false));
        CHECK_FALSE(manager.load_font_from_file(batch7047_font_path().string(),"Duplicate7047",18.0f,false));
        CHECK(manager.get_font_registry().size()==1);
    }

    TEST_CASE("batch7047 valid font load without apply leaves active font null") {
        Batch6947FontManagerContext context;
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Passive7047",16.0f,false));
        CHECK(manager.get_active_font()==nullptr);
    }

    TEST_CASE("batch7047 same file can be registered under two independent names") {
        Batch6947FontManagerContext context;
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Alias7047A",16.0f,false));
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Alias7047B",18.0f,false));
        CHECK(manager.get_font_registry().size()==2);
    }

    TEST_CASE("batch7047 active loaded font purge stub reports false and preserves registration") {
        Batch6947FontManagerContext context;
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Purge7047",16.0f,true));
        REQUIRE(manager.get_active_font()!=nullptr);
        auto* active_before=manager.get_active_font();
        CHECK_FALSE(manager.purge_font("Purge7047"));
        CHECK(manager.get_active_font()==active_before);
        CHECK(manager.get_font("Purge7047")==active_before);
    }

    TEST_CASE("batch7047 loaded font can be unregistered without ever becoming active") {
        Batch6947FontManagerContext context;
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Unregister7047",16.0f,false));
        REQUIRE(manager.unregister_font("Unregister7047"));
        CHECK(manager.get_font_registry().empty());
    }

    TEST_CASE("batch7047 batched real font can be applied by generated variant name") {
        Batch6947FontManagerContext context;
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.create_font_batch(batch7047_font_path().string(),"ApplyBatch7047",8.0f,8.0f);
        REQUIRE(manager.apply_font("ApplyBatch7047_8px"));
        REQUIRE(manager.get_active_font()!=nullptr);
        CHECK(manager.get_active_font()->get_family()=="ApplyBatch7047");
    }

    TEST_CASE("batch7047 batch registration exposes generated family through unique family query") {
        Batch6947FontManagerContext context;
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.create_font_batch(batch7047_font_path().string(),"Unique7047",8.0f,10.0f);
        const auto families=manager.get_unique_font_families();
        CHECK(families.size()==1);
        CHECK(families[0]=="Unique7047");
    }
}


TEST_SUITE("Salix::gui::ImGuiFontManagerBatch7374") {
    TEST_CASE("batch7374 OpenGL initialization rejects non OpenGL GUI after renderer cast succeeds") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::OpenGL;

        Salix::OpenGLRenderer renderer;
        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, &renderer, nullptr, nullptr, nullptr);

        Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.initialize(&gui));
        CHECK(manager.get_font_registry().empty());
        CHECK(manager.get_active_font() == nullptr);
    }

    TEST_CASE("batch7374 OpenGL atlas rebuild rejects non OpenGL GUI without backend access") {
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::None;

        MockIGui gui;
        gui.set_app_config(&config);
        gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);

        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));

        config.renderer_type = Salix::RendererType::OpenGL;
        CHECK_FALSE(manager.rebuild_font_atlas_texture());
        CHECK(manager.get_font_registry().empty());
        CHECK(manager.get_active_font() == nullptr);
    }
}

namespace {
class Batch7578OpenGLGui final : public Salix::OpenGLImGui {
public:
    Batch7578OpenGLGui(Salix::ApplicationConfig* config, Salix::IRenderer* renderer)
        : config_(config), renderer_(renderer) { set_app_config(config); }
    Salix::IRenderer* get_renderer() override { return renderer_; }
    Salix::ApplicationConfig* get_app_config() override { return config_; }
private:
    Salix::ApplicationConfig* config_;
    Salix::IRenderer* renderer_;
};

class Batch7578SDLGui final : public Salix::SDLImGui {
public:
    Batch7578SDLGui(Salix::ApplicationConfig* config, Salix::IRenderer* renderer)
        : config_(config), renderer_(renderer) { set_app_config(config); }
    Salix::IRenderer* get_renderer() override { return renderer_; }
    Salix::ApplicationConfig* get_app_config() override { return config_; }
private:
    Salix::ApplicationConfig* config_;
    Salix::IRenderer* renderer_;
};
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch7578") {
    TEST_CASE("batch7578 OpenGL concrete GUI and renderer reach null context continuation") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::OpenGL;
        Salix::OpenGLRenderer renderer;
        Batch7578OpenGLGui gui(&config, &renderer);
        Salix::ImGuiFontManager manager;
        CHECK(manager.initialize(&gui));
        CHECK(manager.get_font_registry().empty());
    }
    TEST_CASE("batch7578 OpenGL concrete GUI rebuild reports success after backend early return") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::OpenGL;
        Salix::OpenGLRenderer renderer;
        Batch7578OpenGLGui gui(&config, &renderer);
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));
        CHECK(manager.rebuild_font_atlas_texture());
    }
    TEST_CASE("batch7578 OpenGL manager shutdown clears GUI reference after successful initialize") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::OpenGL;
        Salix::OpenGLRenderer renderer;
        Batch7578OpenGLGui gui(&config, &renderer);
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));
        manager.shutdown();
        CHECK_FALSE(manager.rebuild_font_atlas_texture());
    }
    TEST_CASE("batch7578 SDL concrete GUI accepts nonnull native renderer handle") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::SDL;
        MockIRenderer renderer; renderer.native_handle = reinterpret_cast<void*>(static_cast<uintptr_t>(0x7578));
        Batch7578SDLGui gui(&config, &renderer);
        Salix::ImGuiFontManager manager;
        CHECK(manager.initialize(&gui));
    }
    TEST_CASE("batch7578 SDL concrete GUI rebuild reports success after backend early return") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::SDL;
        MockIRenderer renderer; renderer.native_handle = reinterpret_cast<void*>(static_cast<uintptr_t>(0x7578));
        Batch7578SDLGui gui(&config, &renderer);
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));
        CHECK(manager.rebuild_font_atlas_texture());
    }
    TEST_CASE("batch7578 SDL manager shutdown disables later rebuild") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::SDL;
        MockIRenderer renderer; renderer.native_handle = reinterpret_cast<void*>(static_cast<uintptr_t>(0x7578));
        Batch7578SDLGui gui(&config, &renderer);
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));
        manager.shutdown();
        CHECK_FALSE(manager.rebuild_font_atlas_texture());
    }
    TEST_CASE("batch7578 renderer None initializes without backend handles") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::None;
        MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        CHECK(manager.initialize(&gui));
    }
    TEST_CASE("batch7578 renderer None rebuild remains rejected as unrecognized backend") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::None;
        MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));
        CHECK_FALSE(manager.rebuild_font_atlas_texture());
    }
    TEST_CASE("batch7578 successful backend initialize still permits normal font registration") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::None;
        MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Batch7578Font", "Batch7578", 12.0f)));
        CHECK(manager.get_font("Batch7578Font") != nullptr);
    }
    TEST_CASE("batch7578 shutdown after registered font empties registry and active pointer") {
        Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::None;
        MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr, nullptr, nullptr, nullptr, nullptr);
        Salix::ImGuiFontManager manager;
        REQUIRE(manager.initialize(&gui));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Batch7578Shutdown", "Batch7578", 14.0f)));
        manager.shutdown();
        CHECK(manager.get_font_registry().empty());
        CHECK(manager.get_active_font() == nullptr);
    }
}

namespace {
struct Batch7678FontSDLBackend {
    SDL_Window* raw_window = nullptr;
    SDL_Renderer* raw_renderer = nullptr;
    MockIWindow window;
    MockIRenderer renderer;
    Salix::ApplicationConfig config;
    Salix::SDLImGui gui;
    Salix::ImGuiFontManager manager;
    bool ready = false;

    Batch7678FontSDLBackend() {
        SDL_Quit();
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
        SDL_setenv("SDL_RENDER_DRIVER", "software", 1);
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return;
        raw_window = SDL_CreateWindow("Batch7678 Font", 0, 0, 64, 48, SDL_WINDOW_HIDDEN);
        if (!raw_window) return;
        raw_renderer = SDL_CreateRenderer(raw_window, -1, SDL_RENDERER_SOFTWARE);
        if (!raw_renderer) return;
        window.native_handle = raw_window;
        renderer.native_handle = raw_renderer;
        config.renderer_type = Salix::RendererType::SDL;
        gui.set_app_config(&config);
        if (!gui.initialize(&window, &renderer, nullptr, &manager, nullptr)) return;
        ready = manager.initialize(&gui);
    }

    ~Batch7678FontSDLBackend() {
        manager.shutdown();
        if (ready) gui.shutdown();
        if (raw_renderer) SDL_DestroyRenderer(raw_renderer);
        if (raw_window) SDL_DestroyWindow(raw_window);
        SDL_Quit();
        _putenv_s("SDL_VIDEODRIVER", "");
        _putenv_s("SDL_RENDER_DRIVER", "");
    }
};
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch7678") {
    TEST_CASE("batch7678 real SDL GUI initializes font manager with native renderer") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        CHECK(fixture.manager.get_font_registry().empty());
    }
    TEST_CASE("batch7678 real SDL GUI rebuilds font atlas backend") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        CHECK(fixture.manager.rebuild_font_atlas_texture());
    }
    TEST_CASE("batch7678 real SDL GUI supports repeated atlas rebuild") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        REQUIRE(fixture.manager.rebuild_font_atlas_texture());
        CHECK(fixture.manager.rebuild_font_atlas_texture());
    }
    TEST_CASE("batch7678 real SDL font manager still rejects missing registered font apply") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        CHECK_FALSE(fixture.manager.apply_font("Missing7678"));
    }
    TEST_CASE("batch7678 real SDL font manager registers mock font normally") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        REQUIRE(fixture.manager.register_font(std::make_unique<MockIFont>("RealSDL7678", "RealSDL", 16.0f)));
        CHECK(fixture.manager.get_font("RealSDL7678") != nullptr);
    }
    TEST_CASE("batch7678 real SDL font manager unregisters registered font") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        REQUIRE(fixture.manager.register_font(std::make_unique<MockIFont>("Remove7678", "RealSDL", 16.0f)));
        CHECK(fixture.manager.unregister_font("Remove7678"));
        CHECK(fixture.manager.get_font("Remove7678") == nullptr);
    }
    TEST_CASE("batch7678 real SDL font manager reports unique family") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        REQUIRE(fixture.manager.register_font(std::make_unique<MockIFont>("Family7678", "Family7678", 18.0f)));
        const auto families = fixture.manager.get_unique_font_families();
        CHECK(std::find(families.begin(), families.end(), "Family7678") != families.end());
    }
    TEST_CASE("batch7678 real SDL font manager reports family size") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        REQUIRE(fixture.manager.register_font(std::make_unique<MockIFont>("Size7678", "Family7678", 20.0f)));
        const auto sizes = fixture.manager.get_font_sizes_for_font_family("Family7678");
        REQUIRE(sizes.size()==1);
        CHECK(sizes[0]==doctest::Approx(20.0f));
    }
    TEST_CASE("batch7678 real SDL font manager missing batch path remains safe") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        CHECK_NOTHROW(fixture.manager.create_font_batch("__batch7678_missing_font__.ttf", "Missing7678", 8.0f, 12.0f));
        CHECK(fixture.manager.get_font_registry().empty());
    }
    TEST_CASE("batch7678 real SDL font manager shutdown disables future rebuild") {
        Batch7678FontSDLBackend fixture;
        REQUIRE(fixture.ready);
        fixture.manager.shutdown();
        CHECK_FALSE(fixture.manager.rebuild_font_atlas_texture());
    }
}

namespace {
struct Batch7778FontGLFixture {
    std::filesystem::path previous=std::filesystem::current_path();bool had=false;std::string old;bool sdl_ready=false;
    Salix::OpenGLRenderer renderer;Salix::OpenGLImGui gui;Salix::ImGuiFontManager manager;Salix::ApplicationConfig config;
    bool renderer_ready=false,gui_ready=false,manager_ready=false;
    Batch7778FontGLFixture(){
        auto p=std::filesystem::absolute(std::filesystem::path(__FILE__)).parent_path();while(p.has_parent_path()&&!std::filesystem::exists(p/"Assets"/"Shaders"))p=p.parent_path();
        if(std::filesystem::exists(p/"Assets"/"Shaders"))std::filesystem::current_path(p);
        if(const char* v=SDL_getenv("SDL_VIDEODRIVER")){had=true;old=v;}SDL_Quit();SDL_setenv("SDL_VIDEODRIVER","windows",1);sdl_ready=SDL_Init(SDL_INIT_VIDEO)==0;if(!sdl_ready)return;
        Salix::WindowConfig wc;wc.title="Batch7778 Font GL";wc.width=96;wc.height=64;renderer_ready=renderer.initialize(wc);if(!renderer_ready)return;
        config.renderer_type=Salix::RendererType::OpenGL;gui.set_app_config(&config);gui_ready=gui.initialize(renderer.get_window(),&renderer,nullptr,&manager,nullptr);if(!gui_ready)return;
        manager_ready=manager.initialize(&gui);
    }
    ~Batch7778FontGLFixture(){manager.shutdown();if(gui_ready)gui.shutdown();renderer.shutdown();SDL_Quit();if(had)SDL_setenv("SDL_VIDEODRIVER",old.c_str(),1);else _putenv_s("SDL_VIDEODRIVER","");std::error_code ec;std::filesystem::current_path(previous,ec);}
};
bool batch7778_mutated_rebuild(Salix::RendererType type){
    Salix::ApplicationConfig config;config.renderer_type=Salix::RendererType::None;MockIGui gui;gui.set_app_config(&config);gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
    Salix::ImGuiFontManager manager; if(!manager.initialize(&gui))return true;config.renderer_type=type;return manager.rebuild_font_atlas_texture();
}
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch7778") {
    TEST_CASE("batch7778 real OpenGL font manager initializes") { Batch7778FontGLFixture f; REQUIRE(f.renderer_ready);REQUIRE(f.gui_ready);CHECK(f.manager_ready); }
    TEST_CASE("batch7778 real OpenGL font manager rebuilds atlas backend") { Batch7778FontGLFixture f; REQUIRE(f.manager_ready); CHECK(f.manager.rebuild_font_atlas_texture()); }
    TEST_CASE("batch7778 bundled ProggyClean font loads and registers in live OpenGL context") { Batch7778FontGLFixture f; REQUIRE(f.manager_ready); REQUIRE(f.manager.load_font_from_file("Assets/Fonts/ProggyClean.ttf","Proggy7778",12.0f,false)); CHECK(f.manager.get_font("Proggy7778")!=nullptr); }
    TEST_CASE("batch7778 bundled DroidSans font loads at independent size") { Batch7778FontGLFixture f; REQUIRE(f.manager_ready); REQUIRE(f.manager.load_font_from_file("Assets/Fonts/DroidSans.ttf","Droid7778",18.0f,false)); const auto* font=f.manager.get_font("Droid7778"); REQUIRE(font!=nullptr); REQUIRE(font->get_data()!=nullptr); CHECK(font->get_data()->get_font_size()==doctest::Approx(18.0f)); }
    TEST_CASE("batch7778 bundled Karla font loads and applies immediately") { Batch7778FontGLFixture f; REQUIRE(f.manager_ready); REQUIRE(f.manager.load_font_from_file("Assets/Fonts/Karla-Regular.ttf","Karla7778",24.0f,true)); REQUIRE(f.manager.get_active_font()!=nullptr); CHECK(f.manager.get_active_font()->get_name()=="Karla7778"); }
    TEST_CASE("batch7778 mutated SDL type on generic GUI reaches cast rejection") { CHECK_FALSE(batch7778_mutated_rebuild(Salix::RendererType::SDL)); }
    TEST_CASE("batch7778 mutated OpenGL type on generic GUI reaches cast rejection") { CHECK_FALSE(batch7778_mutated_rebuild(Salix::RendererType::OpenGL)); }
    TEST_CASE("batch7778 mutated unsupported type reaches rebuild fallback") { CHECK_FALSE(batch7778_mutated_rebuild(static_cast<Salix::RendererType>(7778))); }
    TEST_CASE("batch7778 too small font remains rejected before ImGui load") { Batch7778FontGLFixture f; REQUIRE(f.manager_ready); CHECK_FALSE(f.manager.load_font_from_file("__missing_small7778__.ttf","Small7778",5.0f,false)); }
    TEST_CASE("batch7778 empty font path remains rejected with live OpenGL context") { Batch7778FontGLFixture f; REQUIRE(f.manager_ready); CHECK_FALSE(f.manager.load_font_from_file("","Empty7778",12.0f,false)); }
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch7978") {
    TEST_CASE("batch7978 public file loader accepts repository font at minimum practical size") {
        Batch6947FontManagerContext context; Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"LoadRegister7978A",8.0f,false));
        const auto* font=manager.get_font("LoadRegister7978A"); REQUIRE(font!=nullptr); REQUIRE(font->get_data()!=nullptr);
        CHECK(font->get_data()->get_font_size()==doctest::Approx(8.0f));
    }
    TEST_CASE("batch7978 duplicate public file loader name preserves first registered size") {
        Batch6947FontManagerContext context; Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"LoadRegister7978B",10.0f,false));
        CHECK_FALSE(manager.load_font_from_file(batch7047_font_path().string(),"LoadRegister7978B",12.0f,false));
        const auto* font=manager.get_font("LoadRegister7978B"); REQUIRE(font!=nullptr); REQUIRE(font->get_data()!=nullptr);
        CHECK(font->get_data()->get_font_size()==doctest::Approx(10.0f));
    }
    TEST_CASE("batch7978 file loader accepts exact six pixel lower boundary") {
        Batch6947FontManagerContext context; Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Boundary7978",6.0f,false));
        CHECK(manager.get_font("Boundary7978")!=nullptr);
    }
    TEST_CASE("batch7978 file loader rejects value below six pixel lower boundary") {
        Batch6947FontManagerContext context; Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.load_font_from_file(batch7047_font_path().string(),"TooSmall7978",5.99f,false));
        CHECK(manager.get_font("TooSmall7978")==nullptr);
    }
    TEST_CASE("batch7978 file loader rejects empty path before touching atlas") {
        Batch6947FontManagerContext context; Salix::ImGuiFontManager manager;
        CHECK_FALSE(manager.load_font_from_file("","EmptyPath7978",16.0f,false));
        CHECK(manager.get_font_registry().empty());
    }
    TEST_CASE("batch7978 loaded repository font can be immediately applied") {
        Batch6947FontManagerContext context; Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Apply7978",14.0f,true));
        REQUIRE(manager.get_active_font()!=nullptr); CHECK(manager.get_active_font()->get_name()=="Apply7978");
    }
    TEST_CASE("batch7978 loaded passive font can be unregistered cleanly") {
        Batch6947FontManagerContext context; Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Remove7978",18.0f,false));
        REQUIRE(manager.unregister_font("Remove7978")); CHECK(manager.get_font("Remove7978")==nullptr);
    }
    TEST_CASE("batch7978 renderer none initialize keeps atlas rebuild explicitly unsupported") {
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; MockIGui gui;
        gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui)); CHECK_FALSE(manager.rebuild_font_atlas_texture());
    }
    TEST_CASE("batch7978 public font batch exposes sorted generated family sizes") {
        Batch6947FontManagerContext context;
        Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None;
        MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
        Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
        manager.create_font_batch(batch7047_font_path().string(),"Sizes7978",10.0f,12.0f);
        const auto sizes=manager.get_font_sizes_for_font_family("Sizes7978"); REQUIRE(sizes.size()==2);
        CHECK(sizes[0]==doctest::Approx(10.0f)); CHECK(sizes[1]==doctest::Approx(12.0f));
    }
    TEST_CASE("batch7978 shutdown clears a real loaded active font") {
        Batch6947FontManagerContext context; Salix::ImGuiFontManager manager;
        REQUIRE(manager.load_font_from_file(batch7047_font_path().string(),"Shutdown7978",16.0f,true));
        manager.shutdown(); CHECK(manager.get_active_font()==nullptr); CHECK(manager.get_font_registry().empty());
    }
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch8078") {
    TEST_CASE("batch8078 public loader rejects five point nine nine again without registry mutation") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; CHECK_FALSE(m.load_font_from_file(batch7047_font_path().string(),"Small8078A",5.99f,false)); CHECK(m.get_font_registry().empty()); }
    TEST_CASE("batch8078 public loader accepts six point zero one") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; REQUIRE(m.load_font_from_file(batch7047_font_path().string(),"Six8078",6.01f,false)); CHECK(m.get_font("Six8078")!=nullptr); }
    TEST_CASE("batch8078 loaded font can be selected active without immediate apply") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; REQUIRE(m.load_font_from_file(batch7047_font_path().string(),"Active8078",12.0f,false)); REQUIRE(m.set_active_font("Active8078")); REQUIRE(m.get_active_font()); CHECK(m.get_active_font()->get_name()=="Active8078"); }
    TEST_CASE("batch8078 missing active font selection is rejected") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; CHECK_FALSE(m.set_active_font("Missing8078")); CHECK(m.get_active_font()==nullptr); }
    TEST_CASE("batch8078 missing font unregister is rejected") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; CHECK_FALSE(m.unregister_font("Missing8078")); }
    TEST_CASE("batch8078 missing font purge is rejected") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; CHECK_FALSE(m.purge_font("Missing8078")); }
    TEST_CASE("batch8078 one loaded family appears exactly once") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; REQUIRE(m.load_font_from_file(batch7047_font_path().string(),"Family8078",14.0f,false)); auto names=m.get_registered_font_names(); REQUIRE(names.size()==1); CHECK(names[0]=="Family8078"); }
    TEST_CASE("batch8078 two distinct loaded names remain independently retrievable") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; REQUIRE(m.load_font_from_file(batch7047_font_path().string(),"A8078",10.0f,false)); REQUIRE(m.load_font_from_file(batch7047_font_path().string(),"B8078",18.0f,false)); CHECK(m.get_font("A8078")!=nullptr); CHECK(m.get_font("B8078")!=nullptr); }
    TEST_CASE("batch8078 shutdown after passive font clears registry") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; REQUIRE(m.load_font_from_file(batch7047_font_path().string(),"Shutdown8078",16.0f,false)); m.shutdown(); CHECK(m.get_font_registry().empty()); }
    TEST_CASE("batch8078 renderer none atlas rebuild stays unsupported after repeated call") { Salix::ApplicationConfig cfg; cfg.renderer_type=Salix::RendererType::None; MockIGui gui; gui.set_app_config(&cfg); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr); Salix::ImGuiFontManager m; REQUIRE(m.initialize(&gui)); CHECK_FALSE(m.rebuild_font_atlas_texture()); CHECK_FALSE(m.rebuild_font_atlas_texture()); }
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch8178") {
    TEST_CASE("batch8178 null GUI initialization remains rejected") { Salix::ImGuiFontManager m; CHECK_FALSE(m.initialize(nullptr)); }
    TEST_CASE("batch8178 renderer none initialization remains accepted") { Salix::ApplicationConfig c;c.renderer_type=Salix::RendererType::None;MockIGui g;g.set_app_config(&c);g.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);Salix::ImGuiFontManager m;CHECK(m.initialize(&g)); }
    TEST_CASE("batch8178 missing apply remains rejected") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; CHECK_FALSE(m.apply_font("Missing8178")); }
    TEST_CASE("batch8178 missing active selection remains rejected") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; CHECK_FALSE(m.set_active_font("Missing8178")); }
    TEST_CASE("batch8178 missing unregister remains rejected") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; CHECK_FALSE(m.unregister_font("Missing8178")); }
    TEST_CASE("batch8178 missing purge remains rejected") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; CHECK_FALSE(m.purge_font("Missing8178")); }
    TEST_CASE("batch8178 six pixel repository font loads") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; REQUIRE(m.load_font_from_file(batch7047_font_path().string(),"Font8178A",6.0f,false)); CHECK(m.get_font("Font8178A")!=nullptr); }
    TEST_CASE("batch8178 passive loaded font appears in names") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; REQUIRE(m.load_font_from_file(batch7047_font_path().string(),"Font8178B",12.0f,false)); auto n=m.get_registered_font_names(); CHECK(std::find(n.begin(),n.end(),"Font8178B")!=n.end()); }
    TEST_CASE("batch8178 loaded active font clears on shutdown") { Batch6947FontManagerContext c; Salix::ImGuiFontManager m; REQUIRE(m.load_font_from_file(batch7047_font_path().string(),"Font8178C",14.0f,true)); m.shutdown(); CHECK(m.get_active_font()==nullptr); }
    TEST_CASE("batch8178 empty registry exposes no families") { Salix::ImGuiFontManager m; CHECK(m.get_unique_font_families().empty()); }
}

namespace {
void batch8278_expect_missing_public_font_load(const std::string& name, float size) {
    Batch6947FontManagerContext context;
    Salix::ImGuiFontManager manager;
    CHECK_FALSE(manager.load_font_from_file(
        (std::filesystem::temp_directory_path()/("__missing_batch8278_"+name+".ttf")).string(),
        name,size,false));
    CHECK(manager.get_font(name)==nullptr);
}
void batch8278_expect_directory_batch_failure(const std::string& family, float size) {
    Batch6947FontManagerContext context;
    Salix::ApplicationConfig config; config.renderer_type=Salix::RendererType::None; config.gui_settings.global_dpi_scaling=1.0f;
    MockIGui gui; gui.set_app_config(&config); gui.initialize(nullptr,nullptr,nullptr,nullptr,nullptr);
    Salix::ImGuiFontManager manager; REQUIRE(manager.initialize(&gui));
    const auto dir=std::filesystem::temp_directory_path()/("Salix_Batch8278_FontDir_"+family);
    std::error_code ec; std::filesystem::create_directories(dir,ec); REQUIRE_FALSE(ec);
    manager.create_font_batch(dir.string(),family,size,size);
    CHECK(manager.get_font_registry().empty());
    std::filesystem::remove_all(dir,ec);
}
}

TEST_SUITE("Salix::gui::ImGuiFontManagerBatch8278") {
    TEST_CASE("batch8278 public font loader rejects missing file at six pixels") { batch8278_expect_missing_public_font_load("Missing8278A",6.0f); }
    TEST_CASE("batch8278 public font loader rejects missing file at twelve pixels") { batch8278_expect_missing_public_font_load("Missing8278B",12.0f); }
    TEST_CASE("batch8278 public font loader rejects missing file at seventy two pixels") { batch8278_expect_missing_public_font_load("Missing8278C",72.0f); }
    TEST_CASE("batch8278 public font loader missing file never registers requested name") { batch8278_expect_missing_public_font_load("Missing8278D",18.0f); }
    TEST_CASE("batch8278 public font loader missing file remains repeatable") { batch8278_expect_missing_public_font_load("Missing8278E",24.0f); }
    TEST_CASE("batch8278 font batch directory path reaches private loader failure at eight pixels") { batch8278_expect_directory_batch_failure("Dir8278A",8.0f); }
    TEST_CASE("batch8278 font batch directory path reaches private loader failure at ten pixels") { batch8278_expect_directory_batch_failure("Dir8278B",10.0f); }
    TEST_CASE("batch8278 font batch directory path reaches private loader failure at fourteen pixels") { batch8278_expect_directory_batch_failure("Dir8278C",14.0f); }
    TEST_CASE("batch8278 font batch directory path reaches private loader failure at twenty pixels") { batch8278_expect_directory_batch_failure("Dir8278D",20.0f); }
    TEST_CASE("batch8278 font batch directory failure leaves no active font") { batch8278_expect_directory_batch_failure("Dir8278E",16.0f); }
}

