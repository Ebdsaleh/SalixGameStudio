// Filename:    Salix/window/SDLWindow.cpp
// Author:      Nadir Saleh
// Created:     2025-06-20
// Description: Implements the SDLWindow class, a concrete implementation of IWindow
//              using the SDL library.
// =================================================================================
#include "SDLWindow.h"
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
            window_config.title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_config.width,
            window_config.height,
            SDL_WINDOW_SHOWN
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

} // namespace Salix