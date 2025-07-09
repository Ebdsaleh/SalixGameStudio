// Salix/gui/imgui/ImGuiFont.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/IFont.h>
#include <Salix/gui/imgui/ImGuiFontData.h>
#include <memory>

// Forward declare ImFont (ImGui's internal font type)
struct ImFont;

namespace Salix {

    class SALIX_API ImGuiFont : public IFont {
    public:
        ImGuiFont();
        ImGuiFont(const std::string& file_path, const std::string& name, float size);

        ~ImGuiFont() override;

        // --- IFont overrides ---
        const std::string& get_name() const override; // RE-IMPLEMENTED
        IFontData* get_data() override;
        ImFont* get_imgui_font_ptr() const override; // RE-IMPLEMENTED

        // Setter for imgui_font_ptr (called by ImGuiFontManager after loading)
        void set_imgui_font_ptr(ImFont* ptr);

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
    };

} // namespace Salix