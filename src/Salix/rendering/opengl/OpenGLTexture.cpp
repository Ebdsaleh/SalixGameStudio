// Salix/rendering/OpenGLTexture.cpp
#include <Salix/rendering/opengl/OpenGLTexture.h>
#include <iostream> // For debug output

namespace Salix {

    struct OpenGLTexture::Pimpl {
        GLuint texture_id;
        int width;
        int height;
    };
    
    
    
    OpenGLTexture::OpenGLTexture(GLuint texture_id, int width, int height) : pimpl(std::make_unique<Pimpl>()) {
        pimpl->texture_id = texture_id;
        pimpl->width = width;
        pimpl->height = height;
        std::cout << "DEBUG: OpenGLTexture created with ID: " << pimpl->texture_id << std::endl;
    }
    
    
    
    OpenGLTexture::~OpenGLTexture() {
        if (pimpl->texture_id != 0) {
            glDeleteTextures(1, &pimpl->texture_id);
            std::cout << "DEBUG: OpenGLTexture " << pimpl->texture_id << std::endl;
            pimpl->texture_id = 0;  // Mark as deleted.
        } else {
        // This case means the destructor was called but ID was already 0 (double deletion attempt or already zeroed)
        std::cout << "DEBUG: OpenGLTexture destructor called for already deleted/invalid ID (was 0)." << std::endl;
        }
    }
    


    GLuint OpenGLTexture::get_id() const { return pimpl->texture_id; }


    
    int OpenGLTexture::get_width() const {
        return pimpl->width;
    }

    
    
    int OpenGLTexture::get_height() const {
        return pimpl->height;
    }

}  // namespace Salix