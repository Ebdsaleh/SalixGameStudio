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

#include <Salix/gui/imgui/ImGuiTheme.h>
#include <fstream>

TEST_SUITE("Salix::gui::ImGuiThemeManagerBatch2447") {
    TEST_CASE("batch2447 activating a missing theme on fresh manager returns false") {
        Salix::ImGuiThemeManager manager;
        CHECK_FALSE(manager.set_active_theme("Missing2447"));
        CHECK(manager.get_active_theme() == nullptr);
    }

    TEST_CASE("batch2447 empty theme name can be registered and selected exactly") {
        Salix::ImGuiThemeManager manager;
        auto theme = std::make_unique<MockITheme>("");
        MockITheme* raw = theme.get();
        REQUIRE(manager.register_theme(std::move(theme)));
        REQUIRE(manager.set_active_theme(""));
        CHECK(manager.get_active_theme() == raw);
        CHECK(manager.get_theme("") == raw);
    }

    TEST_CASE("batch2447 unregistering inactive theme preserves active name selection") {
        Salix::ImGuiThemeManager manager;
        auto active = std::make_unique<MockITheme>("Active2447");
        MockITheme* raw = active.get();
        REQUIRE(manager.register_theme(std::move(active)));
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Inactive2447")));
        REQUIRE(manager.set_active_theme("Active2447"));
        REQUIRE(manager.unregister_theme("Inactive2447"));
        CHECK(manager.get_active_theme() == raw);
        CHECK(manager.get_theme("Inactive2447") == nullptr);
    }

    TEST_CASE("batch2447 purging active theme clears selection and removes registration") {
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Purge2447")));
        REQUIRE(manager.set_active_theme("Purge2447"));
        REQUIRE(manager.purge_theme("Purge2447"));
        CHECK(manager.get_active_theme() == nullptr);
        CHECK(manager.get_theme("Purge2447") == nullptr);
        CHECK(manager.get_registered_themes().empty());
    }

    TEST_CASE("batch2447 shutdown clears several registered themes and active selection") {
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("One2447")));
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Two2447")));
        REQUIRE(manager.set_active_theme("Two2447"));
        manager.shutdown();
        CHECK(manager.get_active_theme() == nullptr);
        CHECK(manager.get_registered_themes().empty());
        CHECK(manager.get_theme("One2447") == nullptr);
    }

    TEST_CASE("batch2447 nonpositive style scales return safely without ImGui context") {
        Salix::ImGuiThemeManager manager;
        CHECK_NOTHROW(manager.apply_style_scale(0.0f));
        CHECK_NOTHROW(manager.apply_style_scale(-2.0f));
        CHECK(manager.get_active_theme() == nullptr);
    }

    TEST_CASE("batch2447 internal loader registers valid minimal theme by YAML name") {
        Batch1947ThemeDirectory directory;
        const auto file = directory.path / "internal2447.theme";
        { std::ofstream out(file); out << "name: Internal2447\ngui_type: ImGui\n"; }
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.load_internal_theme_from_file(file.string(), false));
        REQUIRE(manager.get_theme("Internal2447") != nullptr);
        CHECK(manager.get_registered_themes().size() == 1);
    }

    TEST_CASE("batch2447 internal loader replaces an existing theme with the same loaded name") {
        Batch1947ThemeDirectory directory;
        const auto first = directory.path / "first2447.theme";
        const auto second = directory.path / "second2447.theme";
        { std::ofstream out(first); out << "name: Replace2447\ngui_type: ImGui\nFont:\n  font_size: 12\n"; }
        { std::ofstream out(second); out << "name: Replace2447\ngui_type: ImGui\nFont:\n  font_size: 18\n"; }
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.load_internal_theme_from_file(first.string(), false));
        REQUIRE(manager.load_internal_theme_from_file(second.string(), false));
        CHECK(manager.get_registered_themes().size() == 1);
        REQUIRE(manager.get_theme("Replace2447") != nullptr);
    }

    TEST_CASE("batch2447 directory loader loads theme extension and ignores unrelated files") {
        Batch1947ThemeDirectory directory;
        { std::ofstream out(directory.path / "load2447.theme"); out << "name: Directory2447\ngui_type: ImGui\n"; }
        { std::ofstream out(directory.path / "ignore2447.txt"); out << "name: Ignored2447\ngui_type: ImGui\n"; }
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.load_themes_from_directory(directory.path.string()));
        CHECK(manager.get_theme("Directory2447") != nullptr);
        CHECK(manager.get_theme("Ignored2447") == nullptr);
        CHECK(manager.get_registered_themes().size() == 1);
    }

    TEST_CASE("batch2447 registered ImGui theme saves successfully to owned temporary path") {
        Batch1947ThemeDirectory directory;
        const auto file = directory.path / "saved2447.yaml";
        Salix::ImGuiThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<Salix::ImGuiTheme>("Saved2447")));
        REQUIRE(manager.save_theme_to_file("Saved2447", file.string()));
        CHECK(std::filesystem::is_regular_file(file));
        CHECK(std::filesystem::file_size(file) > 0);
    }
}

