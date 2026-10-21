#include <doctest.h>
#include <Salix/gui/imgui/ImGuiFont.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <Salix/gui/IFont.h>
#include <cstdint>
#include <string>

namespace {
class Batch1047ForeignFontData : public Salix::IFontData {
public:
    const std::string& get_name() const override { return name; }
    const std::string& get_file_path() const override { return path; }
    float get_font_size() const override { return size; }
    const std::string& get_gui_type() const override { return type; }
    void set_font_size(float value) override { size = value; }

private:
    std::string name = "Foreign";
    std::string path = "foreign.font";
    std::string type = "ForeignGui";
    float size = 10.0f;
};
}

TEST_SUITE("Salix::gui::ImGuiFontBatch1047") {
    TEST_CASE("default font owns default data undefined family and no runtime ImFont") {
        Salix::ImGuiFont font;
        CHECK(font.get_name() == "Default Font");
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data()->get_file_path().empty());
        CHECK(font.get_data()->get_font_size() == doctest::Approx(20.0f));
        CHECK(font.get_family() == "undefined");
        CHECK(font.get_imgui_font_ptr() == nullptr);
    }

    TEST_CASE("file constructor forwards path name and size into owned font data") {
        Salix::ImGuiFont font("Assets/Fonts/Mono.ttf", "Mono", 15.5f);
        CHECK(font.get_name() == "Mono");
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data()->get_file_path() == "Assets/Fonts/Mono.ttf");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(15.5f));
        CHECK(font.get_family() == "undefined");
    }

    TEST_CASE("font family can be replaced independently of font data") {
        Salix::ImGuiFont font("font.ttf", "Font", 12.0f);
        font.set_family("Monospace");
        CHECK(font.get_family() == "Monospace");
        CHECK(font.get_name() == "Font");
        CHECK(font.get_data()->get_file_path() == "font.ttf");
        font.set_family("");
        CHECK(font.get_family().empty());
    }

    TEST_CASE("runtime ImFont pointer is stored as a non-owning identity") {
        Salix::ImGuiFont font;
        ImFont* fake = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x1234));
        font.set_imgui_font_ptr(fake);
        CHECK(font.get_imgui_font_ptr() == fake);
        font.set_imgui_font_ptr(nullptr);
        CHECK(font.get_imgui_font_ptr() == nullptr);
    }

    TEST_CASE("compatible ImGuiFontData replaces owned data by value") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData replacement("Replacement", "replacement.ttf", 28.0f);
        replacement.config.merge_mode = true;
        font.set_data(&replacement);
        CHECK(font.get_name() == "Replacement");
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data()->get_file_path() == "replacement.ttf");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(28.0f));
        auto* concrete = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(concrete != nullptr);
        CHECK(concrete->config.merge_mode);
    }

    TEST_CASE("set data copies source so later source mutations cannot alter the font") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData replacement("Snapshot", "snapshot.ttf", 18.0f);
        font.set_data(&replacement);
        replacement.name = "Mutated";
        replacement.file_path = "mutated.ttf";
        replacement.set_font_size(99.0f);
        CHECK(font.get_name() == "Snapshot");
        CHECK(font.get_data()->get_file_path() == "snapshot.ttf");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(18.0f));
    }

    TEST_CASE("null data assignment is rejected without losing existing data") {
        Salix::ImGuiFont font("stable.ttf", "Stable", 13.0f);
        font.set_family("StableFamily");
        font.set_data(nullptr);
        CHECK(font.get_name() == "Stable");
        CHECK(font.get_data()->get_file_path() == "stable.ttf");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(13.0f));
        CHECK(font.get_family() == "StableFamily");
    }

    TEST_CASE("incompatible font data assignment is rejected without replacing owned ImGui data") {
        Salix::ImGuiFont font("stable.ttf", "Stable", 13.0f);
        Batch1047ForeignFontData foreign;
        font.set_data(&foreign);
        CHECK(font.get_name() == "Stable");
        CHECK(font.get_data()->get_gui_type() == "ImGui");
        CHECK(font.get_data()->get_file_path() == "stable.ttf");
    }

    TEST_CASE("IFont interface dispatches metadata family and runtime pointer operations") {
        Salix::ImGuiFont concrete("virtual.ttf", "Virtual", 11.0f);
        Salix::IFont* font = &concrete;
        CHECK(font->get_name() == "Virtual");
        REQUIRE(font->get_data() != nullptr);
        font->set_family("InterfaceFamily");
        CHECK(font->get_family() == "InterfaceFamily");
        ImFont* fake = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x4321));
        concrete.set_imgui_font_ptr(fake);
        CHECK(font->get_imgui_font_ptr() == fake);
    }

    TEST_CASE("replacing data preserves independent family and runtime font pointer state") {
        Salix::ImGuiFont font("old.ttf", "Old", 10.0f);
        font.set_family("KeepFamily");
        ImFont* fake = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x2222));
        font.set_imgui_font_ptr(fake);
        Salix::ImGuiFontData replacement("New", "new.ttf", 20.0f);
        font.set_data(&replacement);
        CHECK(font.get_name() == "New");
        CHECK(font.get_family() == "KeepFamily");
        CHECK(font.get_imgui_font_ptr() == fake);
    }
}
