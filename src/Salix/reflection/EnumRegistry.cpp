// Salix/reflection/EnumRegistry.cpp
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/rendering/ICamera.h>
#include <Salix/ecs/Camera.h>

namespace Salix {
    std::unordered_map<std::type_index, EnumRegistry::EnumData> EnumRegistry::enum_data_registry;

    void  EnumRegistry::register_all_enums() {
        EnumRegistry::EnumData projection_mode_data;

        // Populate value_to_string map
        projection_mode_data.value_to_string[static_cast<int>(Salix::ProjectionMode::Perspective)] = "Perspective";
        projection_mode_data.value_to_string[static_cast<int>(Salix::ProjectionMode::Orthographic)] = "Orthographic";

        // Populate ordered_names vector (ensure order matches combo box needs)
        projection_mode_data.ordered_names.push_back("Perspective");
        projection_mode_data.ordered_names.push_back("Orthographic");

        // Register the enum data
        EnumRegistry::register_enum(typeid(Salix::ProjectionMode), std::move(projection_mode_data));
    }
}
