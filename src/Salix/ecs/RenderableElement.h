// Salix/ecs/RenderableElement.h
#pragma once

#include <Element.h>  // Required to inherit from the Element base class.
namespace Salix {
    // Forward declare the renderer interface
    class IRenderer;

    // This is a specialized type of Element for anything that needs to be drawn
    class RenderableElement : public Element {
    public:
        // The render method is now a "pure virtual" function (= 0)
        // This means any class that  inherits from RenderableElement MUST provide its own
        // implementation of the render method.
        virtual void render(IRenderer* renderer) = 0;

    };
} // namespace Salix