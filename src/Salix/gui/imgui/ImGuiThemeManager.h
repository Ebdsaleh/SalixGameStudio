// Salix/gui/imgui/ImGuiThemeManager.h

#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/IThemeManager.h>
#include <Salix/gui/ITheme.h>
#include <unordered_map>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

struct ImVec4;
struct ImVec2;


namespace Salix {

    class SALIX_API ImGuiThemeManager : public IThemeManager {
    public:
        ImGuiThemeManager();
        ~ImGuiThemeManager() override;
        bool initialize(IGui* gui_system) override;
        void shutdown() override;

        bool register_theme(std::unique_ptr<ITheme> theme) override;
        bool unregister_theme(const std::string& theme_name) override;
        bool apply_theme(const std::string& theme_name) override;

        std::vector<std::string> get_registered_themes() const override;
        ITheme* get_theme(const std::string& theme_name) const override;
        ITheme* get_active_theme() override;
        bool load_internal_theme_from_file(const std::string& file_path, bool apply_immediately = false);
        bool load_theme_from_file(const std::string& theme_name, const std::string& file_path, bool apply_immediately = false) override;
        bool load_themes_from_directory(const std::string& directory_path) override;
        bool save_theme_to_file(const std::string& theme_name, const std::string& file_path) override;
        bool set_active_theme(const std::string& theme_name) override;
        bool purge_theme(const std::string& theme_name) override;
        void apply_style_scale(float scale) override;

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl>pimpl;
        
    };

} // namespace Salix
