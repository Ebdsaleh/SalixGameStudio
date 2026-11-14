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

TEST_SUITE("Salix::gui::IThemeManagerBatch6647") {
    TEST_CASE("batch6647 null theme registration is rejected without changing registry") {
        MockIThemeManager manager;
        CHECK_FALSE(manager.register_theme(nullptr));
        CHECK(manager.get_registered_themes().empty());
    }

    TEST_CASE("batch6647 missing unregister preserves all registered themes") {
        MockIThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Keep6647")));
        CHECK_FALSE(manager.unregister_theme("Missing6647"));
        REQUIRE(manager.get_registered_themes().size() == 1);
        CHECK(manager.get_theme("Keep6647") != nullptr);
    }

    TEST_CASE("batch6647 failed active selection preserves current active theme") {
        MockIThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Active6647")));
        REQUIRE(manager.set_active_theme("Active6647"));
        Salix::ITheme* active = manager.get_active_theme();
        REQUIRE(active != nullptr);
        CHECK_FALSE(manager.set_active_theme("Missing6647"));
        CHECK(manager.get_active_theme() == active);
    }

    TEST_CASE("batch6647 failed purge preserves current active theme") {
        MockIThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Active6647")));
        REQUIRE(manager.set_active_theme("Active6647"));
        Salix::ITheme* active = manager.get_active_theme();
        CHECK_FALSE(manager.purge_theme("Missing6647"));
        CHECK(manager.get_active_theme() == active);
    }

    TEST_CASE("batch6647 duplicate theme registration preserves original object") {
        MockIThemeManager manager;
        auto original = std::make_unique<MockITheme>("Dup6647", "original6647.yaml");
        MockITheme* raw = original.get();
        REQUIRE(manager.register_theme(std::move(original)));
        CHECK_FALSE(manager.register_theme(std::make_unique<MockITheme>("Dup6647", "replacement6647.yaml")));
        CHECK(manager.get_theme("Dup6647") == raw);
        CHECK(manager.get_theme("Dup6647")->get_file_path() == "original6647.yaml");
    }

    TEST_CASE("batch6647 controlled file load failure still records complete request") {
        MockIThemeManager manager;
        manager.load_file_result = false;
        CHECK_FALSE(manager.load_theme_from_file("Theme6647", "missing6647.yaml", true));
        CHECK(manager.load_file_calls == 1);
        CHECK(manager.last_theme_name == "Theme6647");
        CHECK(manager.last_theme_path == "missing6647.yaml");
        CHECK(manager.last_apply_immediately);
    }

    TEST_CASE("batch6647 controlled directory load failure records directory") {
        MockIThemeManager manager;
        manager.load_directory_result = false;
        CHECK_FALSE(manager.load_themes_from_directory("Themes/6647"));
        CHECK(manager.load_directory_calls == 1);
        CHECK(manager.last_directory == "Themes/6647");
    }

    TEST_CASE("batch6647 controlled save failure records theme and destination") {
        MockIThemeManager manager;
        manager.save_file_result = false;
        CHECK_FALSE(manager.save_theme_to_file("Theme6647", "readonly6647.yaml"));
        CHECK(manager.save_file_calls == 1);
        CHECK(manager.last_save_theme_name == "Theme6647");
        CHECK(manager.last_save_path == "readonly6647.yaml");
    }

    TEST_CASE("batch6647 repeated style scaling keeps latest value and count") {
        MockIThemeManager manager;
        manager.apply_style_scale(1.25f);
        manager.apply_style_scale(-6.647f);
        CHECK(manager.style_scale_calls == 2);
        CHECK(manager.last_style_scale == doctest::Approx(-6.647f));
    }

    TEST_CASE("batch6647 repeated shutdown safely clears registry and active state") {
        MockIThemeManager manager;
        REQUIRE(manager.register_theme(std::make_unique<MockITheme>("Disposable6647")));
        REQUIRE(manager.set_active_theme("Disposable6647"));
        manager.shutdown();
        manager.shutdown();
        CHECK(manager.shutdown_calls == 2);
        CHECK(manager.get_registered_themes().empty());
        CHECK(manager.get_active_theme() == nullptr);
    }
}
