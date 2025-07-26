// Salix/gui/imgui/ImGuiIconManager.h
#pragma once

#include <Salix/gui/IIconManager.h>
#include <memory> // For std::unique_ptr
#include <map>

namespace Salix {

    class SALIX_API ImGuiIconManager : public IIconManager {
    public:
        ImGuiIconManager();
        ~ImGuiIconManager() override;

        // --- IIconManager Interface ---
        void initialize(AssetManager* asset_manager) override;
        void register_default_icons() override;
        const IconInfo& get_icon_for_entity(Entity* entity) override;
        const IconInfo& get_icon_for_element(Element* element) override;
        void update_icon(const std::string& type_name, const std::string& new_path) override;
        const std::map<std::string, IconInfo>& get_icon_registry() override;
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix