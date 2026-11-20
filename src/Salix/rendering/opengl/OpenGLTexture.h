// Salix/graphics/OpenGLTexture.h
#pragma once
#include <glad/glad.h> 
#include <Salix/rendering/ITexture.h>
#include <memory>

namespace Salix {
    #ifdef SALIX_TESTS_ENABLED
    class SALIX_API OpenGLTexture : public ITexture {
    #else
    class OpenGLTexture : public ITexture {
    #endif
    public:
        // Constructor that takes the OpenGL texture ID, width, and height
        OpenGLTexture(GLuint texture_id, int width, int height);

        // Destructor to delete the OpenGL texture from GPU memory
        ~OpenGLTexture() override; 

        // Get the raw OpenGL texture ID
        GLuint get_id() const; 

        ImTextureID get_imgui_texture_id() const override;
        // ITexture interface methods
        int get_width() const override;
        int get_height() const override;

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
    };
}