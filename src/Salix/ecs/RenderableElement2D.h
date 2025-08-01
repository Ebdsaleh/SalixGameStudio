// Salix/ecs/RenderableElement2D.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/ecs/RenderableElement.h> 
#include <cereal/cereal.hpp>             

namespace Salix {

    // RenderableElement2D serves as a specific base for all 2D components.
    // It remains an abstract class because it doesn't implement the pure virtual functions
    // from its parent, RenderableElement.
    class SALIX_API RenderableElement2D : public RenderableElement {
    public:
        // We don't need to re-declare the pure virtual functions (`render` and `on_load`).
        // They are automatically inherited from RenderableElement. Any class that inherits
        // from this class (like Sprite2D) will still be required to implement them.

        // --- Serialization ---
        // This is necessary to ensure that when a child class (like Sprite2D) is serialized,
        // the data from this part of the inheritance chain is also saved.
        template <class Archive>
        void serialize(Archive& archive) {
            // This line tells Cereal to first serialize our direct parent class, RenderableElement.
            // This continues the chain: Sprite2D -> RenderableElement2D -> RenderableElement -> Element
            archive( cereal::base_class<RenderableElement>(this) );
        }
    };

} // namespace Salix