// =================================================================================
// Filename:    Salix/rendering/opengl/OpenGLShaderProgram.h
// Author:      SalixGameStudio
// Description: Defines the OpenGLShaderProgram class.
// =================================================================================
#pragma once
#include <Salix/core/Core.h> // For SALIX_API
#include <glm/glm.hpp>       // For glm::mat4, glm::vec4
#include <glad/glad.h>       // For GLuint
#include <memory>            // For std::unique_ptr

// No need for glm/gtc/type_ptr.hpp, string, iostream here.
// These are used in the .cpp or in the implementation details.

namespace Salix {
    class SALIX_API OpenGLShaderProgram {
    public:
        GLuint ID; // The OpenGL program ID 

        // Constructor takes pointers to the vertex and fragment shader source code.
        OpenGLShaderProgram(const char* vertex_shader_source, const char* fragment_shader_source);
        
        // Destructor to clean up the OpenGL program.
        ~OpenGLShaderProgram();
        
        // Activate the shader program for rendering.
        void use() const;

        // Set uniform values.
        void setMat4(const std::string& name, const glm::mat4& mat) const;
        void setInt(const std::string& name, int value) const;
        void setVec4(const std::string& name, const glm::vec4& value) const;
        void setVec3(const std::string& name, const glm::vec3& value) const;

    private:
        // Pimpl idiom to hide internal implementation details like checkCompileErrors.
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
} // namespace Salix