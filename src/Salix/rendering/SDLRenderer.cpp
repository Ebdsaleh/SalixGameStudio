// Salix/rendering/Renderer.cpp
#include <SDLRenderer.h>
#include <SDLTexture.h>
#include <SDL_image.h>
#include <Salix/window/SDLWindow.h>
#include <iostream>


namespace Salix {

    SDLRenderer::SDLRenderer() : sdl_renderer(nullptr), window(nullptr) {}

    SDLRenderer::~SDLRenderer() {
        // The unique_ptr will automatically be destroyed here, but we'll still call shutdown as a safeguard.
       shutdown();
    }

    bool SDLRenderer::initialize(const WindowConfig& config) {
        // 1. Create and take ownership of a new SDLWindow object.
        window = std::make_unique<SDLWindow>();

        // 2. Initialize it. If this fails, we don't need to manually delete it.
        // The function will exit, and the unique_ptr will be cleared automatically.
        if (!window->initialize(config)) {
            std:: cout << "SDLRenderer::initialize - Window initialization failed!" << std::endl;
            window.reset();  // Explicitly clear the pointer.
            return false;
        }

        // 3. Get the native handle and create the actual SDL renderer.
        void* native_handle = window->get_native_handle();
        sdl_renderer = SDL_CreateRenderer(static_cast<SDL_Window*>(native_handle), -1, SDL_RENDERER_ACCELERATED);

        if (sdl_renderer == nullptr) {
            std::cout << "Renderer::initalize - SDL_CreateRenderer Failed!" << std::endl;
            window.reset();  // Cleanup the window object.
            return false;
        }

        std::cout << "Renderer created successfully." << std::endl;
        return true;
    }

    IWindow* SDLRenderer::get_window() {
    // .get() returns the raw pointer so other parts of the engine can use it,
    // but the unique_ptr still retains ownership
    return window.get();
    }

    void SDLRenderer::shutdown() {
        std::cout << "Shutting down renderer." << std::endl;
        if (sdl_renderer) {
            SDL_DestroyRenderer(sdl_renderer);
        }

        sdl_renderer = nullptr; // Prevent double-deletion  
        window = nullptr;
    }

    void SDLRenderer::begin_frame() {
        // Set the draw color and clear the screen
        SDL_SetRenderDrawColor(sdl_renderer, 15, 20, 40, 255); // A dark-blue color.
        SDL_RenderClear(sdl_renderer);
    }

    void SDLRenderer::end_frame() {
        // Present the back buffer to the screen.
        SDL_RenderPresent(sdl_renderer);
    }

    // Texture loading implementation.
    ITexture* SDLRenderer::load_texture(const char* file_path) {
        SDL_Texture* texture = IMG_LoadTexture(sdl_renderer, file_path);

        if (texture == nullptr) {
            std::cerr << "SDLRenderer::load_texture - Failed to load texture " << file_path << "! SDL_image Error: " << IMG_GetError()<< std::endl;
            return nullptr;
        }
        return new SDLTexture(texture);
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
            SDL_RenderCopy(sdl_renderer, raw_texture, NULL, &sdl_dest_rect);
        }
    }

    void SDLRenderer::draw_sprite(ITexture* texture, const Rect& dest_rect, double angle, const Point* pivot, const Color& color, SpriteFlip flip) {
        if (texture) {

            // --- Translation Layer --- 

            // Convert our Engine's Rect to an SDL_Rect.
            SDL_Rect sdl_dest_rect = { dest_rect.x, dest_rect.y, dest_rect.w, dest_rect.h };

            // Convert our Engine's Point to an SDL_Point.
            SDL_Point sdl_pivot_point;
            if (pivot) {
                sdl_pivot_point = { pivot->x, pivot->y };
            }
            
            // Convert our 0.0-1.0 float values to 0-255 integer values.
            Uint8 r = static_cast<Uint8>(color.r * 255.0f);
            Uint8 g = static_cast<Uint8>(color.g * 255.0f);
            Uint8 b = static_cast<Uint8>(color.b * 255.0f);
            Uint8 a = static_cast<Uint8>(color.a * 255.0f);

            // Safely cast the interface pointer back to our concrete SDLTexture type.
            SDLTexture* sdl_texture_wrapper = dynamic_cast<SDLTexture*>(texture);

            // Get the raw SDL_Texture pointer needed for the API call.
            SDL_Texture* raw_texture = sdl_texture_wrapper->get_raw_texture();

            // Set the texture's color and alpha modulation.
            SDL_SetTextureColorMod(raw_texture, r, g, b);
            SDL_SetTextureAlphaMod(raw_texture, a);

            // Translation from our SpriteFlip enum to SDL's flag
            SDL_RendererFlip flip_flag = SDL_FLIP_NONE;
            switch (flip) {
                case SpriteFlip::Horizontal:
                    flip_flag = SDL_FLIP_HORIZONTAL;
                    break;
                case SpriteFlip::Vertical:
                    flip_flag = SDL_FLIP_VERTICAL;
                    break;
                case SpriteFlip::Both:
                    flip_flag = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
                    break;
                default:
                    flip_flag = SDL_FLIP_NONE;
                    break;
            }

            // Draw the textured, tinted, and rotated sprite.
            SDL_RenderCopyEx(sdl_renderer, raw_texture, NULL, &sdl_dest_rect, angle, (pivot ? &sdl_pivot_point : NULL), flip_flag);
        }
    }
} // namespace Salix