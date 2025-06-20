// =================================================================================
// Filename:    Salix/window/SDLWindow.h
// Author:      Nadir Saleh
// Created:     2025-06-20
// Description: Declares the SDLWindow class, a concrete implementation of IWindow
//              using the SDL library.
// =================================================================================
#pragma once

#include "IWindow.h" // The interface we are implementing
#include <SDL.h>     // The SDL library we are wrapping

namespace Salix {

    // PascalCase for the class name
    class SDLWindow : public IWindow {
    public:
        SDLWindow();
        virtual ~SDLWindow();

        // Implement the IWindow interface using the 'override' keyword
        bool initialize(const WindowConfig& config) override;
        void shutdown() override;
        void* get_native_handle() const override;

        int get_width() const override;
        int get_height() const override;

    private:
        // The actual, underlying SDL_Window object.
        // We use :: to specify the global SDL type and snake_case for the name.
        ::SDL_Window* window;

        // Store the config for easy access to width/height.
        WindowConfig window_config;
    };

} // namespace Salix