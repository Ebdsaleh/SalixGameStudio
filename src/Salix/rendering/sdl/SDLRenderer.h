// Salix/rendering/sdl/SDLRenderer.h
#pragma once
#include <Salix/rendering/IRenderer.h> // We need to include the interface we are implementing.
#include <SDL.h>
#include <memory>
// SDLRenderer is concrete implementation of the IRenderer interface.

namespace Salix{

    class SDLRenderer : public IRenderer {
        public:
        SDLRenderer();
        virtual ~SDLRenderer();  // Good practice to mark overridan destructers as virtual.

        // We use the 'override' keyword to make clear we are implmenting a method from the base class.
        // The compiler will error if get the function signature incorrect, this is a great safety feature.


        // The Renderer's lifecycle methods
        bool initialize(const WindowConfig& config) override;
        void shutdown() override;

        // The core rendering commands
        void begin_frame() override;
        void end_frame() override;
        void clear() override;
        void clear_depth_buffer() override { return; }
        // Delcare Texture loading.
        ITexture* load_texture(const char* file_path) override;
        // Declare Texture Drawing
        void draw_texture(ITexture* texture, const Rect& dest_rect) override;
        // Declare Sprite2D drawing
        void draw_sprite(ITexture* texture, const Rect& dest_rect, double angle, const Point* pivot, const Color& color, SpriteFlip flip) override;
        // We now have a method to access the IWindow-inherited object.
        IWindow* get_window() override;
        void* get_native_handle() override;
        SDL_Renderer* get_sdl_renderer();
        private:
        // The Renderer's own SDL_Renderer object
        ::SDL_Renderer* sdl_renderer;
        std::unique_ptr<IWindow> window;  // The renderer now has full, exclusive ownership of an IWindow object.
    };
} // namespace Salix