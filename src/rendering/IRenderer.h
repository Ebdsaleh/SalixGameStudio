// IRenderer.h
#pragma once

// We forward declare SDL_Window because the interface needs to know what kind of 
// window to initialize itself with.

struct SDL_Window;

// This is an abstract base class that defines the "contract" for any renderer.

class IRenderer {
    public:
    // A virtual desctructor is essential for any class intended for polymorphism.
    virtual ~IRenderer() = default;

    // These are "pure virtual" functions, denoted by the '= 0'.
    // Any class that inherits from IRenderer MUST provide its own implementation.
    virtual bool initialize(SDL_Window* window) = 0;
    virtual void shutdown() = 0;
    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;

};