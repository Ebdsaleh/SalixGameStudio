// Salix/rendering/sdl/SDLTexture.cpp
#include <Salix/rendering/sdl/SDLTexture.h>
#include <iostream>

namespace Salix {

    // Constructor
    SDLTexture::SDLTexture(SDL_Texture* texture) :
        sdl_texture(texture),
        width(0),
        height(0) 
        {
            if (sdl_texture) {
                // Query the texture to get its dimensions when its created.
                SDL_QueryTexture(sdl_texture, NULL, NULL, &width, &height);
            }
    }

    // Destructor

    SDLTexture::~SDLTexture() {
        // The destructor is responsible for cleaning up the SDL resource.
        if (sdl_texture) {
            SDL_DestroyTexture(sdl_texture);
        }
    }

    int SDLTexture::get_width() const {
        return width;
    }

    int SDLTexture::get_height() const {
        return height;
    }

    SDL_Texture* SDLTexture::get_raw_texture() const {
        return sdl_texture;
    }

    ImTextureID SDLTexture::get_imgui_texture_id() const {
        return (ImTextureID)sdl_texture;

    }
} // namespace Salix