// =================================================================================
// Filename:    Salix/window/SDLWindow.cpp
// Author:      SalixGameStudio
// Created:     2025-06-20
// Description: Implements the SDLWindow class, a concrete implementation of IWindow
//              using the SDL library.
// =================================================================================
#include "SDLWindow.h"
#include <iostream> // For logging success or errors

namespace Salix {

    SDLWindow::SDLWindow()
        : m_window(nullptr) {
        // Constructor: Initialize the window pointer to null.
    }

    SDLWindow::~SDLWindow() {
        // Destructor: Ensure shutdown is called if the window still exists,
        // although explicit shutdown is preferred.
        if (m_window) {
            shutdown();
        }
    }

    bool SDLWindow::initialize(const WindowConfig& config) {
        // Store the configuration.
        m_config = config;

        // Create the actual SDL window using the configuration.
        m_window = SDL_CreateWindow(
            m_config.title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            m_config.width,
            m_config.height,
            SDL_WINDOW_SHOWN
        );

        // Check if the window was created successfully.
        if (m_window == nullptr) {
            std::cout << "SDLWindow::initialize - Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        std::cout << "Window created successfully." << std::endl;
        return true;
    }

    void SDLWindow::shutdown() {
        if (m_window) {
            // Destroy the SDL window.
            SDL_DestroyWindow(m_window);
            // Set the pointer to null to prevent dangling pointers.
            m_window = nullptr;
        }
    }

    void* SDLWindow::get_native_handle() const {
        // Return the opaque pointer to the underlying SDL window.
        return m_window;
    }

    int SDLWindow::get_width() const {
        return m_config.width;
    }

    int SDLWindow::get_height() const {
        return m_config.height;
    }

} // namespace Salix