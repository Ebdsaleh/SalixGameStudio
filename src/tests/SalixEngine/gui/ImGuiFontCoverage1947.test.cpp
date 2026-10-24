#include <doctest.h>
#include <Salix/gui/imgui/ImGuiFont.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <Salix/gui/IFont.h>
#include <Tests/SalixEngine/mocking/gui/MockIFontData.h>
#include <cstdint>
#include <string>

TEST_SUITE("Salix::gui::ImGuiFontCoverage1947") {
    TEST_CASE("owned font data can be edited through IFontData interface") {
        Salix::ImGuiFont font("font.ttf", "Editable", 12.0f);
        Salix::IFontData* data = font.get_data();
        REQUIRE(data != nullptr);
        data->set_font_size(19.5f);
        CHECK(font.get_data()->get_font_size() == doctest::Approx(19.5f));
    }

    TEST_CASE("direct concrete data mutation is immediately reflected by font name getter") {
        Salix::ImGuiFont font;
        auto* data = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(data != nullptr);
        data->name = "Renamed";
        CHECK(font.get_name() == "Renamed");
    }

    TEST_CASE("successive compatible data assignments make the most recent snapshot visible") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData first("First", "first.ttf", 10.0f);
        Salix::ImGuiFontData second("Second", "second.ttf", 20.0f);
        font.set_data(&first);
        CHECK(font.get_name() == "First");
        font.set_data(&second);
        CHECK(font.get_name() == "Second");
        CHECK(font.get_data()->get_file_path() == "second.ttf");
    }

    TEST_CASE("compatible data assignment copies ImGui config independently") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source("Configured", "configured.ttf", 16.0f);
        source.config.pixel_snap_h = true;
        source.config.rasterizer_density = 2.5f;
        font.set_data(&source);
        auto* stored = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(stored != nullptr);
        CHECK(stored->config.pixel_snap_h);
        CHECK(stored->config.rasterizer_density == doctest::Approx(2.5f));
        source.config.pixel_snap_h = false;
        source.config.rasterizer_density = 1.0f;
        CHECK(stored->config.pixel_snap_h);
        CHECK(stored->config.rasterizer_density == doctest::Approx(2.5f));
    }

    TEST_CASE("null assignment after successful replacement preserves replacement data") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source("Keep", "keep.ttf", 17.0f);
        font.set_data(&source);
        font.set_data(nullptr);
        CHECK(font.get_name() == "Keep");
        CHECK(font.get_data()->get_file_path() == "keep.ttf");
    }

    TEST_CASE("foreign data assignment after successful replacement preserves replacement data") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source("Keep", "keep.ttf", 17.0f);
        font.set_data(&source);
        MockIFontData foreign("Foreign", "foreign.ttf", 99.0f, "MockGui");
        font.set_data(&foreign);
        CHECK(font.get_name() == "Keep");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(17.0f));
    }

    TEST_CASE("font family stores embedded null bytes as ordinary string data") {
        Salix::ImGuiFont font;
        const std::string family("Mono\0Alt", 8);
        font.set_family(family);
        CHECK(font.get_family().size() == 8);
        CHECK(font.get_family() == family);
    }

    TEST_CASE("runtime ImFont pointer survives family changes") {
        Salix::ImGuiFont font;
        ImFont* fake = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0xABC0));
        font.set_imgui_font_ptr(fake);
        font.set_family("First");
        font.set_family("Second");
        CHECK(font.get_imgui_font_ptr() == fake);
    }

    TEST_CASE("runtime ImFont pointer survives compatible data replacement") {
        Salix::ImGuiFont font;
        ImFont* fake = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x1230));
        font.set_imgui_font_ptr(fake);
        Salix::ImGuiFontData replacement("New", "new.ttf", 22.0f);
        font.set_data(&replacement);
        CHECK(font.get_imgui_font_ptr() == fake);
        CHECK(font.get_name() == "New");
    }

    TEST_CASE("empty constructor strings remain valid independent font metadata") {
        Salix::ImGuiFont font("", "", 0.0f);
        CHECK(font.get_name().empty());
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data()->get_file_path().empty());
        CHECK(font.get_data()->get_font_size() == doctest::Approx(0.0f));
        CHECK(font.get_family() == "undefined");
    }
}
