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
        ImGuiTheme(const std::string& theme_name, const std::string& file_path);
        ~ImGuiTheme() override;
        const std::string& get_name() const override;
        const std::string& get_file_path() const override;
        ImGuiThemeData* get_data();
        void apply(IGui* gui_system) override;
        bool load_from_yaml(const std::string& file_path) override;
        bool save_to_yaml(const std::string& file_path) const override;

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix
