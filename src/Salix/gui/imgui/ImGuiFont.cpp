// Salix/gui/imgui/ImGuiFont.cpp
#include <Salix/gui/imgui/ImGuiFont.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <imgui/imgui.h>
#include <iostream>
#include <memory>

namespace Salix {

    // Pimpl struct definition
    struct ImGuiFont::Pimpl {
        std::unique_ptr<ImGuiFontData> data; // Owns the font data
        ImFont* imgui_font_ptr = nullptr; // Non-owning pointer to the actual ImGui font object
        std::string font_family;
    };


    ImGuiFont::ImGuiFont() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->data = std::make_unique<ImGuiFontData>();
        pimpl->font_family = "undefined";

    }

    ImGuiFont::~ImGuiFont() = default;

    // Constructor for font from file
    ImGuiFont::ImGuiFont(const std::string& file_path, const std::string& name, float size) :
        pimpl(std::make_unique<Pimpl>()) {
        pimpl->data = std::make_unique<ImGuiFontData>(name, file_path, size);
        pimpl->font_family = "undefined";
    }

   
    // IFont overrides
    const std::string& ImGuiFont::get_name() const {
        return pimpl->data->get_name();
    }

    IFontData* ImGuiFont::get_data() const {
        return pimpl->data.get();
    }

    void ImGuiFont::set_family(const std::string& font_family) {
        pimpl->font_family = font_family;
    }

    const std::string& ImGuiFont::get_family() const {
        return pimpl->font_family;
    }

    ImFont* ImGuiFont::get_imgui_font_ptr() const {
        return pimpl->imgui_font_ptr;
    }

    // Setter for imgui_font_ptr (called by ImGuiFontManager after loading)
    void ImGuiFont::set_imgui_font_ptr(ImFont* ptr) {
        pimpl->imgui_font_ptr = ptr;
    }

    void ImGuiFont::set_data(IFontData* new_data) {
        if (!new_data) {
            std::cerr << "ImGuiFont::set_data - Attempted to set null data." << std::endl;
        return;
        }

        // Safely cast the interface pointer to the concrete type.
        ImGuiFontData* new_imgui_font_data = dynamic_cast<ImGuiFontData*>(new_data);
        if (!new_imgui_font_data) {
            std::cerr << "ImGuiFont::set_data - Provided data is not a compatible ImGuiFontData type." << std::endl;
        return;
        }

        // Create a new unique_ptr by copying the data from the raw pointer.
        // This assumes ImGuiFontData has a copy constructor.
        pimpl->data = std::make_unique<ImGuiFontData>(*new_imgui_font_data);
    }
    
} // namespace Salix}

