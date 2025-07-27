// Salix/reflection/EnumRegistry.cpp
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/rendering/ICamera.h>
#include <Salix/ecs/Camera.h>

namespace Salix {
    std::unordered_map<std::type_index, EnumRegistry::EnumData> EnumRegistry::enum_data_registry;

    void register_all_enums() {
        EnumRegistry::register_enum(typeid(Salix::ProjectionMode), {
            {
                {static_cast<int>(Salix::ProjectionMode::Perspective), "Perspective"},
                {static_cast<int>(Salix::ProjectionMode::Orthographic), "Orthographic"}
            },
            { "Perspective", "Orthographic" }
        });
    }
}