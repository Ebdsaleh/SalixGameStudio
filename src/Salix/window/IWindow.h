// =================================================================================
// Filename:    Salix/window/IWindow.h
// Author:      Nadir Saleh
// Created:     2025-06-20
// Description: Declares the IWindow interface, an abstract representation
//              of a platform-specific window.
// =================================================================================
#pragma once
#include <Salix/core/Core.h>
#include <Salix/window/WindowConfig.h>
#include <string>

namespace Salix {

    

    class SALIX_API IWindow {
    public:
        virtual ~IWindow() = default;

        // Creates and shows the window based on the provided config.
        virtual bool initialize(const WindowConfig& window_config) = 0;

        // Destroys the window and cleans up resources.
        virtual void shutdown() = 0;

        // --- The key to connecting to the renderer ---
        // Returns the underlying, platform-specific window handle (e.g., SDL_Window*).
        // The renderer will know how to cast and use this.
        virtual void* get_native_handle() const = 0;
        // ---------------------------------------------
        virtual void query_dimensions(int& out_width, int& out_height) const = 0;
        virtual int get_width() const = 0;
        virtual int get_height() const = 0;
        virtual void set_size(int width, int height) = 0;
        virtual float get_dpi_scale() const = 0;
    };

} // namespace Salix