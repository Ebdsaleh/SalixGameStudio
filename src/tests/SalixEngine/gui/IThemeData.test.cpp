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
