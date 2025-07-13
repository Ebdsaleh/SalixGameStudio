// =================================================================================
// Filename:    Salix/rendering/opengl/OpenGLShaderProgram.cpp
// Author:      SalixGameStudio
// Description: Implementation of the OpenGLShaderProgram class.
// =================================================================================
#include <Salix/rendering/opengl/OpenGLShaderProgram.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // Needed here for glm::value_ptr
#include <string>               // Needed here for std::string
#include <iostream>             // Needed here for std::cerr
#include <memory>               // Needed here for std::make_unique

namespace Salix {
    
    // Pimpl struct definition
    struct OpenGLShaderProgram::Pimpl {
        // Utility function for checking shader compilation/linking errors.
        void checkCompileErrors(GLuint shader, const std::string& type); // Note: GLuint, not GLuint&
    };

    // Pimpl's checkCompileErrors implementation
    void OpenGLShaderProgram::Pimpl::checkCompileErrors(GLuint shader, const std::string& type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }

    // OpenGLShaderProgram Class Implementation
    OpenGLShaderProgram::OpenGLShaderProgram(
        const char* vertex_shader_source,
        const char* fragment_shader_source) : pimpl(std::make_unique<Pimpl>()) {
        
        // 1. Compile Shaders
        GLuint vertex, fragment;

        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_shader_source, NULL);
        glCompileShader(vertex);
        pimpl->checkCompileErrors(vertex, "VERTEX"); // Use pimpl instance

        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragment_shader_source, NULL);
        glCompileShader(fragment);
        pimpl->checkCompileErrors(fragment, "FRAGMENT"); // Use pimpl instance

        // Shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        pimpl->checkCompileErrors(ID, "PROGRAM"); // Use pimpl instance

        // Delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    OpenGLShaderProgram::~OpenGLShaderProgram() {
        if (ID != 0) {
            glDeleteProgram(ID);
            ID = 0;
        }
    }

    void OpenGLShaderProgram::use() const {
        glUseProgram(ID);
    }

    // --- NEW: Robust Uniform Setters with Error Checking ---

    void OpenGLShaderProgram::setMat4(const std::string& name, const glm::mat4& mat) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) {
            std::cerr << "SHADER WARNING: Uniform '" << name << "' not found in shader program " << ID << std::endl;
        }
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }

    void OpenGLShaderProgram::setInt(const std::string& name, int value) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) {
            std::cerr << "SHADER WARNING: Uniform '" << name << "' not found in shader program " << ID << std::endl;
        }
        glUniform1i(location, value);
    }

    void OpenGLShaderProgram::setVec4(const std::string& name, const glm::vec4& value) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) {
            std::cerr << "SHADER WARNING: Uniform '" << name << "' not found in shader program " << ID << std::endl;
        }
        glUniform4fv(location, 1, glm::value_ptr(value));
    }

    // You should also add a robust version for setVec3 if you have one
    void OpenGLShaderProgram::setVec3(const std::string& name, const glm::vec3& value) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) {
            std::cerr << "SHADER WARNING: Uniform '" << name << "' not found in shader program " << ID << std::endl;
        }
        glUniform3fv(location, 1, glm::value_ptr(value));
    }
    // --- REMOVED GLSL Shader Sources from here ---
    // These should only be defined in OpenGLRenderer.cpp where they are used to create the shader program objects.

} // namespace Salix