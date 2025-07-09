// Salix/gui/imgui/ImGuiTheme.cpp
#include <Salix/gui/imgui/ImGuiTheme.h>
#include <Salix/gui/IGui.h>
#include <Salix/gui/imgui/ImGuiThemeData.h> // Correctly included for ImGuiThemeData definition
#include <Salix/gui/imgui/SDLImGui.h>     // Correctly included for dynamic_cast<SDLImGui*>
#include <iostream>                       // For std::cerr, std::cout

// --- Required Includes for ImGui Core and Backend (No #ifdefs here) ---
// These are included directly because this file is part of the ImGui backend implementation
// which is always compiled for the universal engine DLL.
#include <imgui.h>                     // For ImGuiStyle, ImGuiCol_, ImVec4 etc.
#include <backends/imgui_impl_sdlrenderer2.h> // For ImGui_ImplSDLRenderer2_CreateFontsTexture
// --- End Required Includes ---


namespace Salix {

    // Pimpl struct definition (should be in ImGuiTheme.h if using Pimpl)
    struct ImGuiTheme::Pimpl {
        std::string name;
        std::unique_ptr<ImGuiThemeData> theme_data;
    };

    // Constructors
    ImGuiTheme::ImGuiTheme() : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = "Default ImGui Theme"; // Default name
        pimpl->theme_data = std::make_unique<ImGuiThemeData>(); // Default data
    }


    ImGuiTheme::ImGuiTheme(const std::string& theme_name)
        : pimpl(std::make_unique<Pimpl>()) {
        pimpl->name = theme_name;
        pimpl->theme_data = std::make_unique<ImGuiThemeData>(theme_name); // Pass name to data constructor
    }

    // Destructor (must be defined in .cpp for unique_ptr Pimpl)
    ImGuiTheme::~ImGuiTheme() = default;


    // Getters
    const std::string& ImGuiTheme::get_name() const {
        return pimpl->name;
    }

    ImGuiThemeData* ImGuiTheme::get_data() { 
        return pimpl->theme_data.get();
    }

}