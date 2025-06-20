// Salix/rendering/IRenderer.h
#pragma once

#include <Salix/math/Color.h>  // Need the Color definition to allow tinting while rendering sprites.
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>
#include <Salix/rendering/ITexture.h>  // Need this to use our own renderer agnostic Texture.
namespace Salix {

    // We forward declare the types we need.
    class IWindow;
    struct WindowConfig;


    // A renderer agnostic enum for flipping a sprite.
    enum class SpriteFlip {
        None,
        Horizontal,
        Vertical,
        Both
    };

    // An enum for declaring the type of renderer in use.
    enum class RendererType {
        SDL, // The default option
        OpenGl, // OpenGl using the SDL API
        Vulkan,  // For the future
        DirectX, // For the future
    };

    // Used to pass the window creation to Renderer.
    struct WindowConfig {
        const char* title = "Salix Game Studio";
        int width = 1280;
        int height = 720;
        RendererType renderer_type = RendererType::SDL; // default renderer_type config value.
    };

    // This is an abstract base class that defines the "contract" for any renderer.

    class IRenderer {
        public:
        // A virtual desctructor is essential for any class intended for polymorphism.
        virtual ~IRenderer() = default;

        // These are "pure virtual" functions, denoted by the '= 0'.
        // Any class that inherits from IRenderer MUST provide its own implementation.
        virtual bool initialize(const WindowConfig& config) = 0;
        virtual void shutdown() = 0;
        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;

        // Provide access to the window it owns, without giving up ownership.
        virtual IWindow* get_window() = 0;

        // A contract that all renderers must know how to load a texture.
        virtual ITexture* load_texture(const char* file_path) = 0;

        // A contract for drawing a texture at a specific location.
        virtual void draw_texture(ITexture* texture, const Rect& dest_rect) = 0;

        // A contract for drawing a Sprite2D
        virtual void draw_sprite(ITexture* texture, const Rect& dest_rect, double angle, const Point* pivot, const Color& color, SpriteFlip flip) = 0;

    };
} // namespace Salix