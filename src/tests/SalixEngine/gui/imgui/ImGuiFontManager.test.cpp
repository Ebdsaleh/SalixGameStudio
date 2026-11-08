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

