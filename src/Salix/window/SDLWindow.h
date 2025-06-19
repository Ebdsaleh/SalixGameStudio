// =================================================================================
// Filename:    Salix/window/SDLWindow.h
// Author:      SalixGameStudio
// Created:     2025-06-20
// Description: Declares the SDLWindow class, a concrete implementation of IWindow
//              using the SDL library.
// =================================================================================
#pragma once

#include "IWindow.h" // Our interface
#include <SDL.h>     // The SDL library we are wrapping

namespace Salix {

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
        // We use :: to specify the global SDL type.
        ::SDL_Window* m_window;

        // Store the config for easy access to width/height.
        WindowConfig m_config;
    };

} // namespace Salix