// Salix/rendering/SDLTexture.h
#pragma once
#include <ITexture.h>
#include <SDL.h>

namespace Salix {

    // This is the concrete SDL implementation of our ITexture interface.
    class SDLTexture : public ITexture {
        public:
            // This constructor will take the raw SDL_Texture pointer.
            SDLTexture(SDL_Texture* texture);
            virtual ~SDLTexture();

            int get_width() const override;
            int get_height() const override;

            // A public helper method for the SDLRenderer to get the underlying raw pointer,
            // that it requires for SDL API calls.
            SDL_Texture*  get_raw_texture() const;
        
        private:
            SDL_Texture* sdl_texture;
            int width;
            int height;
    };
} // namespace Salix