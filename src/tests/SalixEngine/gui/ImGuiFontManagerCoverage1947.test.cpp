#include <doctest.h>
#include <Salix/gui/imgui/ImGuiFontManager.h>
#include <Salix/gui/IFont.h>
#include <Tests/SalixEngine/mocking/gui/MockIFont.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::gui::ImGuiFontManagerCoverage1947") {
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
