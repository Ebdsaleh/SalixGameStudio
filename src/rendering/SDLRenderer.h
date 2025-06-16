// Renderer.h
#pragma once
#include "IRenderer.h" // We need to include the interface we are implementing.
#include <SDL.h>
// SDLRenderer is concrete implementation of the IRenderer interface.

class SDLRenderer : public IRenderer {
    public:
    SDLRenderer();
    virtual ~SDLRenderer();  // Good practice to mark overridan destructers as virtual.

    // We use the 'override' keyword to make clear we are implmenting a method from the base class.
    // The compiler will error if get the function signature incorrect, this is a great safety feature.


    // The Renderer's lifecycle methods
    SDL_Window* initialize(const WindowConfig& config) override;
    void shutdown() override;

    // The core rendering commands
    void begin_frame() override;
    void end_frame() override;
    // Delcare Texture loading.
    SDL_Texture* load_texture(const char* file_path) override;
    // Declare Texture Drawing
    void draw_texture(SDL_Texture* texture, const SDL_Rect& dest_rect) override;
    // Declare Sprite2D drawing
    void draw_sprite(SDL_Texture* texture, const SDL_Rect& dest_rect, double angle, const struct SDL_Point* pivot, const Color& color, SpriteFlip flip) override;
    private:
    // The Renderer's own SDL_Renderer object
    SDL_Renderer* sdl_renderer;
    SDL_Window* window;
};
