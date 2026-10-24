#include <doctest.h>
#include <Salix/gui/imgui/ImGuiThemeManager.h>
#include <Salix/gui/ITheme.h>
#include <Tests/SalixEngine/mocking/gui/MockITheme.h>
#include <atomic>
#include <filesystem>
#include <memory>
#include <string>

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

TEST_SUITE("Salix::gui::ImGuiThemeManagerCoverage1947") {
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
