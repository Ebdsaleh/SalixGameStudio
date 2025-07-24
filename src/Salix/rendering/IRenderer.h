// Salix/rendering/IRenderer.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/core/InitEnums.h>
#include <Salix/math/Color.h>  // Need the Color definition to allow tinting while rendering sprites.
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>
#include <Salix/rendering/ITexture.h>  // Need this to use our own renderer agnostic Texture.
#include <Salix/window/IWindow.h>
namespace Salix {

    // A renderer agnostic enum for flipping a sprite.
    enum class SpriteFlip {
        None,
        Horizontal,
        Vertical,
        Both
    };

  

    // This is an abstract base class that defines the "contract" for any renderer.

    class SALIX_API IRenderer {
        public:
        // A virtual desctructor is essential for any class intended for polymorphism.
        virtual ~IRenderer() = default;

        // The renderer creates its own window from a config.
        virtual bool initialize(const WindowConfig& config) = 0;
        virtual void shutdown() = 0;
        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;
        virtual void clear_depth_buffer() = 0;

        // Provide access to the window it owns, without giving up ownership.
        virtual IWindow* get_window() = 0;
        virtual void* get_native_handle() = 0;
        // A contract that all renderers must know how to load a texture.
        virtual ITexture* load_texture(const char* file_path) = 0;

        // This is essential to prevent drawing artifacts from previous frames.
        virtual void clear() = 0; 

        // A contract for drawing a texture at a specific location.
        virtual void draw_texture(ITexture* texture, const Rect& dest_rect) = 0;

        // A contract for drawing a Sprite2D
        virtual void draw_sprite(ITexture* texture, const Rect& dest_rect, double angle, const Point* pivot, const Color& color, SpriteFlip flip) = 0;

        virtual void on_window_resize(int width, int height) = 0;
    };
} // namespace Salix