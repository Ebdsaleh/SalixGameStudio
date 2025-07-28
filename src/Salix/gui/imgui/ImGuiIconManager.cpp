// Salix/gui/imgui/ImGuiIconManager.cpp
#include <Salix/gui/imgui/ImGuiIconManager.h>
#include <memory>
#include <Salix/gui/IconInfo.h>
#include <Salix/assets/AssetManager.h>
#include <Salix/rendering/ITexture.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Camera.h>   
#include <map>
#include <string>
#include <iostream>

namespace Salix {

    struct ImGuiIconManager::Pimpl {
        AssetManager* asset_manager = nullptr;
        std::map<std::string, IconInfo> icon_registry;
        IconInfo default_icon;

        void register_icon(const std::string& type_name, const std::string& path);
    };


    ImGuiIconManager::ImGuiIconManager() : pimpl(std::make_unique<Pimpl>()) {}

    ImGuiIconManager::~ImGuiIconManager() = default;



    void ImGuiIconManager::initialize(AssetManager* asset_manager) {
        if (!asset_manager) {
            std::cerr << "ImGuiIconManager::intiailize - Failed to initialize, 'asset_manager' is nullptr!"  <<
            std::endl;
            return;
        }
        pimpl->asset_manager = asset_manager;
    }


    void ImGuiIconManager::Pimpl::register_icon(const std::string& type_name, const std::string& path) {
         ITexture* texture = asset_manager->get_texture(path);
        if (texture) {
            IconInfo info;
            info.texture_id = texture->get_imgui_texture_id();
            info.path = path;
            icon_registry[type_name] = info;
        } else {
            std::cerr << "ImGuiIconManager Warning: Could not load icon for type '" << type_name << "' at path: " << path << std::endl;
        }
    }
    void ImGuiIconManager::register_default_icons() {
        if (!pimpl->asset_manager) {
            std::cerr << "ImGuiIconManager Error: Cannot register default icons, AssetManager is not initialized." << std::endl;
            return;
        }
        pimpl->register_icon("Entity", "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_096.png");
        pimpl->register_icon("Camera", "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_039.png");
        pimpl->register_icon("Transform", "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_092.png");
        pimpl->register_icon("Sprite2D", "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_031.png");
        pimpl->register_icon("Panel Locked","Assets/Icons/Editor/Lucid V1.2/PNG/Shadow/16/Lock-Closed.png");
        pimpl->register_icon("Panel Unlocked","Assets/Icons/Editor/Lucid V1.2/PNG/Shadow/16/Lock-Open.png");
        // Also register the default icon itself
        pimpl->register_icon("Default", "Assets/Icons/Editor/Kenney/Generic/PNG/Colored/genericItem_color_153.png");
        pimpl->default_icon = pimpl->icon_registry["Default"];
    }



    const IconInfo& ImGuiIconManager::get_icon_for_entity(Entity* entity) {
        if (!entity) {
            return pimpl->default_icon;
        }

        // Highest priority: If the entity has a Sprite2D, we want to show its specific texture.
        if (entity->has_element<Sprite2D>()) {
            Sprite2D* sprite = entity->get_element<Sprite2D>();
            ITexture* texture = sprite->get_texture();
            if (texture) {
                // This is a dynamic, user-specific icon. We create a temporary IconInfo for it.
                // A more advanced system would cache this result.
                static IconInfo dynamic_icon_info; // Static to avoid re-allocation
                dynamic_icon_info.texture_id = texture->get_imgui_texture_id();
                return dynamic_icon_info;
            }
        }

        // If no sprite, check for other special components like Camera.
        if (entity->has_element<Camera>()) {
            auto it = pimpl->icon_registry.find("Camera");
            if (it != pimpl->icon_registry.end()) {
                return it->second;
            }
        }

        // If it's just a generic entity with no special components, use the default entity icon.
        auto it = pimpl->icon_registry.find("Entity");
        if (it != pimpl->icon_registry.end()) {
            return it->second;
        }

         // If all else fails, return the absolute default icon.
        return pimpl->default_icon;

    }



    const IconInfo& ImGuiIconManager::get_icon_for_element(Element* element) {
        if (!element) {
                return pimpl->default_icon;
        }

            // For elements, we just look up their class name in the registry.
        const std::string& type_name = element->get_class_name();
        auto it = pimpl->icon_registry.find(type_name);

        if (it != pimpl->icon_registry.end()) {
            // We found a specific icon registered for this component type.
            return it->second;
        }

        // If no specific icon is found, return the default.
        return pimpl->default_icon;
    }

    void ImGuiIconManager::update_icon(const std::string& type_name, const std::string& new_path) {
        if (pimpl->icon_registry.count(type_name)) {
            ITexture* texture = pimpl->asset_manager->get_texture(new_path);
            if (texture) {
                pimpl->icon_registry[type_name].texture_id = texture->get_imgui_texture_id();
                pimpl->icon_registry[type_name].path = new_path;
            } else {
                std::cerr << "ImGuiIconManager::update_icon - Failed to update icon, texture is nullptr!" <<
                std::endl;
                return;
            }
        } else {
            std::cerr << "ImGuiIconManager::update_icon - Failed to update icon, type_name: '" << type_name << 
            "' does not exist in icon_registry! " << std::endl;
        }
    }


    const std::map<std::string, IconInfo>& ImGuiIconManager::get_icon_registry()  {
        return pimpl->icon_registry;
    }

    const IconInfo& ImGuiIconManager::get_icon_by_name(const std::string& name) const {
        auto it = pimpl->icon_registry.find(name);
        if (it != pimpl->icon_registry.end()) {
            return it->second;
        }
        // If the icon is not found, return the default icon to avoid crashes
        std::cerr << "ImGuiIconManager Error: Icon '" << name << "' not found in registry. Returning default icon.\n";
        return pimpl->default_icon;
    }
} // namespace Salix