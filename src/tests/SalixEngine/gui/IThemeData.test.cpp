#include <doctest.h>
#include <Salix/gui/IThemeData.h>
#include <Tests/SalixEngine/mocking/gui/MockIThemeData.h>
#include <memory>

TEST_SUITE("Salix::gui::IThemeDataBatch1447") {
    TEST_CASE("theme data interface exposes configured name and GUI type") {
        MockIThemeData data("Dark", "ImGui");
        Salix::IThemeData* interface_ptr = &data;
        CHECK(interface_ptr->get_name() == "Dark");
        CHECK(interface_ptr->get_gui_type() == "ImGui");
    }

    TEST_CASE("theme data permits empty name without changing GUI type") {
        MockIThemeData data("", "ImGui");
        CHECK(data.get_name().empty());
        CHECK(data.get_gui_type() == "ImGui");
    }

    TEST_CASE("theme data permits empty GUI type without changing name") {
        MockIThemeData data("Theme", "");
        CHECK(data.get_name() == "Theme");
        CHECK(data.get_gui_type().empty());
    }

    TEST_CASE("theme data name mutation is independently observable") {
        MockIThemeData data("Old", "MockGui");
        data.set_name("New");
        CHECK(data.get_name() == "New");
        CHECK(data.get_gui_type() == "MockGui");
    }

    TEST_CASE("theme data GUI type mutation is independently observable") {
        MockIThemeData data("Theme", "OldGui");
        data.set_gui_type("NewGui");
        CHECK(data.get_name() == "Theme");
        CHECK(data.get_gui_type() == "NewGui");
    }

    TEST_CASE("theme data name getter returns stable internal storage across replacement") {
        MockIThemeData data("Short", "Gui");
        const std::string* storage = &data.get_name();
        data.set_name("LongerThemeName");
        CHECK(&data.get_name() == storage);
        CHECK(*storage == "LongerThemeName");
    }

    TEST_CASE("theme data GUI type getter returns stable internal storage across replacement") {
        MockIThemeData data("Theme", "A");
        const std::string* storage = &data.get_gui_type();
        data.set_gui_type("DifferentGui");
        CHECK(&data.get_gui_type() == storage);
        CHECK(*storage == "DifferentGui");
    }

    TEST_CASE("separate theme data instances keep independent names") {
        MockIThemeData first("First", "Gui");
        MockIThemeData second("Second", "Gui");
        first.set_name("Changed");
        CHECK(first.get_name() == "Changed");
        CHECK(second.get_name() == "Second");
    }

    TEST_CASE("separate theme data instances keep independent GUI types") {
        MockIThemeData first("One", "GuiA");
        MockIThemeData second("Two", "GuiB");
        first.set_gui_type("ChangedA");
        CHECK(second.get_gui_type() == "GuiB");
    }

    TEST_CASE("theme data interface supports polymorphic destruction") {
        std::unique_ptr<Salix::IThemeData> data =
            std::make_unique<MockIThemeData>("PolyTheme", "MockGui");
        REQUIRE(data != nullptr);
        CHECK(data->get_name() == "PolyTheme");
        CHECK(data->get_gui_type() == "MockGui");
    }
}

TEST_SUITE("Salix::gui::IThemeDataBatch6647") {
    TEST_CASE("batch6647 default mock theme data remains canonical") {
        MockIThemeData data;
        CHECK(data.get_name() == "MockThemeData");
        CHECK(data.get_gui_type() == "MockGui");
    }

    TEST_CASE("batch6647 both strings may be empty simultaneously") {
        MockIThemeData data("", "");
        CHECK(data.get_name().empty());
        CHECK(data.get_gui_type().empty());
    }

    TEST_CASE("batch6647 name preserves spaces punctuation and traversal text") {
        MockIThemeData data;
        data.set_name("../Theme #6647");
        CHECK(data.get_name() == "../Theme #6647");
    }

    TEST_CASE("batch6647 GUI type preserves spaces and punctuation") {
        MockIThemeData data;
        data.set_gui_type("Mock GUI #6647");
        CHECK(data.get_gui_type() == "Mock GUI #6647");
    }

    TEST_CASE("batch6647 name preserves embedded null bytes") {
        MockIThemeData data;
        data.set_name(std::string("Theme\0Data", 10));
        CHECK(data.get_name().size() == 10);
        CHECK(data.get_name()[5] == '\0');
    }

    TEST_CASE("batch6647 GUI type preserves embedded null bytes") {
        MockIThemeData data;
        data.set_gui_type(std::string("Gui\0Type", 8));
        CHECK(data.get_gui_type().size() == 8);
        CHECK(data.get_gui_type()[3] == '\0');
    }

    TEST_CASE("batch6647 copy remains independent after source name mutation") {
        MockIThemeData source("Source6647", "Gui6647");
        MockIThemeData copy = source;
        source.set_name("Changed6647");
        CHECK(copy.get_name() == "Source6647");
        CHECK(copy.get_gui_type() == "Gui6647");
    }

    TEST_CASE("batch6647 copy remains independent after source GUI type mutation") {
        MockIThemeData source("Source6647", "Gui6647");
        MockIThemeData copy = source;
        source.set_gui_type("ChangedGui6647");
        CHECK(copy.get_name() == "Source6647");
        CHECK(copy.get_gui_type() == "Gui6647");
    }

    TEST_CASE("batch6647 assignment replaces both owned strings") {
        MockIThemeData source("Assigned6647", "AssignedGui6647");
        MockIThemeData destination("Old6647", "OldGui6647");
        destination = source;
        CHECK(destination.get_name() == "Assigned6647");
        CHECK(destination.get_gui_type() == "AssignedGui6647");
    }

    TEST_CASE("batch6647 base interface observes later concrete mutations") {
        MockIThemeData data("Before6647", "BeforeGui6647");
        Salix::IThemeData& interface = data;
        data.set_name("After6647");
        data.set_gui_type("AfterGui6647");
        CHECK(interface.get_name() == "After6647");
        CHECK(interface.get_gui_type() == "AfterGui6647");
    }
}
