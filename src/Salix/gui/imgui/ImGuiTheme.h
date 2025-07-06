// Salix/gui/imgui/ImGuiTheme.h

#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/ITheme.h>
#include <Salix/gui/IGui.h>
#include <memory>
#include <string>

namespace Salix {
    struct ImGuiThemeData;

    struct SALIX_API ImGuiTheme : public ITheme {
    public:
        ImGuiTheme();
        ImGuiTheme(const std::string& theme_name);
        bool  apply(IGui* gui_system) override;
        const std::string& get_name() const override;
        ImGuiThemeData* get_theme_data();
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix
