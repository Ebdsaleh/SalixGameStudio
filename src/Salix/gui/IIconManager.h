// Salix/gui/IIconManager.h
#pragma once

#include <Salix/core/Core.h>
#include <string>
#include <map>

// Forward-declare the types we need
namespace Salix {
    class AssetManager;
    class Entity;
    class Element;
    struct IconInfo;
}

namespace Salix {

    class SALIX_API IIconManager {
    public:
        virtual ~IIconManager() = default;

        // Initializes the manager and gives it access to the AssetManager.
        virtual void initialize(AssetManager* asset_manager) = 0;

        // Loads the default set of icons for built-in engine types.
        virtual void register_default_icons() = 0;

        // The main workhorse methods for the UI panels.
        virtual const IconInfo& get_icon_for_entity(Entity* entity) = 0;
        virtual const IconInfo& get_icon_for_element(Element* element) = 0;
        virtual const IconInfo& get_icon_by_name(const std::string& name) const = 0;
        virtual void update_icon(const std::string& type_name, const std::string& new_path) = 0;
        virtual const std::map<std::string, IconInfo>& get_icon_registry()  = 0;
    };

} // namespace Salix