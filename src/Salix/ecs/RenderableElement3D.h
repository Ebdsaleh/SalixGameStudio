// Salix/ecs/RenderableElement3D.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/ecs/RenderableElement.h> 
#include <cereal/cereal.hpp>            

namespace Salix {

    // RenderableElement3D serves as a specific base for all 3D components.
    // It remains an abstract class, forcing its children (like a future MeshRenderer)
    // to provide their own implementations for the render() and on_load() methods.
    class SALIX_API RenderableElement3D : public RenderableElement {
    public:
        // --- Serialization ---
        // This ensures that when a child class (like a future MeshRenderer) is serialized,
        // the data from this part of the inheritance chain is also saved correctly.
        template <class Archive>
        void serialize(Archive& archive) {
            // This line tells Cereal to first serialize our direct parent class, RenderableElement.
            archive( cereal::base_class<RenderableElement>(this) );
        }
    };

} // namespace Salix