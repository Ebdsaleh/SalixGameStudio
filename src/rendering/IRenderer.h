// IRenderer.h
#pragma once

#include "../math/Color.h"  // Need the Color definition to allow tinting while rendering sprites.
// We forward declare SDL_Window because the interface needs to know what kind of 
// window to initialize itself with.
struct SDL_Window;

// Forward declare SDL_Texture for texture loading, and SDL_Rect for Drawing the texture.
struct SDL_Texture;
struct SDL_Rect;

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
    virtual SDL_Window* initialize(const WindowConfig& config) = 0;
    virtual void shutdown() = 0;
    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;

    // A contract that all renderers must know how to load a texture.
    virtual SDL_Texture* load_texture(const char* file_path) = 0;

    // A contract for drawing a texture at a specific location.
    virtual void draw_texture(SDL_Texture* texture, const SDL_Rect& dest_rect) = 0;

    // A contract for drawing a Sprite2D
    virtual void draw_sprite(SDL_Texture* texture, const SDL_Rect& dest_rect, double angle, const Color& color, SpriteFlip flip) = 0;

};