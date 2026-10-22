#include <doctest.h>
#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/ITheme.h>
#include <Tests/SalixEngine/mocking/gui/MockITheme.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

TEST_SUITE("Salix::gui::ImGuiThemeManagerBatch1247") {
    TEST_CASE("theme manager starts with empty registry and no active theme") {
        Salix::ImGuiThemeManager manager;
        CHECK(manager.get_registered_themes().empty());
        CHECK(manager.get_active_theme() == nullptr);
        CHECK(manager.get_theme("Missing") == nullptr);
    }

    TEST_CASE("theme manager rejects null GUI initialization instead of dereferencing it") {
        Salix::ImGuiThemeManager manager;
        CHECK_FALSE(manager.initialize(nullptr));
        CHECK(manager.get_active_theme() == nullptr);
    }

    TEST_CASE("theme manager rejects null theme registration without changing registry") {
        Salix::ImGuiThemeManager manager;
        std::unique_ptr<Salix::ITheme> null_theme;
        CHECK_FALSE(manager.register_theme(std::move(null_theme)));
        CHECK(manager.get_registered_themes().empty());
    }

    TEST_CASE("theme manager registers and retrieves the exact owned theme") {
        Salix::ImGuiThemeManager manager;
        auto theme = std::make_unique<MockITheme>("Night");
        MockITheme* raw = theme.get();
        REQUIRE(manager.register_theme(std::move(theme)));
        CHECK(manager.get_theme("Night") == raw);
        CHECK(manager.get_registered_themes().size() == 1);
    }

    TEST_CASE("theme manager duplicate names are rejected while original registration survives") {
        Salix::ImGuiThemeManager manager;
        auto first = std::make_unique<MockITheme>("Shared", "first.theme");
        MockITheme* first_raw = first.get();
        REQUIRE(manager.register_theme(std::move(first)));
        CHECK_FALSE(manager.register_theme(std::make_unique<MockITheme>("Shared", "second.theme")));
        CHECK(manager.get_theme("Shared") == first_raw);
        CHECK(manager.get_theme("Shared")->get_file_path() == "first.theme");
    }

    TEST_CASE("theme manager registered names contain each independent theme") {
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("A")));
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("B")));
        auto names = manager.get_registered_themes();
        std::sort(names.begin(), names.end());
        REQUIRE(names.size() == 2);
        CHECK(names[0] == "A");
        CHECK(names[1] == "B");
    }

    TEST_CASE("theme manager unregister removes existing theme and reports missing names") {
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Disposable")));
        CHECK(manager.unregister_theme("Disposable"));
        CHECK(manager.get_theme("Disposable") == nullptr);
        CHECK_FALSE(manager.unregister_theme("Disposable"));
    }

    TEST_CASE("theme manager set active accepts registered theme and exposes same pointer") {
        Salix::ImGuiThemeManager manager;
        auto theme = std::make_unique<MockITheme>("Active");
        MockITheme* raw = theme.get();
        REQUIRE(manager.register_theme(std::move(theme)));
        CHECK(manager.set_active_theme("Active"));
        CHECK(manager.get_active_theme() == raw);
    }

    TEST_CASE("theme manager failed active selection preserves previous active theme") {
        Salix::ImGuiThemeManager manager;
        auto theme = std::make_unique<MockITheme>("Retained");
        MockITheme* raw = theme.get();
        REQUIRE(manager.register_theme(std::move(theme)));
        REQUIRE(manager.set_active_theme("Retained"));
        CHECK_FALSE(manager.set_active_theme("Missing"));
        CHECK(manager.get_active_theme() == raw);
    }

    TEST_CASE("theme manager purge and shutdown never leave active pointers to destroyed themes") {
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Active")));
        REQUIRE(manager.set_active_theme("Active"));
        REQUIRE(manager.get_active_theme() != nullptr);
        CHECK(manager.purge_theme("Active"));
        CHECK(manager.get_active_theme() == nullptr);
        CHECK(manager.get_theme("Active") == nullptr);

        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Second")));
        REQUIRE(manager.set_active_theme("Second"));
        manager.shutdown();
        CHECK(manager.get_active_theme() == nullptr);
        CHECK(manager.get_registered_themes().empty());
    }
}
