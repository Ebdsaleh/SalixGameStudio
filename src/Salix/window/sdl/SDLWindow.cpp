// Filename:    Salix/window/sdl/SDLWindow.cpp
// Author:      Nadir Saleh
// Created:     2025-06-20
// Description: Implements the SDLWindow class, a concrete implementation of IWindow
//              using the SDL library.
// =================================================================================
#include <Salix/window/sdl/SDLWindow.h>
#include <iostream> // For logging success or errors

namespace Salix {

    SDLWindow::SDLWindow()
        : window(nullptr) {
        // Constructor: Initialize the window pointer to null.
    }

    SDLWindow::~SDLWindow() {
        // Destructor: Ensure shutdown is called if the window still exists.
        // This is a safety net; explicit shutdown by the user is the correct pattern.
        if (window) {
            shutdown();
        }
    }

    bool SDLWindow::initialize(const WindowConfig& config) {
        // Store the configuration.
        window_config = config;

        // Create the actual SDL window using the configuration.
        window = SDL_CreateWindow(
            window_config.title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_config.width,
            window_config.height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
        );

        // Check if the window was created successfully.
        if (window == nullptr) {
            std::cout << "SDLWindow::initialize - Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        std::cout << "Window created successfully." << std::endl;
        return true;
    }

    void SDLWindow::shutdown() {
        if (window) {
            // Destroy the SDL window.
            SDL_DestroyWindow(window);
            // Set the pointer to null to prevent dangling pointers and double deletion.
            window = nullptr;
        }
    }

    void* SDLWindow::get_native_handle() const {
        // Return the opaque pointer to the underlying SDL window.
        // This is the bridge that lets the renderer use this window.
        return window;
    }

    int SDLWindow::get_width() const {
        return window_config.width;
    }

    int SDLWindow::get_height() const {
        return window_config.height;
    }

    void SDLWindow::query_dimensions(int& out_width, int& out_height) const {
        if (window) {
            SDL_GetWindowSize(window, &out_width, &out_height);
        } else {
            // If the SDL window hasn't been created or has been destroyed,
            // return zero dimensions to indicate an invalid state.
            out_width = 0;
            out_height = 0;
        }

    }


    void SDLWindow::set_size(int width, int height) {
        SDL_SetWindowSize(window, width, height);
        
        // Also update our internal config state
        window_config.width = width;
        window_config.height = height;

    }

    float SDLWindow::get_dpi_scale() const {
        float dpi_scale = 1.0f;
        if (!window) {
            return 1.0f;
        }
        int logical_width, drawable_width;
        SDL_GetWindowSize(window, &logical_width, nullptr);
        SDL_GL_GetDrawableSize(window, &drawable_width, nullptr);
        if (logical_width > 0) {
            dpi_scale = static_cast<float>(drawable_width) / logical_width;
        }
        return dpi_scale;
    }

} // namespace Salix