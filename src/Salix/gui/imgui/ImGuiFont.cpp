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
    };


    ImGuiFont::ImGuiFont() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->data = std::make_unique<ImGuiFontData>();

    }

    ImGuiFont::~ImGuiFont() = default;

    // Constructor for font from file
    ImGuiFont::ImGuiFont(const std::string& file_path, const std::string& name, float size) :
        pimpl(std::make_unique<Pimpl>()) {
        pimpl->data = std::make_unique<ImGuiFontData>(name, file_path, size);
    }

   
    // IFont overrides
    const std::string& ImGuiFont::get_name() const {
        return pimpl->data->get_name();
    }

    IFontData* ImGuiFont::get_data() {
        return pimpl->data.get();
    }

    ImFont* ImGuiFont::get_imgui_font_ptr() const {
        return pimpl->imgui_font_ptr;
    }

    // Setter for imgui_font_ptr (called by ImGuiFontManager after loading)
    void ImGuiFont::set_imgui_font_ptr(ImFont* ptr) {
        pimpl->imgui_font_ptr = ptr;
    }

    
} // namespace Salix}

