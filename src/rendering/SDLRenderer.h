// Renderer.h
#pragma once
#include "IRenderer.h" // We need to include the interface we are implementing.

// SDLRenderer is concrete implementation of the IRenderer interface.

class SDLRenderer : public IRenderer {
    public:
    SDLRenderer();
    virtual ~SDLRenderer();  // Good practice to mark overridan destructers as virtual.

    // We use the 'override' keyword to make clear we are implmenting a method from the base class.
    // The compiler will error if get the function signature incorrect, this is a great safety feature.


    // The Renderer's lifecycle methods
    bool initialize(SDL_Window*) override;
    void shutdown() override;

    // The core rendering commands
    void begin_frame() override;
    void end_frame() override;

    private:
    // The Renderer's own SDL_Renderer object
    struct SDL_Renderer* sdl_renderer = nullptr;
};
