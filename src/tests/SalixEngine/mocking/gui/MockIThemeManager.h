#pragma once

#include <Salix/gui/IThemeManager.h>
#include <Salix/gui/ITheme.h>
#include <Salix/gui/IGui.h>
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class MockIThemeManager : public Salix::IThemeManager {
public:
    bool initialize(Salix::IGui* gui_system) override {
        gui = gui_system;
        ++initialize_calls;
        return initialize_result;
    }

    void shutdown() override {
        ++shutdown_calls;
        active_theme = nullptr;
        registry.clear();
    }

    bool register_theme(std::unique_ptr<Salix::ITheme> theme) override {
        if (!theme) return false;
        const std::string name = theme->get_name();
        if (registry.find(name) != registry.end()) return false;
        registry.emplace(name, std::move(theme));
        return true;
    }

    bool unregister_theme(const std::string& theme_name) override {
        auto it = registry.find(theme_name);
        if (it == registry.end()) return false;
        if (active_theme == it->second.get()) active_theme = nullptr;
        registry.erase(it);
        return true;
    }

    bool apply_theme(const std::string& theme_name) override {
        ++apply_calls;
        auto it = registry.find(theme_name);
        if (it == registry.end()) return false;
        it->second->apply(gui);
        return true;
    }

    std::vector<std::string> get_registered_themes() const override {
        std::vector<std::string> names;
        for (const auto& [name, _] : registry) names.push_back(name);
        std::sort(names.begin(), names.end());
        return names;
    }

    Salix::ITheme* get_theme(const std::string& theme_name) const override {
        auto it = registry.find(theme_name);
        return it == registry.end() ? nullptr : it->second.get();
    }

    Salix::ITheme* get_active_theme() override { return active_theme; }

    bool load_theme_from_file(const std::string& theme_name, const std::string& file_path, bool apply_immediately = false) override {
        ++load_file_calls;
        last_theme_name = theme_name;
        last_theme_path = file_path;
        last_apply_immediately = apply_immediately;
        return load_file_result;
    }

    bool load_themes_from_directory(const std::string& directory_path) override {
        ++load_directory_calls;
        last_directory = directory_path;
        return load_directory_result;
    }

    bool save_theme_to_file(const std::string& theme_name, const std::string& file_path) override {
        ++save_file_calls;
        last_save_theme_name = theme_name;
        last_save_path = file_path;
        return save_file_result;
    }

    bool set_active_theme(const std::string& theme_name) override {
        auto it = registry.find(theme_name);
        if (it == registry.end()) return false;
        active_theme = it->second.get();
        return true;
    }

    bool purge_theme(const std::string& theme_name) override { return unregister_theme(theme_name); }

    void apply_style_scale(float scale) override {
        ++style_scale_calls;
        last_style_scale = scale;
    }

    Salix::IGui* gui = nullptr;
    bool initialize_result = true;
    bool load_file_result = true;
    bool load_directory_result = true;
    bool save_file_result = true;
    int initialize_calls = 0;
    int shutdown_calls = 0;
    int apply_calls = 0;
    int load_file_calls = 0;
    int load_directory_calls = 0;
    int save_file_calls = 0;
    int style_scale_calls = 0;
    std::string last_theme_name;
    std::string last_theme_path;
    bool last_apply_immediately = false;
    std::string last_directory;
    std::string last_save_theme_name;
    std::string last_save_path;
    float last_style_scale = 0.0f;

private:
    std::unordered_map<std::string, std::unique_ptr<Salix::ITheme>> registry;
    Salix::ITheme* active_theme = nullptr;
};
