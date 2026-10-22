#pragma once

#include <Salix/gui/IIconManager.h>
#include <Salix/gui/IconInfo.h>
#include <string>
#include <map>

class MockIIconManager : public Salix::IIconManager {
public:
    void initialize(Salix::AssetManager* asset_manager) override {
        asset_manager_ = asset_manager;
        ++initialize_calls;
    }

    void register_default_icons() override {
        ++register_default_calls;
        if (registry_.find("Default") == registry_.end()) {
            Salix::IconInfo info;
            info.path = "mock/default.png";
            registry_.emplace("Default", info);
        }
    }

    const Salix::IconInfo& get_icon_for_entity(Salix::Entity*) override {
        ++entity_lookup_calls;
        return get_or_default("Entity");
    }

    const Salix::IconInfo& get_icon_for_element(Salix::Element*) override {
        ++element_lookup_calls;
        return get_or_default("Element");
    }

    const Salix::IconInfo& get_icon_by_name(const std::string& name) const override {
        auto it = registry_.find(name);
        if (it != registry_.end()) return it->second;
        auto fallback = registry_.find("Default");
        if (fallback != registry_.end()) return fallback->second;
        return empty_icon_;
    }

    void update_icon(const std::string& type_name, const std::string& new_path) override {
        registry_[type_name].path = new_path;
        ++update_calls;
        last_update_name = type_name;
        last_update_path = new_path;
    }

    const std::map<std::string, Salix::IconInfo>& get_icon_registry() override {
        return registry_;
    }

    Salix::AssetManager* get_mock_asset_manager() const { return asset_manager_; }

    int initialize_calls = 0;
    int register_default_calls = 0;
    int entity_lookup_calls = 0;
    int element_lookup_calls = 0;
    int update_calls = 0;
    std::string last_update_name;
    std::string last_update_path;

private:
    const Salix::IconInfo& get_or_default(const std::string& name) {
        auto it = registry_.find(name);
        if (it != registry_.end()) return it->second;
        return get_icon_by_name("Default");
    }

    Salix::AssetManager* asset_manager_ = nullptr;
    std::map<std::string, Salix::IconInfo> registry_;
    Salix::IconInfo empty_icon_{};
};
