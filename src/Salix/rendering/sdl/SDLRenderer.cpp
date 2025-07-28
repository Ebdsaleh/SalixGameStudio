// Salix/rendering/sdl/SDLRenderer.cpp
#include <Salix/rendering/sdl/SDLRenderer.h>
#include <Salix/rendering/sdl/SDLTexture.h>
#include <SDL_image.h>
#include <Salix/window/sdl/SDLWindow.h>
#include <imgui/imgui.h>
#include <iostream>
#include <SDL.h>


namespace Salix {

    struct SDLRenderer::Pimpl {
        // The Renderer's own SDL_Renderer object
        SDL_Renderer* sdl_renderer = nullptr;
        std::unique_ptr<IWindow> window = nullptr;  // The renderer now has full, exclusive ownership of an IWindow object.
        SDL_Window* sdl_window = nullptr;
        SDL_GLContext sdl_gl_context = nullptr;
        Color clear_color = Color::from_rgba_int(15, 20, 40, 255);
    };

    SDLRenderer::SDLRenderer() : pimpl(std::make_unique<Pimpl>()) {}

    SDLRenderer::~SDLRenderer() {
        // The unique_ptr will automatically be destroyed here, but we'll still call shutdown as a safeguard.
       shutdown();
    }

    bool SDLRenderer::initialize(const WindowConfig& config) {
        // 1. Create and take ownership of a new SDLWindow object.
        pimpl->window = std::make_unique<SDLWindow>();

        // 2. Initialize it. If this fails, we don't need to manually delete it.
        // The function will exit, and the unique_ptr will be cleared automatically.
        if (!pimpl->window->initialize(config)) {
            std:: cout << "SDLRenderer::initialize - Window initialization failed!" << std::endl;
            pimpl->window.reset();  // Explicitly clear the pointer.
            return false;
        }

        // 3. Get the native handle and create the actual SDL renderer.
        void* native_handle = pimpl->window->get_native_handle();
        pimpl->sdl_renderer = SDL_CreateRenderer(static_cast<SDL_Window*>(native_handle), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        if (pimpl->sdl_renderer == nullptr) {
            std::cout << "Renderer::initalize - SDL_CreateRenderer Failed!" << std::endl;
            pimpl->window.reset();  // Cleanup the window object.
            return false;
        }

        std::cout << "Renderer created successfully." << std::endl;
        return true;
    }

    IWindow* SDLRenderer::get_window() {
    // .get() returns the raw pointer so other parts of the engine can use it,
    // but the unique_ptr still retains ownership
    return pimpl->window.get();
    }

    void* SDLRenderer::get_native_handle(){
        return static_cast <void *> (pimpl->sdl_renderer);
    }

    
    SDL_Renderer* SDLRenderer::get_sdl_renderer() {
        return pimpl->sdl_renderer;
    }

    void SDLRenderer::shutdown() {
        std::cout << "Shutting down renderer." << std::endl;
        if (pimpl->sdl_renderer) {
            SDL_DestroyRenderer(pimpl->sdl_renderer);
        }

        pimpl->sdl_renderer = nullptr; // Prevent double-deletion  
        pimpl->window = nullptr;
    }

    void SDLRenderer::clear() {
        SDL_SetRenderDrawColor(pimpl->sdl_renderer, 0, 0, 0, 255);
        SDL_RenderClear(pimpl->sdl_renderer);
    }

    void SDLRenderer::begin_frame() {
        // Set the draw color and clear the screen
        SDL_SetRenderDrawColor(pimpl->sdl_renderer, 15, 20, 40, 255); // A dark-blue color.
        SDL_RenderClear(pimpl->sdl_renderer);
    }

    void SDLRenderer::end_frame() {
        // Present the back buffer to the screen.
        SDL_RenderPresent(pimpl->sdl_renderer);
    }

    // Texture loading implementation.
    ITexture* SDLRenderer::load_texture(const char* file_path) {
        SDL_Texture* texture = IMG_LoadTexture(pimpl->sdl_renderer, file_path);

        if (texture == nullptr) {
            std::cerr << "SDLRenderer::load_texture - Failed to load texture " << file_path << "! SDL_image Error: " << IMG_GetError()<< std::endl;
            return nullptr;
        }
        return new SDLTexture(texture);
    }

    Color SDLRenderer::get_clear_color() const {
        return pimpl->clear_color;
    }
    void SDLRenderer::draw_texture(ITexture* texture, const Rect& dest_rect) {
        if (texture) {
            // --- Translation Layer ---
            // Convert our Rect to an SDL_Rect
            SDL_Rect sdl_dest_rect = { dest_rect.x, dest_rect.y, dest_rect.w, dest_rect.h };

            // Safely cast the interface pointer back to our concrete SDLTexture type.
            SDLTexture* sdl_texture_wrapper = dynamic_cast<SDLTexture*>(texture);

            // Get the raw SDL_Texture pointer needed for the API call.
            SDL_Texture* raw_texture = sdl_texture_wrapper->get_raw_texture();

            //  SDL_RenderCopy is the function that actually draws the texture to the backbuffer
            SDL_RenderCopy(pimpl->sdl_renderer, raw_texture, NULL, &sdl_dest_rect);
        }
    }
    // NULLIFIED NOW USING OpenGL and others.
    void SDLRenderer::draw_sprite(ITexture* texture, const Transform* transform, const Color& color, SpriteFlip flip) {
        (void)texture;
        (void)transform;
        (void)color; 
        (void)(flip);
        return;
    }
          

    void SDLRenderer::on_window_resize(int width, int height) {
    
        if (pimpl->sdl_renderer && width > 0 && height > 0) {
            // This function tells SDL to maintain a logical rendering resolution
            // and scale it to the window. It's a great way to handle
            // different resolutions in a 2D renderer.
            SDL_RenderSetLogicalSize(pimpl->sdl_renderer, width, height);

            // You should also update any internal width/height members you have
            // this->window_width = width;
            // this->window_height = height;
        }
    }


    uint32_t SDLRenderer::create_framebuffer(int /*width*/, int /*height*/) {
        return 0;
    }
    
    ImTextureID SDLRenderer::get_framebuffer_texture_id(uint32_t /*framebuffer_id*/) {
        return 0;
    }

    void SDLRenderer::bind_framebuffer(uint32_t /*framebuffer_id*/) {}
    void SDLRenderer::unbind_framebuffer() {}
    void SDLRenderer::delete_framebuffer(uint32_t /*framebuffer_id*/) {
        // Not implemented for this Renderer.
    }

    // This will just return nullptr.
    SDL_GLContext SDLRenderer::get_sdl_gl_context() const {
        return nullptr;
    } 

    SDL_Window* SDLRenderer::get_sdl_window() const {
        return pimpl->sdl_window;
    }

} // namespace Salix