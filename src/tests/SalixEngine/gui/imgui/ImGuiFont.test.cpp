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
TEST_SUITE("Salix::gui::ImGuiFontBatch3447") {
    TEST_CASE("batch3447 owned data pointer remains stable across family mutations") {
        Salix::ImGuiFont font("stable.ttf", "Stable", 14.f);
        auto* data = font.get_data();
        REQUIRE(data != nullptr);
        font.set_family("First3447");
        font.set_family("Second3447");
        CHECK(font.get_data() == data);
        CHECK(font.get_family() == "Second3447");
    }

    TEST_CASE("batch3447 clearing runtime font pointer preserves metadata and family") {
        Salix::ImGuiFont font("runtime.ttf", "Runtime", 17.f);
        font.set_family("RuntimeFamily3447");
        ImFont* fake = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x3447));
        font.set_imgui_font_ptr(fake);
        REQUIRE(font.get_imgui_font_ptr() == fake);
        font.set_imgui_font_ptr(nullptr);
        CHECK(font.get_imgui_font_ptr() == nullptr);
        CHECK(font.get_name() == "Runtime");
        CHECK(font.get_family() == "RuntimeFamily3447");
    }

    TEST_CASE("batch3447 compatible data replacement installs a distinct owned allocation") {
        Salix::ImGuiFont font("old.ttf", "Old", 10.f);
        auto* before = font.get_data();
        Salix::ImGuiFontData replacement("New3447", "new3447.ttf", 22.f);
        font.set_data(&replacement);
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data() != before);
        CHECK(font.get_name() == "New3447");
    }

    TEST_CASE("batch3447 replacing data from its own current snapshot is safe and value preserving") {
        Salix::ImGuiFont font("self.ttf", "Self3447", 19.f);
        auto* before = font.get_data();
        REQUIRE(before != nullptr);
        font.set_data(before);
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data() != before);
        CHECK(font.get_name() == "Self3447");
        CHECK(font.get_data()->get_file_path() == "self.ttf");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(19.f));
    }

    TEST_CASE("batch3447 compatible replacement copies glyph offset configuration by value") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData replacement("Offset3447", "offset3447.ttf", 20.f);
        replacement.config.glyph_offset = ImVec2(-4.5f, 6.25f);
        font.set_data(&replacement);
        auto* stored = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(stored != nullptr);
        CHECK(stored->config.glyph_offset.x == doctest::Approx(-4.5f));
        CHECK(stored->config.glyph_offset.y == doctest::Approx(6.25f));
    }

    TEST_CASE("batch3447 compatible replacement preserves configured glyph range pointer identity") {
        static const ImWchar ranges[] = {0x20, 0x7e, 0};
        Salix::ImGuiFont font;
        Salix::ImGuiFontData replacement("Ranges3447", "ranges3447.ttf", 16.f);
        replacement.config.glyph_ranges = ranges;
        font.set_data(&replacement);
        auto* stored = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(stored != nullptr);
        CHECK(stored->config.glyph_ranges == ranges);
    }

    TEST_CASE("batch3447 IFont interface dispatch replaces compatible concrete data") {
        Salix::ImGuiFont concrete;
        Salix::IFont& base = concrete;
        Salix::ImGuiFontData replacement("Base3447", "base3447.ttf", 23.f);
        base.set_data(&replacement);
        CHECK(base.get_name() == "Base3447");
        REQUIRE(base.get_data() != nullptr);
        CHECK(base.get_data()->get_file_path() == "base3447.ttf");
    }

    TEST_CASE("batch3447 family storage preserves case whitespace and punctuation exactly") {
        Salix::ImGuiFont font;
        const std::string family = "  Mono-Case_3447  ";
        font.set_family(family);
        CHECK(font.get_family() == family);
        CHECK(font.get_family().size() == family.size());
    }

    TEST_CASE("batch3447 constructor preserves embedded null bytes in owned name and path strings") {
        const std::string path("font\0path.ttf", 13);
        const std::string name("Name\0Alt", 8);
        Salix::ImGuiFont font(path, name, 11.f);
        CHECK(font.get_name() == name);
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data()->get_file_path() == path);
    }

    TEST_CASE("batch3447 mutating owned font size through interface leaves family and runtime pointer intact") {
        Salix::ImGuiFont font("mutable.ttf", "Mutable3447", 12.f);
        font.set_family("Family3447");
        ImFont* fake = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x34470));
        font.set_imgui_font_ptr(fake);
        Salix::IFontData* data = font.get_data();
        REQUIRE(data != nullptr);
        data->set_font_size(31.25f);
        CHECK(data->get_font_size() == doctest::Approx(31.25f));
        CHECK(font.get_family() == "Family3447");
        CHECK(font.get_imgui_font_ptr() == fake);
    }
}
TEST_SUITE("Salix::gui::ImGuiFontBatch3747") {
    TEST_CASE("batch3747 separate default fonts own independent concrete data objects") {
        Salix::ImGuiFont first, second;
        REQUIRE(first.get_data() != nullptr);
        REQUIRE(second.get_data() != nullptr);
        CHECK(first.get_data() != second.get_data());
        first.get_data()->set_font_size(37.47f);
        CHECK(first.get_data()->get_font_size() == doctest::Approx(37.47f));
        CHECK(second.get_data()->get_font_size() == doctest::Approx(20.0f));
    }

    TEST_CASE("batch3747 copied replacement remains valid after source object leaves scope") {
        Salix::ImGuiFont font;
        {
            Salix::ImGuiFontData source("Scoped3747", "scoped3747.ttf", 23.0f);
            source.config.merge_mode = true;
            font.set_data(&source);
        }
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_name() == "Scoped3747");
        CHECK(font.get_data()->get_file_path() == "scoped3747.ttf");
        auto* stored = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(stored != nullptr);
        CHECK(stored->config.merge_mode);
    }

    TEST_CASE("batch3747 compatible replacement copies loader flags and rasterizer density") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source("Config3747", "config3747.ttf", 18.0f);
        source.config.font_loader_flags = 0x3747u;
        source.config.rasterizer_density = 3.25f;
        font.set_data(&source);
        auto* stored = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(stored != nullptr);
        CHECK(stored->config.font_loader_flags == 0x3747u);
        CHECK(stored->config.rasterizer_density == doctest::Approx(3.25f));
    }

    TEST_CASE("batch3747 source config mutation after replacement never changes owned snapshot") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source("Snapshot3747", "snapshot3747.ttf", 19.0f);
        source.config.glyph_offset = ImVec2(1.0f, 2.0f);
        font.set_data(&source);
        source.config.glyph_offset = ImVec2(9.0f, 10.0f);
        auto* stored = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(stored != nullptr);
        CHECK(stored->config.glyph_offset.x == doctest::Approx(1.0f));
        CHECK(stored->config.glyph_offset.y == doctest::Approx(2.0f));
    }

    TEST_CASE("batch3747 compatible replacement copies public GUI type value") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source("GuiType3747", "gui3747.ttf", 17.0f);
        source.gui_type = "ImGuiCustom3747";
        font.set_data(&source);
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data()->get_gui_type() == "ImGuiCustom3747");
    }

    TEST_CASE("batch3747 compatible replacement accepts empty metadata values exactly") {
        Salix::ImGuiFont font("old.ttf", "Old3747", 16.0f);
        Salix::ImGuiFontData source("", "", 0.0f);
        font.set_data(&source);
        CHECK(font.get_name().empty());
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data()->get_file_path().empty());
        CHECK(font.get_data()->get_font_size() == doctest::Approx(0.0f));
    }

    TEST_CASE("batch3747 family storage preserves embedded null bytes") {
        Salix::ImGuiFont font;
        const std::string family("Family\0Hidden3747", 17);
        font.set_family(family);
        CHECK(font.get_family() == family);
        CHECK(font.get_family().size() == family.size());
    }

    TEST_CASE("batch3747 null data replacement preserves family runtime pointer and owned data") {
        Salix::ImGuiFont font("stable3747.ttf", "Stable3747", 21.0f);
        font.set_family("Family3747");
        ImFont* runtime = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x3747));
        font.set_imgui_font_ptr(runtime);
        auto* data = font.get_data();
        REQUIRE(data != nullptr);
        font.set_data(nullptr);
        CHECK(font.get_data() == data);
        CHECK(font.get_family() == "Family3747");
        CHECK(font.get_imgui_font_ptr() == runtime);
        CHECK(font.get_name() == "Stable3747");
    }

    TEST_CASE("batch3747 foreign data rejection preserves current compatible snapshot and runtime state") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source("Compatible3747", "compatible3747.ttf", 24.0f);
        font.set_data(&source);
        font.set_family("KeepFamily3747");
        ImFont* runtime = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x37470));
        font.set_imgui_font_ptr(runtime);
        Batch1047ForeignFontData foreign;
        font.set_data(&foreign);
        CHECK(font.get_name() == "Compatible3747");
        CHECK(font.get_data()->get_file_path() == "compatible3747.ttf");
        CHECK(font.get_family() == "KeepFamily3747");
        CHECK(font.get_imgui_font_ptr() == runtime);
    }

    TEST_CASE("batch3747 multiple compatible replacements preserve family and runtime while latest values win") {
        Salix::ImGuiFont font;
        font.set_family("PersistentFamily3747");
        ImFont* runtime = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x37471));
        font.set_imgui_font_ptr(runtime);
        Salix::ImGuiFontData first("First3747", "first3747.ttf", 10.0f);
        Salix::ImGuiFontData second("Second3747", "second3747.ttf", 30.0f);
        font.set_data(&first);
        font.set_data(&second);
        CHECK(font.get_name() == "Second3747");
        CHECK(font.get_data()->get_file_path() == "second3747.ttf");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(30.0f));
        CHECK(font.get_family() == "PersistentFamily3747");
        CHECK(font.get_imgui_font_ptr() == runtime);
    }
}

TEST_SUITE("Salix::gui::ImGuiFontBatch4447") {
    TEST_CASE("batch4447 default owned data is concrete ImGuiFontData with synchronized default size") {
        Salix::ImGuiFont font;
        auto* data = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(data != nullptr);
        CHECK(data->get_font_size() == doctest::Approx(20.0f));
        CHECK(data->config.size_pixels == doctest::Approx(20.0f));
    }

    TEST_CASE("batch4447 custom constructor synchronizes owned config size with requested size") {
        Salix::ImGuiFont font("font4447.ttf", "Font4447", 31.0f);
        auto* data = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(data != nullptr);
        CHECK(data->config.size_pixels == doctest::Approx(31.0f));
        CHECK(font.get_name() == "Font4447");
    }

    TEST_CASE("batch4447 family setter copies caller string storage") {
        Salix::ImGuiFont font;
        std::string family = "Family4447";
        font.set_family(family);
        family = "MutatedCaller";
        CHECK(font.get_family() == "Family4447");
    }

    TEST_CASE("batch4447 direct mutation through owned concrete data updates delegated name getter") {
        Salix::ImGuiFont font;
        auto* data = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(data != nullptr);
        data->name = "Direct4447";
        CHECK(font.get_name() == "Direct4447");
    }

    TEST_CASE("batch4447 self data replacement preserves complete compatible snapshot") {
        Salix::ImGuiFont font("self4447.ttf", "Self4447", 22.0f);
        auto* original = font.get_data();
        REQUIRE(original != nullptr);
        font.set_data(original);
        CHECK(font.get_name() == "Self4447");
        CHECK(font.get_data()->get_file_path() == "self4447.ttf");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(22.0f));
    }

    TEST_CASE("batch4447 compatible replacement copies custom GUI type") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData replacement("Gui4447", "gui4447.ttf", 13.0f);
        replacement.gui_type = "CustomGui4447";
        font.set_data(&replacement);
        REQUIRE(font.get_data() != nullptr);
        CHECK(font.get_data()->get_gui_type() == "CustomGui4447");
    }

    TEST_CASE("batch4447 compatible replacement copies ellipsis configuration") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData replacement("Ellipsis4447", "ellipsis4447.ttf", 14.0f);
        replacement.config.ellipsis_char = static_cast<ImWchar>(0x2026);
        font.set_data(&replacement);
        auto* stored = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(stored != nullptr);
        CHECK(stored->config.ellipsis_char == static_cast<ImWchar>(0x2026));
    }

    TEST_CASE("batch4447 compatible data replacement preserves runtime ImFont pointer") {
        Salix::ImGuiFont font;
        ImFont* runtime = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x4447));
        font.set_imgui_font_ptr(runtime);
        Salix::ImGuiFontData replacement("Runtime4447", "runtime4447.ttf", 15.0f);
        font.set_data(&replacement);
        CHECK(font.get_imgui_font_ptr() == runtime);
    }

    TEST_CASE("batch4447 compatible data replacement preserves family metadata") {
        Salix::ImGuiFont font;
        font.set_family("PersistentFamily4447");
        Salix::ImGuiFontData replacement("Data4447", "data4447.ttf", 16.0f);
        font.set_data(&replacement);
        CHECK(font.get_family() == "PersistentFamily4447");
    }

    TEST_CASE("batch4447 separate fonts retain independent data family and runtime pointer state") {
        Salix::ImGuiFont first("first4447.ttf", "First4447", 10.0f);
        Salix::ImGuiFont second("second4447.ttf", "Second4447", 20.0f);
        first.set_family("FamilyA4447");
        second.set_family("FamilyB4447");
        ImFont* first_runtime = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x44471));
        ImFont* second_runtime = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x44472));
        first.set_imgui_font_ptr(first_runtime);
        second.set_imgui_font_ptr(second_runtime);
        CHECK(first.get_data() != second.get_data());
        CHECK(first.get_family() == "FamilyA4447");
        CHECK(second.get_family() == "FamilyB4447");
        CHECK(first.get_imgui_font_ptr() == first_runtime);
        CHECK(second.get_imgui_font_ptr() == second_runtime);
    }
}

TEST_SUITE("Salix::gui::ImGuiFontBatch4947") {
    TEST_CASE("batch4947 default font combines undefined family null runtime and concrete owned data") {
        Salix::ImGuiFont font;
        CHECK(font.get_family() == "undefined");
        CHECK(font.get_imgui_font_ptr() == nullptr);
        CHECK(dynamic_cast<Salix::ImGuiFontData*>(font.get_data()) != nullptr);
    }

    TEST_CASE("batch4947 custom constructor leaves family undefined and runtime pointer null") {
        Salix::ImGuiFont font("custom4947.ttf", "Custom4947", 49.47f);
        CHECK(font.get_name() == "Custom4947");
        CHECK(font.get_data()->get_file_path() == "custom4947.ttf");
        CHECK(font.get_data()->get_font_size() == doctest::Approx(49.47f));
        CHECK(font.get_family() == "undefined");
        CHECK(font.get_imgui_font_ptr() == nullptr);
    }

    TEST_CASE("batch4947 family reference object address remains stable across replacements") {
        Salix::ImGuiFont font;
        const auto* family = &font.get_family();
        font.set_family("First4947");
        CHECK(&font.get_family() == family);
        font.set_family("Second4947LongerFamily");
        CHECK(&font.get_family() == family);
        CHECK(*family == "Second4947LongerFamily");
    }

    TEST_CASE("batch4947 runtime pointer can be assigned cleared and reassigned independently") {
        Salix::ImGuiFont font;
        ImFont* first = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x49471));
        ImFont* second = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x49472));
        font.set_imgui_font_ptr(first);
        CHECK(font.get_imgui_font_ptr() == first);
        font.set_imgui_font_ptr(nullptr);
        CHECK(font.get_imgui_font_ptr() == nullptr);
        font.set_imgui_font_ptr(second);
        CHECK(font.get_imgui_font_ptr() == second);
    }

    TEST_CASE("batch4947 runtime pointer mutation never changes data or family metadata") {
        Salix::ImGuiFont font("stable4947.ttf", "Stable4947", 19.0f);
        font.set_family("Family4947");
        auto* data = font.get_data();
        ImFont* runtime = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x49473));
        font.set_imgui_font_ptr(runtime);
        CHECK(font.get_data() == data);
        CHECK(font.get_name() == "Stable4947");
        CHECK(font.get_family() == "Family4947");
    }

    TEST_CASE("batch4947 direct owned data size mutation preserves family and runtime pointer") {
        Salix::ImGuiFont font;
        font.set_family("Family4947");
        ImFont* runtime = reinterpret_cast<ImFont*>(static_cast<std::uintptr_t>(0x49474));
        font.set_imgui_font_ptr(runtime);
        font.get_data()->set_font_size(33.0f);
        CHECK(font.get_data()->get_font_size() == doctest::Approx(33.0f));
        CHECK(font.get_family() == "Family4947");
        CHECK(font.get_imgui_font_ptr() == runtime);
    }

    TEST_CASE("batch4947 compatible data replacement preserves glyph range pointer identity in copied snapshot") {
        static const ImWchar ranges[] = {0x20, 0x7e, 0};
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source("Ranges4947", "ranges4947.ttf", 14.0f);
        source.config.glyph_ranges = ranges;
        font.set_data(&source);
        auto* stored = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(stored != nullptr);
        CHECK(stored->config.glyph_ranges == ranges);
    }

    TEST_CASE("batch4947 compatible data replacement copies config independently from later source mutation") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source("Config4947", "config4947.ttf", 16.0f);
        source.config.rasterizer_multiply = 2.0f;
        source.config.glyph_offset = ImVec2(4.0f, 9.0f);
        font.set_data(&source);
        source.config.rasterizer_multiply = 9.0f;
        source.config.glyph_offset = ImVec2(0.0f, 0.0f);
        auto* stored = dynamic_cast<Salix::ImGuiFontData*>(font.get_data());
        REQUIRE(stored != nullptr);
        CHECK(stored->config.rasterizer_multiply == doctest::Approx(2.0f));
        CHECK(stored->config.glyph_offset.x == doctest::Approx(4.0f));
        CHECK(stored->config.glyph_offset.y == doctest::Approx(9.0f));
    }

    TEST_CASE("batch4947 compatible data replacement preserves embedded null metadata bytes") {
        Salix::ImGuiFont font;
        Salix::ImGuiFontData source;
        source.name = std::string("Font\0Name4947", 13);
        source.file_path = std::string("Path\0Font4947", 13);
        font.set_data(&source);
        CHECK(font.get_name() == source.name);
        CHECK(font.get_data()->get_file_path() == source.file_path);
        CHECK(font.get_name().size() == source.name.size());
    }

    TEST_CASE("batch4947 family setter preserves whitespace and control characters exactly") {
        Salix::ImGuiFont font;
        const std::string family = " \tFamily 4947\n ";
        font.set_family(family);
        CHECK(font.get_family() == family);
    }
}
