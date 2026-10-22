#pragma once

#include <Salix/gui/IFontManager.h>
#include <Salix/gui/IFont.h>
#include <Salix/gui/IGui.h>
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class MockIFontManager : public Salix::IFontManager {
public:
    bool initialize(Salix::IGui* gui_system) override {
        gui = gui_system;
        ++initialize_calls;
        return initialize_result;
    }

    void shutdown() override {
        ++shutdown_calls;
        active_font = nullptr;
        registry.clear();
    }

    bool register_font(std::unique_ptr<Salix::IFont> font) override {
        if (!font) return false;
        const std::string name = font->get_name();
        if (registry.find(name) != registry.end()) return false;
        registry.emplace(name, std::move(font));
        return true;
    }

    bool unregister_font(const std::string& font_name) override {
        auto it = registry.find(font_name);
        if (it == registry.end()) return false;
        if (active_font == it->second.get()) active_font = nullptr;
        registry.erase(it);
        return true;
    }

    bool apply_font(const std::string& font_name) override {
        ++apply_calls;
        return registry.find(font_name) != registry.end();
    }

    bool rebuild_font_atlas_texture() override {
        ++rebuild_calls;
        return rebuild_result;
    }

    std::vector<std::string> get_registered_font_names() const override {
        std::vector<std::string> names;
        for (const auto& [name, _] : registry) names.push_back(name);
        std::sort(names.begin(), names.end());
        return names;
    }

    const std::unordered_map<std::string, std::unique_ptr<Salix::IFont>>& get_font_registry() override {
        return registry;
    }

    std::vector<float> get_font_sizes_for_font_family(const std::string& family) const override {
        std::vector<float> sizes;
        for (const auto& [_, font] : registry) {
            if (font && font->get_family() == family && font->get_data()) {
                sizes.push_back(font->get_data()->get_font_size());
            }
        }
        std::sort(sizes.begin(), sizes.end());
        return sizes;
    }

    std::vector<std::string> get_unique_font_families() const override {
        std::vector<std::string> families;
        for (const auto& [_, font] : registry) {
            if (!font) continue;
            if (std::find(families.begin(), families.end(), font->get_family()) == families.end())
                families.push_back(font->get_family());
        }
        std::sort(families.begin(), families.end());
        return families;
    }

    const Salix::IFont* get_font(const std::string& font_name) const override {
        auto it = registry.find(font_name);
        return it == registry.end() ? nullptr : it->second.get();
    }

    void create_font_batch(const std::string& font_path, const std::string& font_family, float min_size, float max_size) override {
        ++create_batch_calls;
        last_batch_path = font_path;
        last_batch_family = font_family;
        last_batch_min = min_size;
        last_batch_max = max_size;
    }

    void setup_default_fonts() override { ++setup_default_calls; }

    bool load_font_from_file(const std::string& file_path, const std::string& font_name, float font_size, bool apply_immediately = false) override {
        ++load_file_calls;
        last_file_path = file_path;
        last_file_name = font_name;
        last_file_size = font_size;
        last_apply_immediately = apply_immediately;
        return load_file_result;
    }

    bool load_fonts_from_directory(const std::string& directory_path) override {
        ++load_directory_calls;
        last_directory = directory_path;
        return load_directory_result;
    }

    bool set_active_font(const std::string& font_name) override {
        auto it = registry.find(font_name);
        if (it == registry.end()) return false;
        active_font = it->second.get();
        return true;
    }

    bool purge_font(const std::string& font_name) override { return unregister_font(font_name); }
    Salix::IFont* get_active_font() override { return active_font; }

    Salix::IGui* gui = nullptr;
    bool initialize_result = true;
    bool rebuild_result = true;
    bool load_file_result = true;
    bool load_directory_result = true;
    int initialize_calls = 0;
    int shutdown_calls = 0;
    int apply_calls = 0;
    int rebuild_calls = 0;
    int create_batch_calls = 0;
    int setup_default_calls = 0;
    int load_file_calls = 0;
    int load_directory_calls = 0;
    std::string last_batch_path;
    std::string last_batch_family;
    float last_batch_min = 0.0f;
    float last_batch_max = 0.0f;
    std::string last_file_path;
    std::string last_file_name;
    float last_file_size = 0.0f;
    bool last_apply_immediately = false;
    std::string last_directory;

private:
    std::unordered_map<std::string, std::unique_ptr<Salix::IFont>> registry;
    Salix::IFont* active_font = nullptr;
};
