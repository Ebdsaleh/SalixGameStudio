#include <doctest.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Salix/gui/IFont.h>
#include <Tests/SalixEngine/mocking/gui/MockIFont.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

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

