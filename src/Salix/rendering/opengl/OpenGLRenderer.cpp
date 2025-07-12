// =================================================================================
// Filename:    Salix/rendering/OpenGLRenderer.cpp
// Author:      SalixGameStudio
// Description: Implementation of the OpenGLRenderer class.
// =================================================================================
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <Salix/window/sdl/SDLWindow.h> // We still use SDL for the window
#include <Salix/window/WindowConfig.h>
#include <Salix/window/IWindow.h>
#include <memory>
#include <SDL.h>
#include <iostream>

// We will add GLAD here later
// #include <glad/glad.h>

namespace Salix {

    struct OpenGLRenderer::Pimpl {

        std::unique_ptr<IWindow> window;
        SDL_GLContext gl_context = nullptr; // The OpenGL context handle
    };


    OpenGLRenderer::OpenGLRenderer() : pimpl(std::make_unique<Pimpl>())
    {
    }

    OpenGLRenderer::~OpenGLRenderer()
    {
        // Ensure shutdown is called, as a safety net
        shutdown();
    }

    bool OpenGLRenderer::initialize(const WindowConfig& config)
    {
        // --- Step 1: Set OpenGL Attributes BEFORE creating the window ---
        // We want OpenGL 3.3
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        // We want to use the "Core" profile, which gives us modern OpenGL
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        // We want a double-buffered window for smooth rendering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // 24-bit depth buffer

        // Now, create the window. It will be created with these OpenGL attributes.
        pimpl->window = std::make_unique<SDLWindow>();
        if (!pimpl->window->initialize(config)) {
            std::cerr << "OpenGLRenderer Error: Failed to initialize window." << std::endl;
            return false;
        }

        // --- Step 2: Create the OpenGL Context ---
        pimpl->gl_context = SDL_GL_CreateContext(static_cast<SDL_Window*>(pimpl->window->get_native_handle()));
        if (pimpl->gl_context == nullptr) {
            std::cerr << "OpenGLRenderer Error: Failed to create OpenGL context! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        std::cout << "OpenGL context created successfully." << std::endl;

        // --- Step 3 (Future): Initialize GLAD ---
        // We'll add this code in the next step
        // if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        //     std::cerr << "OpenGLRenderer Error: Failed to initialize GLAD!" << std::endl;
        //     return false;
        // }
        
        std::cout << "OpenGLRenderer initialized successfully." << std::endl;
        return true;
    }

    void OpenGLRenderer::shutdown()
    {
        if (pimpl->gl_context) {
            SDL_GL_DeleteContext(pimpl->gl_context);
            pimpl->gl_context = nullptr;
        }
        if (pimpl->window) {
            pimpl->window->shutdown();
            pimpl->window.reset();
        }
    }

    void OpenGLRenderer::begin_frame()
    {
        // For now, we'll just clear the screen to a test color
        // In the future, this is where you'd bind framebuffers, etc.
        clear();
    }

    void OpenGLRenderer::end_frame()
    {
        // Swaps the front and back buffers to display what we've rendered
        SDL_GL_SwapWindow(static_cast<SDL_Window*>(pimpl->window->get_native_handle()));
    }

    void OpenGLRenderer::clear()
    {
        // TODO: Use OpenGL functions here once GLAD is initialized
        // glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    IWindow* OpenGLRenderer::get_window()
    {
        return pimpl->window.get();
    }

    void* OpenGLRenderer::get_native_handle()
    {
        // The "native handle" for this renderer is its GL context
        return pimpl->gl_context;
    }

} // namespace Salix
