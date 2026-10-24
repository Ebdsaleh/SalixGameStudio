#include <doctest.h>
#include <Salix/gui/IFontManager.h>
#include <Tests/SalixEngine/mocking/gui/MockIFontManager.h>
#include <Tests/SalixEngine/mocking/gui/MockIFont.h>
#include <Tests/SalixEngine/mocking/gui/MockIGui.h>
#include <memory>
#include <vector>

TEST_SUITE("Salix::gui::IFontManagerBatch1447") {
    TEST_CASE("font manager interface initialization stores exact GUI pointer") {
        MockIFontManager manager;
        MockIGui gui;
        Salix::IFontManager* interface_ptr = &manager;
        CHECK(interface_ptr->initialize(&gui));
        CHECK(manager.gui == &gui);
        CHECK(manager.initialize_calls == 1);
    }

    TEST_CASE("font manager interface can report controlled initialization failure") {
        MockIFontManager manager;
        manager.initialize_result = false;
        Salix::IFontManager* interface_ptr = &manager;
        CHECK_FALSE(interface_ptr->initialize(nullptr));
        CHECK(manager.initialize_calls == 1);
        CHECK(manager.gui == nullptr);
    }

    TEST_CASE("font manager registration owns font and rejects duplicate names") {
        MockIFontManager manager;
        CHECK(manager.register_font(std::make_unique<MockIFont>("Body16", "Body", 16.0f)));
        CHECK_FALSE(manager.register_font(std::make_unique<MockIFont>("Body16", "Other", 20.0f)));
        CHECK(manager.get_font("Body16") != nullptr);
        CHECK(manager.get_registered_font_names().size() == 1);
    }

    TEST_CASE("font manager registered names and unique families are deterministic") {
        MockIFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Mono18", "Mono", 18.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Body16", "Body", 16.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Body24", "Body", 24.0f)));
        const auto names = manager.get_registered_font_names();
        const auto families = manager.get_unique_font_families();
        REQUIRE(names.size() == 3);
        CHECK(names[0] == "Body16");
        CHECK(names[1] == "Body24");
        CHECK(names[2] == "Mono18");
        REQUIRE(families.size() == 2);
        CHECK(families[0] == "Body");
        CHECK(families[1] == "Mono");
    }

    TEST_CASE("font manager family size query filters and sorts sizes") {
        MockIFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Body24", "Body", 24.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Body12", "Body", 12.0f)));
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Mono18", "Mono", 18.0f)));
        const auto sizes = manager.get_font_sizes_for_font_family("Body");
        REQUIRE(sizes.size() == 2);
        CHECK(sizes[0] == doctest::Approx(12.0f));
        CHECK(sizes[1] == doctest::Approx(24.0f));
    }

    TEST_CASE("font manager active selection and purge clear owned pointer safely") {
        MockIFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Active", "Body", 16.0f)));
        REQUIRE(manager.set_active_font("Active"));
        REQUIRE(manager.get_active_font() != nullptr);
        CHECK(manager.purge_font("Active"));
        CHECK(manager.get_active_font() == nullptr);
        CHECK(manager.get_font("Active") == nullptr);
    }

    TEST_CASE("font manager apply and atlas rebuild are independently observable") {
        MockIFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Body16", "Body", 16.0f)));
        CHECK(manager.apply_font("Body16"));
        CHECK_FALSE(manager.apply_font("Missing"));
        manager.rebuild_result = false;
        CHECK_FALSE(manager.rebuild_font_atlas_texture());
        CHECK(manager.apply_calls == 2);
        CHECK(manager.rebuild_calls == 1);
    }

    TEST_CASE("font manager batch and default setup preserve exact request metadata") {
        MockIFontManager manager;
        manager.create_font_batch("Fonts/body.ttf", "Body", 8.0f, 72.0f);
        manager.setup_default_fonts();
        CHECK(manager.create_batch_calls == 1);
        CHECK(manager.last_batch_path == "Fonts/body.ttf");
        CHECK(manager.last_batch_family == "Body");
        CHECK(manager.last_batch_min == doctest::Approx(8.0f));
        CHECK(manager.last_batch_max == doctest::Approx(72.0f));
        CHECK(manager.setup_default_calls == 1);
    }

    TEST_CASE("font manager file and directory loaders transport arguments exactly") {
        MockIFontManager manager;
        CHECK(manager.load_font_from_file("Fonts/ui.ttf", "UI18", 18.0f, true));
        CHECK(manager.last_file_path == "Fonts/ui.ttf");
        CHECK(manager.last_file_name == "UI18");
        CHECK(manager.last_file_size == doctest::Approx(18.0f));
        CHECK(manager.last_apply_immediately);
        CHECK(manager.load_fonts_from_directory("Fonts"));
        CHECK(manager.last_directory == "Fonts");
    }

    TEST_CASE("font manager shutdown clears registry and active state") {
        MockIFontManager manager;
        REQUIRE(manager.register_font(std::make_unique<MockIFont>("Body16", "Body", 16.0f)));
        REQUIRE(manager.set_active_font("Body16"));
        manager.shutdown();
        CHECK(manager.shutdown_calls == 1);
        CHECK(manager.get_registered_font_names().empty());
        CHECK(manager.get_active_font() == nullptr);
    }
}
