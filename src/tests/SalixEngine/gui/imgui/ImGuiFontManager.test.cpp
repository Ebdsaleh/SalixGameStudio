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

// ---- Relocated from src/tests/SalixEngine/gui/IFontManager.test.cpp; canonical tests mirror production .cpp files ----
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
