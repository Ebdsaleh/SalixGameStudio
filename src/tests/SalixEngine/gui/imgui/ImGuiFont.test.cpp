#include <doctest.h>
#include <Salix/gui/imgui/ImGuiFont.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <Salix/gui/IFont.h>
#include <cstdint>
#include <string>
#include <Tests/SalixEngine/mocking/gui/MockIFontData.h>

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

TEST_SUITE("Salix::gui::ImGuiFontBatch1947") {
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

TEST_SUITE("Salix::gui::ImGuiFontBatch2547") {
    TEST_CASE("batch2547 default construction owns concrete ImGui font data") {
        Salix::ImGuiFont font; REQUIRE(font.get_data()!=nullptr); CHECK(dynamic_cast<Salix::ImGuiFontData*>(font.get_data())!=nullptr);
    }
    TEST_CASE("batch2547 file constructor preserves negative diagnostic font size exactly") {
        Salix::ImGuiFont font("diag.ttf","Diag",-5.5f); CHECK(font.get_name()=="Diag"); CHECK(font.get_data()->get_file_path()=="diag.ttf"); CHECK(font.get_data()->get_font_size()==doctest::Approx(-5.5f));
    }
    TEST_CASE("batch2547 family replacement does not mutate owned font metadata") {
        Salix::ImGuiFont font("font.ttf","Name",14.f); font.set_family("FamilyA"); font.set_family("FamilyB");
        CHECK(font.get_family()=="FamilyB"); CHECK(font.get_name()=="Name"); CHECK(font.get_data()->get_file_path()=="font.ttf");
    }
    TEST_CASE("batch2547 runtime font pointer may be replaced without altering metadata") {
        Salix::ImGuiFont font("font.ttf","Name",14.f);
        ImFont* first=reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x1000)); ImFont* second=reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x2000));
        font.set_imgui_font_ptr(first); CHECK(font.get_imgui_font_ptr()==first); font.set_imgui_font_ptr(second); CHECK(font.get_imgui_font_ptr()==second); CHECK(font.get_name()=="Name");
    }
    TEST_CASE("batch2547 compatible data replacement preserves source value snapshot") {
        Salix::ImGuiFont font; Salix::ImGuiFontData source("Snapshot2547","snapshot2547.ttf",25.47f); font.set_data(&source);
        source.name="Changed"; source.file_path="changed.ttf"; source.set_font_size(1.f);
        CHECK(font.get_name()=="Snapshot2547"); CHECK(font.get_data()->get_file_path()=="snapshot2547.ttf"); CHECK(font.get_data()->get_font_size()==doctest::Approx(25.47f));
    }
    TEST_CASE("batch2547 null replacement leaves data pointer and metadata intact") {
        Salix::ImGuiFont font("stable.ttf","Stable",18.f); auto* before=font.get_data(); font.set_data(nullptr);
        CHECK(font.get_data()==before); CHECK(font.get_name()=="Stable"); CHECK(font.get_data()->get_font_size()==doctest::Approx(18.f));
    }
    TEST_CASE("batch2547 foreign replacement leaves concrete data pointer intact") {
        Salix::ImGuiFont font("stable.ttf","Stable",18.f); auto* before=font.get_data(); Batch1047ForeignFontData foreign; font.set_data(&foreign);
        CHECK(font.get_data()==before); CHECK(font.get_name()=="Stable"); CHECK(font.get_data()->get_gui_type()=="ImGui");
    }
    TEST_CASE("batch2547 IFont base dispatch exposes exact family reference contents") {
        Salix::ImGuiFont concrete; Salix::IFont& base=concrete; concrete.set_family("Interface2547"); CHECK(base.get_family()=="Interface2547"); CHECK(&base.get_family()==&concrete.get_family());
    }
    TEST_CASE("batch2547 compatible replacement does not disturb family or runtime pointer") {
        Salix::ImGuiFont font; font.set_family("Keep2547"); ImFont* ptr=reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x2547)); font.set_imgui_font_ptr(ptr);
        Salix::ImGuiFontData replacement("New2547","new2547.ttf",30.f); font.set_data(&replacement);
        CHECK(font.get_family()=="Keep2547"); CHECK(font.get_imgui_font_ptr()==ptr); CHECK(font.get_name()=="New2547");
    }
    TEST_CASE("batch2547 empty family remains distinct from undefined constructor family") {
        Salix::ImGuiFont font; CHECK(font.get_family()=="undefined"); font.set_family(""); CHECK(font.get_family().empty()); CHECK(font.get_name()=="Default Font");
    }
}

