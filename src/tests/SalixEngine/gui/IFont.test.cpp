#include <doctest.h>
#include <Salix/gui/IFont.h>
#include <Tests/SalixEngine/mocking/gui/MockIFont.h>
#include <Tests/SalixEngine/mocking/gui/MockIFontData.h>
#include <memory>
#include <cstdint>
#include <string>

TEST_SUITE("Salix::gui::IFontBatch1447") {
    TEST_CASE("font interface exposes configured name family and data") {
        MockIFont font("Body_16px", "Body", 16.0f);
        Salix::IFont* interface_ptr = &font;
        CHECK(interface_ptr->get_name() == "Body_16px");
        CHECK(interface_ptr->get_family() == "Body");
        REQUIRE(interface_ptr->get_data() != nullptr);
        CHECK(interface_ptr->get_data()->get_font_size() == doctest::Approx(16.0f));
    }

    TEST_CASE("font family setter updates family without changing font identity") {
        MockIFont font("Body_16px", "Body", 16.0f);
        font.set_family("Readable");
        CHECK(font.get_family() == "Readable");
        CHECK(font.get_name() == "Body_16px");
    }

    TEST_CASE("font data pointer is stable until data replacement") {
        MockIFont font("Body_16px", "Body", 16.0f);
        Salix::IFontData* first = font.get_data();
        REQUIRE(first != nullptr);
        first->set_font_size(18.0f);
        CHECK(font.get_data() == first);
        CHECK(font.get_data()->get_font_size() == doctest::Approx(18.0f));
    }

    TEST_CASE("font set data copies incoming font data contract") {
        MockIFont font("Old", "Family", 12.0f);
        MockIFontData replacement("New", "new.ttf", 30.0f, "ImGui");
        font.set_data(&replacement);
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_name() == "New");
        CHECK(font.get_data()->get_file_path() == "new.ttf");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(30.0f));
        CHECK(font.get_data()->get_gui_type() == "ImGui");
    }

    TEST_CASE("font set data ignores null replacement") {
        MockIFont font("Keep", "Family", 14.0f);
        Salix::IFontData* before = font.get_data();
        font.set_data(nullptr);
        CHECK(font.get_data() == before);
        CHECK(font.get_name() == "Keep");
    }

    TEST_CASE("font data copy remains independent from source after replacement") {
        MockIFont font("Old", "Family", 12.0f);
        MockIFontData replacement("Copied", "copied.ttf", 20.0f, "Mock");
        font.set_data(&replacement);
        replacement.set_font_size(99.0f);
        replacement.set_name("ChangedSource");
        CHECK(font.get_name() == "Copied");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(20.0f));
    }

    TEST_CASE("font ImGui pointer defaults to null") {
        MockIFont font;
        Salix::IFont* interface_ptr = &font;
        CHECK(interface_ptr->get_imgui_font_ptr() == nullptr);
    }

    TEST_CASE("font ImGui pointer transports exact opaque font address") {
        MockIFont font;
        ImFont* fake = reinterpret_cast<ImFont*>(static_cast<uintptr_t>(0x1234));
        font.set_imgui_font_ptr(fake);
        CHECK(font.get_imgui_font_ptr() == fake);
    }

    TEST_CASE("separate font instances keep independent family and data state") {
        MockIFont first("First", "Sans", 12.0f);
        MockIFont second("Second", "Mono", 24.0f);
        first.set_family("Serif");
        first.get_data()->set_font_size(18.0f);
        CHECK(second.get_family() == "Mono");
        CHECK(second.get_data()->get_font_size() == doctest::Approx(24.0f));
    }

    TEST_CASE("font interface supports polymorphic destruction") {
        std::unique_ptr<Salix::IFont> font =
            std::make_unique<MockIFont>("Poly", "Family", 15.0f);
        REQUIRE(font != nullptr);
        CHECK(font->get_name() == "Poly");
        font->set_family("Changed");
        CHECK(font->get_family() == "Changed");
    }
}
