// Salix/ecs/RenderableElement.h
#pragma once

#include <Salix/core/Core.h>
#include <Salix/ecs/Element.h>  // Required to inherit from the Element base class.
#include <cereal/cereal.hpp>
namespace Salix {
    // Forward declare the renderer interface
    class IRenderer;
    class AssetManager;
    // This is a specialized type of Element for anything that needs to be drawn
    class SALIX_API RenderableElement : public Element {
    public:
        // The render method is now a "pure virtual" function (= 0)
        // This means any class that  inherits from RenderableElement MUST provide its own
        // implementation of the render method.
        virtual void render(IRenderer* renderer) = 0;
        virtual void on_load(AssetManager* asset_manager) override = 0;
        template <class Archive>
        void serialize(Archive& archive) {
            // This is the crucial part. It tells Cereal to first serialize
            // our parent class, Element. This continues the chain.
            archive( cereal::base_class<Element>(this) );
        }
    };
} // namespace Salix