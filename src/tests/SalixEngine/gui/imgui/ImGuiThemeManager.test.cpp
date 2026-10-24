#include <doctest.h>
#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/ITheme.h>
#include <Tests/SalixEngine/mocking/gui/MockITheme.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <filesystem>

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

    TEST_CASE("theme manager unregister removes existing theme and clears active pointer safely") {
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Disposable")));
        REQUIRE(manager.set_active_theme("Disposable"));
        REQUIRE(manager.get_active_theme() != nullptr);
        CHECK(manager.unregister_theme("Disposable"));
        CHECK(manager.get_theme("Disposable") == nullptr);
        CHECK(manager.get_active_theme() == nullptr);
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

namespace {
class Batch1947ThemeDirectory {
public:
    Batch1947ThemeDirectory() {
        static std::atomic<unsigned long> counter{0};
        path = std::filesystem::temp_directory_path() /
            ("SalixGameStudio_Batch1947_ThemeDir_" + std::to_string(++counter));
        std::filesystem::create_directories(path);
    }
    ~Batch1947ThemeDirectory() { std::error_code ec; std::filesystem::remove_all(path, ec); }
    std::filesystem::path path;
};
}

TEST_SUITE("Salix::gui::ImGuiThemeManagerBatch1947") {
    TEST_CASE("unregister missing theme is a safe false result") {
        Salix::ImGuiThemeManager manager;
        CHECK_FALSE(manager.unregister_theme("Missing"));
        CHECK(manager.get_registered_themes().empty());
    }

    TEST_CASE("purging missing theme preserves an existing active theme") {
        Salix::ImGuiThemeManager manager;
        auto theme = std::make_unique<MockITheme>("Active");
        MockITheme* raw = theme.get();
        REQUIRE(manager.register_theme(std::move(theme)));
        REQUIRE(manager.set_active_theme("Active"));
        CHECK_FALSE(manager.purge_theme("Missing"));
        CHECK(manager.get_active_theme() == raw);
    }

    TEST_CASE("save rejects empty path before looking up registered theme") {
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Theme")));
        CHECK_FALSE(manager.save_theme_to_file("Theme", ""));
    }

    TEST_CASE("save rejects missing theme name for nonempty destination") {
        Salix::ImGuiThemeManager manager;
        CHECK_FALSE(manager.save_theme_to_file("Missing", "unused.theme"));
    }

    TEST_CASE("save rejects registered non ImGui theme implementation") {
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Foreign")));
        CHECK_FALSE(manager.save_theme_to_file("Foreign", "unused.theme"));
    }

    TEST_CASE("named theme loader rejects a missing file without registry mutation") {
        Salix::ImGuiThemeManager manager;
        CHECK_FALSE(manager.load_theme_from_file("Missing", "__batch1947_missing_theme__.yaml", false));
        CHECK(manager.get_registered_themes().empty());
    }

    TEST_CASE("internal theme loader rejects a missing file without registry mutation") {
        Salix::ImGuiThemeManager manager;
        CHECK_FALSE(manager.load_internal_theme_from_file("__batch1947_missing_internal_theme__.yaml", false));
        CHECK(manager.get_registered_themes().empty());
    }

    TEST_CASE("directory loader rejects a missing directory") {
        Salix::ImGuiThemeManager manager;
        CHECK_FALSE(manager.load_themes_from_directory("__batch1947_missing_theme_directory__"));
    }

    TEST_CASE("directory loader accepts an existing empty directory") {
        Batch1947ThemeDirectory directory;
        Salix::ImGuiThemeManager manager;
        CHECK(manager.load_themes_from_directory(directory.path.string()));
        CHECK(manager.get_registered_themes().empty());
    }

    TEST_CASE("unregistering inactive theme leaves active theme pointer intact") {
        Salix::ImGuiThemeManager manager;
        auto active = std::make_unique<MockITheme>("Active");
        MockITheme* raw = active.get();
        REQUIRE(manager.register_theme(std::move(active)));
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Inactive")));
        REQUIRE(manager.set_active_theme("Active"));
        REQUIRE(manager.unregister_theme("Inactive"));
        CHECK(manager.get_active_theme() == raw);
        CHECK(manager.get_theme("Inactive") == nullptr);
    }
}

// ---- Relocated from src/tests/SalixEngine/gui/IThemeManager.test.cpp; canonical tests mirror production .cpp files ----
#include <doctest.h>
#include <Salix/gui/IThemeManager.h>
#include <Tests/SalixEngine/mocking/gui/MockIThemeManager.h>
#include <Tests/SalixEngine/mocking/gui/MockITheme.h>
#include <Tests/SalixEngine/mocking/gui/MockIGui.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::gui::IThemeManagerBatch1447") {
    TEST_CASE("theme manager interface initialization stores exact GUI pointer") {
        MockIThemeManager manager;
        MockIGui gui;
        Salix::IThemeManager* interface_ptr = &manager;
        CHECK(interface_ptr->initialize(&gui));
        CHECK(manager.gui == &gui);
        CHECK(manager.initialize_calls == 1);
    }

    TEST_CASE("theme manager controlled initialization failure remains observable") {
        MockIThemeManager manager;
        manager.initialize_result = false;
        Salix::IThemeManager* interface_ptr = &manager;
        CHECK_FALSE(interface_ptr->initialize(nullptr));
        CHECK(manager.gui == nullptr);
        CHECK(manager.initialize_calls == 1);
    }

    TEST_CASE("theme manager registration owns theme and rejects duplicate name") {
        MockIThemeManager manager;
        CHECK(manager.register_theme(std::make_unique<MockITheme>("Dark", "dark.yaml")));
        CHECK_FALSE(manager.register_theme(std::make_unique<MockITheme>("Dark", "other.yaml")));
        CHECK(manager.get_theme("Dark") != nullptr);
        CHECK(manager.get_registered_themes().size() == 1);
    }

    TEST_CASE("theme manager registered theme names are deterministic and sorted") {
        MockIThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Zulu")));
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Alpha")));
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Middle")));
        const auto names = manager.get_registered_themes();
        REQUIRE(names.size() == 3);
        CHECK(names[0] == "Alpha");
        CHECK(names[1] == "Middle");
        CHECK(names[2] == "Zulu");
    }

    TEST_CASE("theme manager active selection preserves exact registered object") {
        MockIThemeManager manager;
        auto theme = std::make_unique<MockITheme>("Active");
        MockITheme* raw = theme.get();
        REQUIRE(manager.register_theme(std::move(theme)));
        CHECK(manager.set_active_theme("Active"));
        CHECK(manager.get_active_theme() == raw);
        CHECK_FALSE(manager.set_active_theme("Missing"));
        CHECK(manager.get_active_theme() == raw);
    }

    TEST_CASE("theme manager apply dispatches registered theme to initialized GUI") {
        MockIThemeManager manager;
        MockIGui gui;
        REQUIRE(manager.initialize(&gui));
        auto theme = std::make_unique<MockITheme>("Applied");
        MockITheme* raw = theme.get();
        REQUIRE(manager.register_theme(std::move(theme)));
        CHECK(manager.apply_theme("Applied"));
        CHECK(raw->apply_count == 1);
        CHECK(raw->last_gui == &gui);
        CHECK_FALSE(manager.apply_theme("Missing"));
        CHECK(manager.apply_calls == 2);
    }

    TEST_CASE("theme manager unregistering active theme clears active pointer") {
        MockIThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Disposable")));
        REQUIRE(manager.set_active_theme("Disposable"));
        REQUIRE(manager.get_active_theme() != nullptr);
        CHECK(manager.unregister_theme("Disposable"));
        CHECK(manager.get_active_theme() == nullptr);
        CHECK(manager.get_theme("Disposable") == nullptr);
    }

    TEST_CASE("theme manager file and directory workflows transport exact arguments") {
        MockIThemeManager manager;
        CHECK(manager.load_theme_from_file("Dark", "themes/dark.yaml", true));
        CHECK(manager.last_theme_name == "Dark");
        CHECK(manager.last_theme_path == "themes/dark.yaml");
        CHECK(manager.last_apply_immediately);
        CHECK(manager.load_themes_from_directory("themes"));
        CHECK(manager.last_directory == "themes");
        CHECK(manager.save_theme_to_file("Dark", "exports/dark.yaml"));
        CHECK(manager.last_save_theme_name == "Dark");
        CHECK(manager.last_save_path == "exports/dark.yaml");
    }

    TEST_CASE("theme manager style scale transport is independent of registry state") {
        MockIThemeManager manager;
        manager.apply_style_scale(1.75f);
        CHECK(manager.style_scale_calls == 1);
        CHECK(manager.last_style_scale == doctest::Approx(1.75f));
        CHECK(manager.get_registered_themes().empty());
    }

    TEST_CASE("theme manager purge and shutdown clear ownership state") {
        MockIThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("One")));
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Two")));
        REQUIRE(manager.set_active_theme("One"));
        CHECK(manager.purge_theme("One"));
        CHECK(manager.get_active_theme() == nullptr);
        CHECK(manager.get_registered_themes().size() == 1);
        manager.shutdown();
        CHECK(manager.shutdown_calls == 1);
        CHECK(manager.get_registered_themes().empty());
        CHECK(manager.get_active_theme() == nullptr);
    }
}
