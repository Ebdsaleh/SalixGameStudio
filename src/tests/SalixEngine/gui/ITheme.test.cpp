#include <doctest.h>
#include <Salix/gui/ITheme.h>
#include <Tests/SalixEngine/mocking/gui/MockITheme.h>
#include <Tests/SalixEngine/mocking/gui/MockIGui.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::gui::IThemeBatch1447") {
    TEST_CASE("theme interface exposes configured name and file path") {
        MockITheme theme("Dark", "themes/dark.yaml");
        Salix::ITheme* interface_ptr = &theme;
        CHECK(interface_ptr->get_name() == "Dark");
        CHECK(interface_ptr->get_file_path() == "themes/dark.yaml");
    }

    TEST_CASE("theme apply dispatch records exact GUI pointer") {
        MockITheme theme;
        MockIGui gui;
        Salix::ITheme* interface_ptr = &theme;
        interface_ptr->apply(&gui);
        CHECK(theme.apply_count == 1);
        CHECK(theme.last_gui == &gui);
    }

    TEST_CASE("theme apply accepts null GUI for controlled mock testing") {
        MockITheme theme;
        Salix::ITheme* interface_ptr = &theme;
        interface_ptr->apply(nullptr);
        CHECK(theme.apply_count == 1);
        CHECK(theme.last_gui == nullptr);
    }

    TEST_CASE("theme load success records requested path and updates theme file path") {
        MockITheme theme("Theme", "old.yaml");
        theme.load_result = true;
        CHECK(theme.load_from_yaml("new.yaml"));
        CHECK(theme.load_count == 1);
        CHECK(theme.last_load_path == "new.yaml");
        CHECK(theme.get_file_path() == "new.yaml");
    }

    TEST_CASE("theme load failure records request without replacing stored path") {
        MockITheme theme("Theme", "stable.yaml");
        theme.load_result = false;
        CHECK_FALSE(theme.load_from_yaml("missing.yaml"));
        CHECK(theme.load_count == 1);
        CHECK(theme.last_load_path == "missing.yaml");
        CHECK(theme.get_file_path() == "stable.yaml");
    }

    TEST_CASE("theme save success records exact destination") {
        MockITheme theme;
        theme.save_result = true;
        const Salix::ITheme* interface_ptr = &theme;
        CHECK(interface_ptr->save_to_yaml("export/theme.yaml"));
        CHECK(theme.save_count == 1);
        CHECK(theme.last_save_path == "export/theme.yaml");
    }

    TEST_CASE("theme save failure remains observable with requested path retained") {
        MockITheme theme;
        theme.save_result = false;
        CHECK_FALSE(theme.save_to_yaml("readonly/theme.yaml"));
        CHECK(theme.save_count == 1);
        CHECK(theme.last_save_path == "readonly/theme.yaml");
    }

    TEST_CASE("theme load and save accounting remain independent") {
        MockITheme theme;
        theme.load_from_yaml("loaded.yaml");
        theme.save_to_yaml("saved.yaml");
        theme.save_to_yaml("saved2.yaml");
        CHECK(theme.load_count == 1);
        CHECK(theme.save_count == 2);
        CHECK(theme.last_load_path == "loaded.yaml");
        CHECK(theme.last_save_path == "saved2.yaml");
    }

    TEST_CASE("separate theme instances retain independent file paths and counters") {
        MockITheme first("First", "first.yaml");
        MockITheme second("Second", "second.yaml");
        first.load_from_yaml("changed.yaml");
        first.apply(nullptr);
        CHECK(second.get_file_path() == "second.yaml");
        CHECK(second.load_count == 0);
        CHECK(second.apply_count == 0);
    }

    TEST_CASE("theme interface supports polymorphic destruction") {
        std::unique_ptr<Salix::ITheme> theme =
            std::make_unique<MockITheme>("Poly", "poly.yaml");
        REQUIRE(theme != nullptr);
        CHECK(theme->get_name() == "Poly");
        CHECK(theme->save_to_yaml("out.yaml"));
    }
}
