// Salix/reflection/EnumRegistry.cpp
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/reflection/EditorDataMode.h>
#include <Salix/rendering/ICamera.h>
#include <Salix/ecs/Camera.h>

namespace Salix {
    std::unordered_map<std::type_index, EnumRegistry::EnumData> EnumRegistry::enum_data_registry;

    void  EnumRegistry::register_all_enums() {
        // ProjectionMode
        EnumRegistry::EnumData projection_mode_data;

        // Populate value_to_string map
        projection_mode_data.value_to_string[static_cast<int>(Salix::ProjectionMode::Perspective)] = "Perspective";
        projection_mode_data.value_to_string[static_cast<int>(Salix::ProjectionMode::Orthographic)] = "Orthographic";

        // Populate ordered_names vector (ensure order matches combo box needs)
        projection_mode_data.ordered_names.push_back("Perspective");
        projection_mode_data.ordered_names.push_back("Orthographic");

        // Register the enum data
        EnumRegistry::register_enum(typeid(Salix::ProjectionMode), std::move(projection_mode_data));



        // --- Register EditorDataMode enum data ---
        EnumRegistry::EnumData editor_mode_data;
        editor_mode_data.value_to_string[static_cast<int>(Salix::EditorDataMode::Live)] = "Live";
        editor_mode_data.value_to_string[static_cast<int>(Salix::EditorDataMode::Yaml)] = "Yaml";
        editor_mode_data.ordered_names.push_back("Live");
        editor_mode_data.ordered_names.push_back("Yaml");
        EnumRegistry::register_enum(typeid(Salix::EditorDataMode), std::move(editor_mode_data));
    }
}
