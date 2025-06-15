// IRenderer.h
#pragma once

// We forward declare SDL_Window because the interface needs to know what kind of 
// window to initialize itself with.

struct SDL_Window;

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

};