// Salix/reflection/EnumRegistry.cpp
#include "EnumRegistry.h"
#include <Salix/ecs/Camera.h>

namespace Salix {
    std::unordered_map<std::type_index, EnumRegistry::EnumData> EnumRegistry::enum_data_registry;

    void register_all_enums() {
        EnumRegistry::register_enum(typeid(Camera::ProjectionMode), {
            {
                {static_cast<int>(Camera::ProjectionMode::Perspective), "Perspective"},
                {static_cast<int>(Camera::ProjectionMode::Orthographic), "Orthographic"}
            },
            { "Perspective", "Orthographic" }
        });
    }
}