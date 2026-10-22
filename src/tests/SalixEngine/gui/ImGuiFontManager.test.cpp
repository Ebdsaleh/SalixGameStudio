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
