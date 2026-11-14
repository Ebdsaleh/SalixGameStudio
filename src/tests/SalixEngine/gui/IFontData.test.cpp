#include <doctest.h>
#include <Salix/gui/IFontData.h>
#include <Tests/SalixEngine/mocking/gui/MockIFontData.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::gui::IFontDataBatch1447") {
    TEST_CASE("font data interface exposes configured identity path size and GUI type") {
        MockIFontData data("Body16", "Fonts/body.ttf", 16.0f, "ImGui");
        Salix::IFontData* interface_ptr = &data;
        CHECK(interface_ptr->get_name() == "Body16");
        CHECK(interface_ptr->get_file_path() == "Fonts/body.ttf");
        CHECK(interface_ptr->get_font_size() == doctest::Approx(16.0f));
        CHECK(interface_ptr->get_gui_type() == "ImGui");
    }

    TEST_CASE("font data size setter updates only size") {
        MockIFontData data("Body", "body.ttf", 12.0f, "MockGui");
        data.set_font_size(28.0f);
        CHECK(data.get_font_size() == doctest::Approx(28.0f));
        CHECK(data.get_name() == "Body");
        CHECK(data.get_file_path() == "body.ttf");
    }

    TEST_CASE("font data permits fractional sizes without hidden rounding") {
        MockIFontData data;
        Salix::IFontData* interface_ptr = &data;
        interface_ptr->set_font_size(13.75f);
        CHECK(interface_ptr->get_font_size() == doctest::Approx(13.75f));
    }

    TEST_CASE("font data permits zero size as representable contract state") {
        MockIFontData data;
        data.set_font_size(0.0f);
        CHECK(data.get_font_size() == doctest::Approx(0.0f));
    }

    TEST_CASE("font data permits negative size for explicit validation tests") {
        MockIFontData data;
        data.set_font_size(-5.0f);
        CHECK(data.get_font_size() == doctest::Approx(-5.0f));
    }

    TEST_CASE("font data name mutation is independent of path and size") {
        MockIFontData data("Old", "font.ttf", 18.0f, "ImGui");
        data.set_name("New");
        CHECK(data.get_name() == "New");
        CHECK(data.get_file_path() == "font.ttf");
        CHECK(data.get_font_size() == doctest::Approx(18.0f));
    }

    TEST_CASE("font data path mutation is independent of name and GUI type") {
        MockIFontData data("Name", "old.ttf", 18.0f, "ImGui");
        data.set_file_path("nested/new.otf");
        CHECK(data.get_file_path() == "nested/new.otf");
        CHECK(data.get_name() == "Name");
        CHECK(data.get_gui_type() == "ImGui");
    }

    TEST_CASE("font data GUI type mutation remains independently observable") {
        MockIFontData data("Name", "font.ttf", 18.0f, "ImGui");
        data.set_gui_type("CustomGui");
        CHECK(data.get_gui_type() == "CustomGui");
        CHECK(data.get_name() == "Name");
    }

    TEST_CASE("separate font data instances do not share mutable state") {
        MockIFontData first("First", "first.ttf", 10.0f, "A");
        MockIFontData second("Second", "second.ttf", 20.0f, "B");
        first.set_font_size(30.0f);
        first.set_name("Changed");
        CHECK(second.get_name() == "Second");
        CHECK(second.get_font_size() == doctest::Approx(20.0f));
        CHECK(second.get_file_path() == "second.ttf");
    }

    TEST_CASE("font data interface supports polymorphic destruction") {
        std::unique_ptr<Salix::IFontData> data =
            std::make_unique<MockIFontData>("Poly", "poly.ttf", 22.0f, "Mock");
        REQUIRE(data != nullptr);
        CHECK(data->get_name() == "Poly");
        data->set_font_size(24.0f);
        CHECK(data->get_font_size() == doctest::Approx(24.0f));
    }
}

TEST_SUITE("Salix::gui::IFontDataBatch6547") {
    TEST_CASE("batch6547 default mock font data remains canonical") {
        MockIFontData data;
        CHECK(data.get_name() == "MockFont");
        CHECK(data.get_file_path() == "mock.ttf");
        CHECK(data.get_font_size() == doctest::Approx(16.0f));
        CHECK(data.get_gui_type() == "MockGui");
    }

    TEST_CASE("batch6547 empty name is stored without changing other fields") {
        MockIFontData data("Name", "font.ttf", 18.0f, "Gui");
        data.set_name("");
        CHECK(data.get_name().empty());
        CHECK(data.get_file_path() == "font.ttf");
        CHECK(data.get_font_size() == doctest::Approx(18.0f));
    }

    TEST_CASE("batch6547 empty file path is stored without changing name") {
        MockIFontData data("Name", "font.ttf", 18.0f, "Gui");
        data.set_file_path("");
        CHECK(data.get_file_path().empty());
        CHECK(data.get_name() == "Name");
    }

    TEST_CASE("batch6547 empty GUI type is stored independently") {
        MockIFontData data("Name", "font.ttf", 18.0f, "Gui");
        data.set_gui_type("");
        CHECK(data.get_gui_type().empty());
        CHECK(data.get_name() == "Name");
    }

    TEST_CASE("batch6547 large finite font size roundtrips exactly") {
        MockIFontData data;
        data.set_font_size(6547.25f);
        CHECK(data.get_font_size() == doctest::Approx(6547.25f));
    }

    TEST_CASE("batch6547 repeated size assignment keeps latest value") {
        MockIFontData data;
        data.set_font_size(6.0f);
        data.set_font_size(54.0f);
        data.set_font_size(7.0f);
        CHECK(data.get_font_size() == doctest::Approx(7.0f));
    }

    TEST_CASE("batch6547 font name preserves embedded null bytes") {
        MockIFontData data;
        data.set_name(std::string("Font\0Name", 9));
        CHECK(data.get_name().size() == 9);
        CHECK(data.get_name()[4] == '\0');
    }

    TEST_CASE("batch6547 path preserves spaces and parent traversal text") {
        MockIFontData data;
        data.set_file_path("../My Fonts/Body 6547.ttf");
        CHECK(data.get_file_path() == "../My Fonts/Body 6547.ttf");
    }

    TEST_CASE("batch6547 copy remains independent after source mutation") {
        MockIFontData source("Source6547", "source.ttf", 17.0f, "GuiA");
        MockIFontData copy = source;
        source.set_name("Changed6547");
        source.set_font_size(99.0f);
        CHECK(copy.get_name() == "Source6547");
        CHECK(copy.get_font_size() == doctest::Approx(17.0f));
        CHECK(copy.get_file_path() == "source.ttf");
        CHECK(copy.get_gui_type() == "GuiA");
    }

    TEST_CASE("batch6547 base interface setter dispatches to concrete size storage") {
        MockIFontData data;
        Salix::IFontData& interface = data;
        interface.set_font_size(65.47f);
        CHECK(interface.get_font_size() == doctest::Approx(65.47f));
    }
}
