// Salix/gui/imgui/ImGuiTheme.cpp

#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/imgui/ImGuiThemeData.h>
#include <imgui/imgui.h>

namespace Salix {

    struct ImGuiTheme::Pimpl {
        std::string name;
        std::unique_ptr<ImGuiThemeData>theme_data;
    };

    ImGuiTheme::ImGuiTheme() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = "New Theme";
        pimpl->theme_data = std::make_unique<ImGuiThemeData>();
    }

    ImGuiTheme::ImGuiTheme(const std::string& theme_name)
        :  pimpl(std::make_unique<Pimpl>()) {
            pimpl->name = theme_name;
            pimpl->theme_data = std::make_unique<ImGuiThemeData>();
    }

    bool ImGuiTheme::apply(IGui* gui_system) {
        if (gui_system == nullptr)  { return false; }
        // Apply default Dear ImGui style for now
        ImGui::StyleColorsDark();  // Temporary default
        return true;
        
    }

    const std::string& ImGuiTheme::get_name() const {
        return pimpl->name;
    }

    ImGuiThemeData* ImGuiTheme::get_theme_data() {
    return pimpl->theme_data.get();
}

} // namespace Salix
