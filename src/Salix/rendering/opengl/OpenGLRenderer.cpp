// =================================================================================
// Filename:    Salix/rendering/OpenGLRenderer.cpp
// Author:      SalixGameStudio
// Description: Implementation of the OpenGLRenderer class.
// =================================================================================

// IMPORTANT: Define STB_IMAGE_IMPLEMENTATION in *ONE* .cpp file only.
// This tells the preprocessor to include the function implementations from stb_image.h.
#define STB_IMAGE_IMPLEMENTATION 
#include <stb/stb_image.h> // Make sure this path is correct for your project setup
#include <Salix/rendering/ICamera.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h> 
#include <Salix/rendering/opengl/OpenGLTexture.h>
#include <Salix/window/sdl/SDLWindow.h>
#include <Salix/window/WindowConfig.h>
#include <Salix/window/IWindow.h>
#include <Salix/rendering/ITexture.h>
#include <Salix/ecs/Transform.h>       
#include <Salix/math/Rect.h>
#include <Salix/math/Color.h>
#include <Salix/math/Point.h>
#include <Salix/events/sdl/SDLEvent.h>
// Include the header for your OpenGLShaderProgram class
#include <Salix/rendering/opengl/OpenGLShaderProgram.h> 
#include <imgui/imgui.h> // required for ImGui framebuffers in the Editor.
#include <glad/glad.h> // GLAD must be included before SDL_opengl.h (if used)
#include <SDL.h>       // For SDL_GL_SwapWindow, SDL_GL_CreateContext, etc.
#include <iostream>    // For debugging output
#include <memory>      // For std::unique_ptr
#include <filesystem>
#include <fstream>
#include <stack>

// GLM includes for matrix transformations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // For glm::ortho, glm::translate, glm::scale, glm::rotate
#include <glm/gtc/type_ptr.hpp>         // For glm::value_ptr


namespace Salix {

    // --- OpenGLRenderer Pimpl Implementation ---
    struct OpenGLRenderer::Pimpl {
        std::unique_ptr<IWindow> window;
        SDL_GLContext gl_context = nullptr;
        SDL_Window* sdl_window = nullptr;
        int window_width = 0;  // Store current window dimensions.
        int window_height = 0;

        float line_width = 1.0f;
        GLuint sphere_vao = 0;
        unsigned int sphere_index_count = 0;
        std::vector<GLuint> sphere_vbos; // For cleanup
        // --- Shader Programs ---
        // 2D Shaders
        const std::string texture_vertex_file = "Assets/Shaders/OpenGL/2D/textured.vert";
        const std::string texture_fragment_file = "Assets/Shaders/OpenGL/2D/textured.frag";
        const std::string color_vertex_file = "Assets/Shaders/OpenGL/2D/color.vert";
        const std::string color_fragment_file = "Assets/Shaders/OpenGL/2D/color.frag";
        std::unique_ptr<OpenGLShaderProgram> texture_shader; // For drawing textures/sprites.
        std::unique_ptr<OpenGLShaderProgram> color_shader;   // For drawing colored rectangles.
        float pixels_per_unit = 100.0f;
        // 3D Shader
        const std::string simple_3d_vertex_file = "Assets/Shaders/OpenGL/3D/simple.vert";
        const std::string simple_3d_fragment_file = "Assets/Shaders/OpenGL/3D/color_only.frag";
        std::unique_ptr<OpenGLShaderProgram> simple_3d_shader;

        // Framebuffer stack
        std::stack<GLint> framebuffer_stack;

        // --- Geometry ---

        // 2D Quad
        GLuint quad_vao = 0; // Vertex Array Object for the quad.
        GLuint quad_vbo = 0; // Vertex Buffer Object for the quad vertices (positions and texture coordinates).

        // 3D Cube
        GLuint cube_vao = 0;
        GLuint cube_vbo = 0;


        // --- Matrices & Camera ---
        glm::mat4 projection_matrix_2d;        // Orthographic projection matrix.
        ICamera* active_camera = nullptr;    // Pointer to the active camera.

        

        std::map<uint32_t, Framebuffer> framebuffers;
        uint32_t next_framebuffer_id = 1; // A simple counter for our own IDs

        // Private helper methods for Pimpl's internal setup
        void setup_quad_geometry();
        void setup_cube_geometry();
        void setup_sphere_geometry(int segments = 16); // Default segments
        void setup_shaders();
        void create_2D_projection_matrix(int width, int height); // Takes window dimensions.
        
        void set_opengl_initial_state(); // Sets up blending, viewport, etc.
    };

    // --- Pimpl Helper Function Implementations ---
    void OpenGLRenderer::Pimpl::setup_quad_geometry() {
        // Define a unit quad (0,0) to (1,1) with texture coordinates
        // This quad will be scaled and translated by the model matrix to match dest_rect
        float vertices[] = {
            // Positions (x, y) // Texture Coords (u, v)
            -0.5f,  0.5f,         0.0f, 1.0f, // Top-left
             0.5f, -0.5f,         1.0f, 0.0f, // Bottom-right
            -0.5f, -0.5f,         0.0f, 0.0f, // Bottom-left
            
            -0.5f,  0.5f,         0.0f, 1.0f, // Top-left
             0.5f,  0.5f,         1.0f, 1.0f, // Top-right
             0.5f, -0.5f,         1.0f, 0.0f  // Bottom-right
        };

        // 1. Create VAO and VBO directly using DSA functions
        glad_glCreateVertexArrays(1, &quad_vao); // Creates a VAO and stores its ID in quad_vao
        glad_glCreateBuffers(1, &quad_vbo);      // Creates a VBO and stores its ID in quad_vbo

        // 2. Upload data to the VBO directly using DSA (glNamedBufferData)
        // No need to bind GL_ARRAY_BUFFER globally.
        glad_glNamedBufferData(quad_vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // 3. Configure VAO attributes and link to VBO directly using DSA functions
        // These functions operate directly on 'quad_vao' by its ID.

        // --- Configure Vertex Attribute 0 (Position: layout (location = 0) in shader) ---
        // Specify which vertex buffer binding point this attribute uses (e.g., binding point 0)
        glad_glVertexArrayAttribBinding(quad_vao, 0, 0); 
        // Specify the format of vertex attribute 0: 2 floats, not normalized, offset 0 from start of vertex data
        glad_glVertexArrayAttribFormat(quad_vao, 0, 2, GL_FLOAT, GL_FALSE, 0); 
        // Enable vertex attribute 0 for the VAO
        glad_glEnableVertexArrayAttrib(quad_vao, 0);

        // --- Configure Vertex Attribute 1 (Texture Coordinates: layout (location = 1) in shader) ---
        // Specify which vertex buffer binding point this attribute uses (e.g., binding point 0, as it's from the same VBO)
        glad_glVertexArrayAttribBinding(quad_vao, 1, 0); 
        // Specify the format of vertex attribute 1: 2 floats, not normalized, offset 2*sizeof(float)
        glad_glVertexArrayAttribFormat(quad_vao, 1, 2, GL_FLOAT, GL_FALSE, (2 * sizeof(float))); 
        // Enable vertex attribute 1 for the VAO
        glad_glEnableVertexArrayAttrib(quad_vao, 1);

        // 4. Link the VBO (quad_vbo) to the VAO's specified binding point (binding point 0)
        // Parameters: VAO ID, binding index, VBO ID, offset into VBO, stride
        glad_glVertexArrayVertexBuffer(quad_vao, 0, quad_vbo, 0, 4 * sizeof(float)); 
        
        std::cout << "DEBUG: Quad geometry setup complete using DSA." << std::endl;
    }


    void OpenGLRenderer::Pimpl::setup_cube_geometry() {
        // A simple cube with positions and colors
        float vertices[] = {
            // positions            // colors
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
             0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
             0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
             0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,

            -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,

            0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
            0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
            0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
            0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
             0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f

        };

        glad_glCreateVertexArrays(1, &cube_vao);
        glad_glCreateBuffers(1, &cube_vbo);
        glad_glNamedBufferData(cube_vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attritbutes (location = 0).
        glad_glEnableVertexArrayAttrib(cube_vao, 0);
        glad_glVertexArrayAttribBinding(cube_vao, 0, 0);
        glad_glVertexArrayAttribFormat(cube_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

        // Color attribute (location = 1)
        glad_glEnableVertexArrayAttrib(cube_vao, 1);
        glad_glVertexArrayAttribBinding(cube_vao, 1, 0);
        glad_glVertexArrayAttribFormat(cube_vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

        glad_glVertexArrayVertexBuffer(cube_vao, 0, cube_vbo, 0, 6 * sizeof(float));
        std::cout << "DEBUG: Cube geometry setup complete." << std::endl;
    }


   
    void OpenGLRenderer::Pimpl::setup_sphere_geometry(int segments) {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uv;
        std::vector<unsigned int> indices;

        // Generate sphere vertices (same as before)
        for (int y = 0; y <= segments; ++y) {
            for (int x = 0; x <= segments; ++x) {
                float xSegment = (float)x / (float)segments;
                float ySegment = (float)y / (float)segments;
                float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) * 
                            std::sin(ySegment * glm::pi<float>());
                float yPos = std::cos(ySegment * glm::pi<float>());
                float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) * 
                            std::sin(ySegment * glm::pi<float>());

                positions.emplace_back(xPos, yPos, zPos);
                normals.emplace_back(xPos, yPos, zPos);
                uv.emplace_back(xSegment, ySegment);
            }
        }

        // Generate indices (same as before)
        for (int y = 0; y < segments; ++y) {
            for (int x = 0; x < segments; ++x) {
                indices.push_back((y + 1) * (segments + 1) + x);
                indices.push_back(y * (segments + 1) + x);
                indices.push_back(y * (segments + 1) + x + 1);

                indices.push_back((y + 1) * (segments + 1) + x);
                indices.push_back(y * (segments + 1) + x + 1);
                indices.push_back((y + 1) * (segments + 1) + x + 1);
            }
        }

        // Create VAO/VBO using DSA (like cube)
        glad_glCreateVertexArrays(1, &sphere_vao);
        GLuint vbo, ebo;
        glad_glCreateBuffers(1, &vbo);
        glad_glCreateBuffers(1, &ebo);

        // Combine all vertex data into one buffer
        std::vector<float> vertexData;
        for (size_t i = 0; i < positions.size(); ++i) {
            vertexData.push_back(positions[i].x);
            vertexData.push_back(positions[i].y);
            vertexData.push_back(positions[i].z);
            vertexData.push_back(normals[i].x);
            vertexData.push_back(normals[i].y);
            vertexData.push_back(normals[i].z);
            vertexData.push_back(uv[i].x);
            vertexData.push_back(uv[i].y);
        }

        // Upload data using DSA
        glad_glNamedBufferData(vbo, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
        glad_glNamedBufferData(ebo, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Configure VAO attributes using DSA
        // Position (location = 0)
        glad_glEnableVertexArrayAttrib(sphere_vao, 0);
        glad_glVertexArrayAttribBinding(sphere_vao, 0, 0);
        glad_glVertexArrayAttribFormat(sphere_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

        // Normal (location = 1)
        glad_glEnableVertexArrayAttrib(sphere_vao, 1);
        glad_glVertexArrayAttribBinding(sphere_vao, 1, 0);
        glad_glVertexArrayAttribFormat(sphere_vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

        // UV (location = 2)
        glad_glEnableVertexArrayAttrib(sphere_vao, 2);
        glad_glVertexArrayAttribBinding(sphere_vao, 2, 0);
        glad_glVertexArrayAttribFormat(sphere_vao, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));

        // Bind buffers to VAO
        glad_glVertexArrayVertexBuffer(sphere_vao, 0, vbo, 0, 8 * sizeof(float));
        glad_glVertexArrayElementBuffer(sphere_vao, ebo);

        sphere_index_count = static_cast<unsigned int>(indices.size());
        sphere_vbos = {vbo, ebo}; // Store for cleanup

        std::cout << "DEBUG: Sphere geometry setup complete (DSA)." << std::endl;
    }


    void OpenGLRenderer::Pimpl::setup_shaders() {

        texture_shader = std::make_unique<OpenGLShaderProgram>(texture_vertex_file, texture_fragment_file);
        color_shader = std::make_unique<OpenGLShaderProgram>(color_vertex_file, color_fragment_file); 
        // Set the texture sampler uniform once (it refers to texture unit 0)
        texture_shader->use();
        texture_shader->setInt("texture_sampler", 0); // Ensure the sampler is set to texture unit 0
        glad_glUseProgram(0); // Unuse shader

        // load the 3D shader
        simple_3d_shader = std::make_unique<OpenGLShaderProgram>(simple_3d_vertex_file, simple_3d_fragment_file);
        if (!simple_3d_shader || simple_3d_shader->ID == 0) {
            std::cerr << "[FATAL] Failed to compile simple_3d_shader\n";
        }


        GLint modelLoc = glad_glGetUniformLocation(texture_shader->ID, "model");
        GLint projLoc = glad_glGetUniformLocation(texture_shader->ID, "projection");
        GLint texSamplerLoc = glad_glGetUniformLocation(texture_shader->ID, "texture_sampler");
        GLint tintColorLoc = glad_glGetUniformLocation(texture_shader->ID, "tint_color");
        
        
        // --- DEBUG LINES ---
        std::cout << "DEBUG: Texture Shader Uniform Locations: "
              << "model=" << modelLoc
              << ", projection=" << projLoc
              << ", texture_sampler=" << texSamplerLoc
              << ", tint_color=" << tintColorLoc << std::endl;
        // --- END DEBUG LINES ---


        

        std::cout << "DEBUG: Shaders setup complete." << std::endl;
    }



    void OpenGLRenderer::Pimpl::create_2D_projection_matrix(int width, int height) {
        // Create an orthographic projection matrix for 2D rendering.
        // This maps pixel coordinates (0,0 at top-left) to OpenGL's clip space (-1 to 1).
        // glm::ortho(left, right, bottom, top, near, far)
        // For top-left origin: left=0, right=width, bottom=height, top=0 (inverted Y)
        window_width = width;
        window_height = height;
        projection_matrix_2d = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
        std::cout << "DEBUG: Projection matrix created for " << width << "x" << height << std::endl;
    }


    void OpenGLRenderer::Pimpl::set_opengl_initial_state() {
        // Enable blending for transparency
        glad_glEnable(GL_BLEND);
        glad_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Disable depth testing for 2D rendering unless explicitly needed
        // glDisable(GL_DEPTH_TEST); 

        // Enable depth testing for 3D
        glad_glEnable(GL_DEPTH_TEST);

        // Set initial clear color
        // glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Removed as it's set in clear() or set_clear_color()
        // Set viewport to cover the entire window
        glad_glViewport(0, 0, window_width, window_height);

    }


    // --- OpenGLRenderer Public Method Implementations ---

    OpenGLRenderer::OpenGLRenderer() : pimpl(std::make_unique<Pimpl>()) {}

    OpenGLRenderer::~OpenGLRenderer() { shutdown(); } // Ensure cleanup 



    bool OpenGLRenderer::initialize(const WindowConfig& config) {
        // Open the log file for writing. This will create it or overwrite it.
        std::ofstream log_file("gl_init.log");
        if (!log_file.is_open()) {
            // If we can't even open the log file, print to cerr as a last resort.
            std::cerr << "FATAL DEBUG ERROR: Could not open gl_init.log for writing!" << std::endl;
            return false;
        }

        log_file << "[DEBUG] OpenGLRenderer::initialize START" << std::endl;
        stbi_set_flip_vertically_on_load(false);

        // --- Step 1: Set OpenGL Attributes ---
        log_file << "[DEBUG] Setting SDL_GL_SetAttribute..." << std::endl;
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        // FIX: Request an 8-bit alpha buffer for the framebuffer
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
        log_file << "[DEBUG] SDL_GL_SetAttribute... OK" << std::endl;

        // --- Create Window ---
        log_file << "[DEBUG] Creating SDLWindow..." << std::endl;
        pimpl->window = std::make_unique<SDLWindow>();
        if (!pimpl->window->initialize(config)) {
            log_file << "OpenGLRenderer Error: Failed to initialize window." << std::endl;
            return false;
        }
        pimpl->sdl_window = static_cast<SDL_Window*>(pimpl->window->get_native_handle());

        log_file << "[DEBUG] SDLWindow created... OK" << std::endl;

        // --- Step 2: Create the OpenGL Context ---
        log_file << "[DEBUG] Creating OpenGL context..." << std::endl;
        pimpl->gl_context = SDL_GL_CreateContext(static_cast<SDL_Window*>(pimpl->window->get_native_handle()));
        if (pimpl->gl_context == nullptr) {
            log_file << "OpenGLRenderer Error: Failed to create OpenGL context! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        log_file << "[DEBUG] OpenGL context created... OK" << std::endl;

        // --- Step 3: Initialize GLAD ---
        log_file << "[DEBUG] Initializing GLAD..." << std::endl;
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            log_file << "OpenGLRenderer Error: Failed to initialize GLAD!" << std::endl;
            return false;
        }
        log_file << "[DEBUG] GLAD initialized... OK" << std::endl;
        log_file << "[DEBUG] OpenGL Version: " << glad_glGetString(GL_VERSION) << std::endl;

        // --- NEW: Get the actual drawable size for High-DPI support ---
        log_file << "[DEBUG] Getting drawable size..." << std::endl;
        int drawable_width, drawable_height;
        SDL_GL_GetDrawableSize(static_cast<SDL_Window*>(pimpl->window->get_native_handle()), &drawable_width, &drawable_height);
        log_file << "[DEBUG] Window Size: " << config.width << "x" << config.height << std::endl;
        log_file << "[DEBUG] Drawable Size: " << drawable_width << "x" << drawable_height << std::endl;

        // --- Step 4: Setup OpenGL Resources using the correct drawable size ---
        pimpl->window_width = drawable_width;
        pimpl->window_height = drawable_height;

        log_file << "[DEBUG] Setting initial GL state..." << std::endl;
        pimpl->set_opengl_initial_state();
        log_file << "[DEBUG] Initial GL state set... OK" << std::endl;

        log_file << "[DEBUG] Setting up quad geometry..." << std::endl;
        pimpl->setup_quad_geometry();
        log_file << "[DEBUG] Quad geometry setup... OK" << std::endl;

        log_file << "[DEBUG] Setting up cube geometry..." << std::endl;
        pimpl->setup_cube_geometry();
        log_file << "[DEBUG Cube geometry setup... OK]" << std::endl;

        log_file << "[DEBUG] Setting up sphere geometry..." << std::endl;
        pimpl->setup_sphere_geometry();  // initializes with 16 segments.
        log_file << "[DEBUG Sphere geometry setup... OK]" << std::endl;

        log_file << "[DEBUG] Setting up shaders..." << std::endl;
        pimpl->setup_shaders();
        log_file << "[DEBUG] Shaders setup... OK" << std::endl;

        log_file << "[DEBUG] Creating projection matrix..." << std::endl;
        pimpl->create_2D_projection_matrix(drawable_width, drawable_height);
        log_file << "[DEBUG] Projection matrix created... OK" << std::endl;

        log_file << "[DEBUG] OpenGLRenderer::initialize FINISHED" << std::endl;


        return true;
    }

    void OpenGLRenderer::shutdown() {
        // Clean up OpenGL resources (VAO, VBO, Shaders)
        // These are handled by Pimpl's destructor or explicit calls

        

        if (pimpl->quad_vao != 0) {
            glad_glDeleteVertexArrays(1, &pimpl->quad_vao);
            pimpl->quad_vao = 0;
        }

        if (pimpl->quad_vbo != 0) {
            glad_glDeleteBuffers(1, &pimpl->quad_vbo); 
            pimpl->quad_vbo = 0;
        }

        if (pimpl->cube_vao != 0) {
            glad_glDeleteVertexArrays(1, &pimpl->cube_vao);
            pimpl->cube_vao = 0;
        }
        if (pimpl->cube_vbo != 0) {
            glad_glDeleteBuffers(1, &pimpl->cube_vbo);
            pimpl->cube_vbo = 0;
        }

        // --- Clean up sphere resources ---
        if (pimpl->sphere_vao != 0) {
            glad_glDeleteVertexArrays(1, &pimpl->sphere_vao);
            pimpl->sphere_vao = 0;
        }
        if (!pimpl->sphere_vbos.empty()) {
            glad_glDeleteBuffers(static_cast<GLsizei>(pimpl->sphere_vbos.size()),
                            pimpl->sphere_vbos.data());
            pimpl->sphere_vbos.clear();
        }

        pimpl->simple_3d_shader.reset();    // Calls destructor, glDeleteProgram
        pimpl->texture_shader.reset();      // Calls destructor, glDeleteProgram
        pimpl->color_shader.reset();        // Calls destructor, glDeleteProgram

        // --- Clean up any created framebuffers ---
        for (const auto& pair : pimpl->framebuffers) {
            const Framebuffer& fb = pair.second;
            glad_glDeleteRenderbuffers(1, &fb.rbo_id);
            glad_glDeleteTextures(1, &fb.texture_id);
            glad_glDeleteFramebuffers(1, &fb.fbo_id);
        }
        
        pimpl->framebuffers.clear();


        // Destroy OpenGL context
        if (pimpl->gl_context) {
            SDL_GL_DeleteContext(pimpl->gl_context);
            pimpl->gl_context = nullptr;
        }

        // Shutdown SDL window
        if (pimpl->window) {
            pimpl->window->shutdown();
            pimpl->window.reset();
        }

        // nullify the local sdl_window member (SDL_Window*)
        pimpl->sdl_window = nullptr;
        std::cout << "OpenGLRenderer shut down." << std::endl;
    }




    void OpenGLRenderer::begin_frame() {
        SDL_GL_MakeCurrent(pimpl->sdl_window, pimpl->gl_context);
        pimpl->set_opengl_initial_state();   // Re-set state in case ImGui or other things changed it.
        // FIX: Removed glClearColor here, as it's set via set_clear_color or in set_opengl_initial_state
        clear(); // Clear the buffer.
    }



    void OpenGLRenderer::end_frame() {
        // Swaps the front and back buffers to display what's rendered
        SDL_GL_SwapWindow(pimpl->sdl_window);
    }



    void OpenGLRenderer::clear() {
        // glClearColor is set in set_opengl_initial_state or via set_clear_color
        glad_glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);    // for testing
        // Clear both color and depth buffers, which is crucial for 3D
        glad_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear both color and depth buffers
    }
   
    
    void OpenGLRenderer::on_event(IEvent& event) {
        if (event.get_event_type() == EventType::WindowResize) {
            auto& resize_event = static_cast<WindowResizeEvent&>(event);
            on_window_resize(resize_event.get_width(), resize_event.get_height());
        }
    }



    void OpenGLRenderer::on_window_resize(int width, int height) {
        if (width > 0 && height > 0) {
            // 1. Update the OpenGL viewport to match the new window size
            glad_glViewport(0, 0, width, height);
            
            // 2. Update the stored dimensions and recreate the 2D projection matrix
            pimpl->create_2D_projection_matrix(width, height);
        }
    }

    // --- 3D Method Implementations ---

    void OpenGLRenderer::set_active_camera(ICamera* camera) {
        pimpl->active_camera = camera;
    }

    ICamera* OpenGLRenderer::get_active_camera() {
        return pimpl->active_camera;
    }

    void OpenGLRenderer::clear_depth_buffer() {
        glad_glClear(GL_DEPTH_BUFFER_BIT);
    }


    void OpenGLRenderer::set_line_width(float line_width) { pimpl->line_width = line_width;}

    const float OpenGLRenderer::get_line_width() const {
        return pimpl->line_width;
    }

    



    void OpenGLRenderer::draw_sphere(
        const glm::vec3& center,
        float radius, 
        const Color& color,
        int segments)
        {
            // 0. Initial checks (same as your cube implementation)
            if (!pimpl->active_camera) {
                std::cerr << "WARNING: Attempted to draw_sphere with no active camera set." << std::endl;
                return;
            }

            if (!pimpl->simple_3d_shader) {
                std::cerr << "[FATAL] simple_3d_shader is null in draw_sphere()\n";
                return;
            }

            // 1. Set up transform matrix (similar to your cube but with scale)
            glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
            model = glm::scale(model, glm::vec3(radius));

            // 2. Enable depth test (like your cube)
            glad_glEnable(GL_DEPTH_TEST);
            pimpl->simple_3d_shader->use();

            // 3. Set shader uniforms (same as cube)
            pimpl->simple_3d_shader->setMat4("model", model);
            pimpl->simple_3d_shader->setMat4("view", pimpl->active_camera->get_view_matrix());
            pimpl->simple_3d_shader->setMat4("projection", pimpl->active_camera->get_projection_matrix());
            pimpl->simple_3d_shader->setVec4("tint_color", 
                glm::vec4(color.r, color.g, color.b, color.a));

            
            // 4. Draw the sphere
            glad_glBindVertexArray(pimpl->sphere_vao);
            glad_glDrawElements(GL_TRIANGLES, 
                            pimpl->sphere_index_count, 
                            GL_UNSIGNED_INT, 
                            nullptr);
            glad_glBindVertexArray(0);
            glad_glUseProgram(0);
        }

    









    void OpenGLRenderer::draw_cube(const glm::mat4& model_matrix, const Color& color) {
        // --- ADD THESE DEBUG PRINTS ---
        GLint test_binding = -1; // Initialize to an invalid value
        glad_glGetIntegerv(GL_FRAMEBUFFER_BINDING, &test_binding); // <-- Test the problematic call here
        // std::cout << "DEBUG: OpenGLRenderer::draw_cube() - GL_FRAMEBUFFER_BINDING: " << test_binding << std::endl;
        // --- END DEBUG 
        if (!pimpl->active_camera) {
            std::cerr << "WARNING: Attempted to draw_cube with no active camera set." << std::endl;
            return;
        }

        if (!pimpl->simple_3d_shader) {
            std::cerr << "[FATAL] simple_3d_shader is null in draw_cube()\n";
            return;
        }
        // Explicitly enable depth testing right before drawing a 3D object.
        glad_glEnable(GL_DEPTH_TEST);
        pimpl->simple_3d_shader->use();


        // Pass the matrices to the shader.
        pimpl->simple_3d_shader->setMat4("model", model_matrix);
        pimpl->simple_3d_shader->setMat4("view", pimpl->active_camera->get_view_matrix());
        pimpl->simple_3d_shader->setMat4("projection", pimpl->active_camera->get_projection_matrix());

        // Create the glm::vec4 directly from your Color's float members. No division needed.
        glm::vec4 tint = { color.r, color.g, color.b, color.a };
        pimpl->simple_3d_shader->setVec4("tint_color", tint);

        if (pimpl->cube_vao == 0) {
            std::cerr << "[ERROR] cube_vao is zero (not initialized).\n";
            return;
        }
        glad_glBindVertexArray(pimpl->cube_vao);
        glad_glDrawArrays(GL_TRIANGLES, 0, 36);      // 36 vertices for a cube made of triangles.
        glad_glBindVertexArray(0);
        glad_glUseProgram(0);
    }


    void OpenGLRenderer::draw_cube(uint32_t framebuffer_id_magenta, uint32_t framebuffer_id_blue, float current_time) {

        // 0. Initial checks
        if (!pimpl->active_camera) {
            std::cerr << "WARNING: draw_cube(fbo_ids) - No active camera set." << std::endl;
            return;
        }
        if (!pimpl->simple_3d_shader) {
            std::cerr << "[FATAL] draw_cube(fbo_ids) - simple_3d_shader is null.\n";
            return;
        }

        // 1. Save the currently bound framebuffer
        GLint last_bound_fbo = 0;
        glad_glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_bound_fbo); // This should NOT crash here

        // 2. Define colors for the two cubes
        Color magenta_color = {1.0f, 0.0f, 1.0f, 1.0f};
        Color blue_color = {0.0f, 0.0f, 1.0f, 1.0f};

        // 3. Define dimensions for framebuffers (assuming they were created with consistent sizes)
        // You'll need to know the dimensions of your FBOs. For simplicity, let's use a fixed size for this test.
        // In a real scenario, you'd retrieve them from your 'framebuffers' map.
        // For now, let's assume a standard size or get it from your Framebuffer struct (if you expand it).
        // For this test, let's just hardcode a common size like 800x600 for rendering to them.
        // You should use the actual dimensions of the created FBOs if you have them.
        int fbo_width = 800; // Replace with actual FBO width
        int fbo_height = 600; // Replace with actual FBO height
        // Better: Retrieve from your pimpl->framebuffers map:
        // auto it_magenta = pimpl->framebuffers.find(framebuffer_id_magenta);
        // if (it_magenta != pimpl->framebuffers.end()) { fbo_width = it_magenta->second.width; ... }
        // (You'd need to add width/height to your Framebuffer struct if not already there)

        // 4. Alternate rendering between framebuffers based on time
        uint32_t target_fbo_id = (static_cast<int>(current_time * 2.0f) % 2 == 0) ? framebuffer_id_magenta : framebuffer_id_blue;
        Color target_color = (target_fbo_id == framebuffer_id_magenta) ? magenta_color : blue_color;

        // 5. Bind target framebuffer
        bind_framebuffer(target_fbo_id); // This will bind the FBO and update glViewport internally.
                                        // Ensure bind_framebuffer also sets the correct viewport for the FBO.
                                        // (Your current bind_framebuffer doesn't take dimensions,
                                        // so it might rely on global state or the FBO's internal dimensions).
                                        // If your FBOs have fixed internal sizes, your current glViewport in RealmDesignerPanel's crashing method is correct:
                                        // glViewport(0, 0, fbo_width, fbo_height); // Make sure to use actual FBO size here.
        
        // For this test, let's directly set viewport based on assumed sizes if bind_framebuffer doesn't.
        glad_glViewport(0, 0, fbo_width, fbo_height); // Set viewport for the FBO content.

        // 6. Clear the target framebuffer
        clear(); // This will clear the currently bound FBO (target_fbo_id)

        // 7. Render the cube to the target framebuffer
        glad_glEnable(GL_DEPTH_TEST);
        pimpl->simple_3d_shader->use();
        pimpl->simple_3d_shader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f)));
        pimpl->simple_3d_shader->setMat4("view", pimpl->active_camera->get_view_matrix());
        pimpl->simple_3d_shader->setMat4("projection", pimpl->active_camera->get_projection_matrix());
        pimpl->simple_3d_shader->setVec4("tint_color", glm::vec4(target_color.r, target_color.g, target_color.b, target_color.a));
        glad_glBindVertexArray(pimpl->cube_vao);
        glad_glDrawArrays(GL_TRIANGLES, 0, 36);
        glad_glBindVertexArray(0);
        glad_glUseProgram(0);

        // 8. Restore the previous framebuffer binding
        glad_glBindFramebuffer(GL_FRAMEBUFFER, last_bound_fbo); // Restore original FBO (usually 0)

    }







    // IRenderer Interface methods (getters)
    IWindow* OpenGLRenderer::get_window() { return pimpl->window.get(); }

    void* OpenGLRenderer::get_native_handle() {
        // The "native handle" for this renderer is its GL context
        return pimpl->gl_context;
    }

    SDL_GLContext OpenGLRenderer::get_sdl_gl_context() const {
        return pimpl->gl_context;
    }
    SDL_Window* OpenGLRenderer::get_sdl_window() const {
        return pimpl->sdl_window;
    }

    GLint OpenGLRenderer::get_gl_framebuffer_binding_internal() const {
        GLint binding = 0;
        glad_glGetIntegerv(GL_FRAMEBUFFER_BINDING, &binding); // Direct GLAD call
        return binding;
    }

    void OpenGLRenderer::set_gl_viewport_internal(int x, int y, int width, int height) {
        glad_glViewport(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height)); // Direct GLAD call
    }


    // --- Texture Management ---

    ITexture* OpenGLRenderer::load_texture(const char* file_path) {
        int width, height, channels;
        // (In OpenGL (0,0) is bottom-left, images often top-left)
        // stbi_set_flip_vertically_on_load(true); // FIX: Moved to initialize()

        unsigned char* data = stbi_load(file_path, &width, &height, &channels, 0);

        if (!data) {
            std::cerr << "ERROR: Failed to load texture: " << file_path << " - " << stbi_failure_reason() << std::endl;
            return nullptr;
        }

        GLuint texture_id;
        glad_glGenTextures(1, &texture_id);
        // While glGenTextures creates the ID, using glBindTexture here is still necessary
        // for glTextureStorage2D and glTextureSubImage2D in some older GL versions (pre-4.5)
        // or if not using the full DSA path for setup. However, for 4.5, direct access is preferred.
        // The glTextureStorage2D and glTextureSubImage2D functions below are DSA.
        glad_glBindTexture(GL_TEXTURE_2D, texture_id); 
        
        // These are not DSA texture parameter setting, but valid.
        // For DSA equivalents: glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
        glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
        glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

        GLenum format = GL_RGB;
        GLenum internal_format = GL_RGB8;   // Specify internal format for GPU storage

        if (channels == 4) {
            format = GL_RGBA;
            internal_format = GL_RGBA8;
        } else if (channels == 3) {
            format = GL_RGB;
            internal_format = GL_RGB8;
        } else {
            std::cerr << "WARNING: Unsupported number of channels (" << channels << ") for texture: " <<
            file_path << std::endl;

            stbi_image_free(data);
            glad_glDeleteTextures(1, &texture_id);
            return nullptr;
        }

        glad_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // <--- TEST CODE
        // Use glTextureStorage2D for immutable storage (OpenGL 4.5 feature)
        // This allocates the memory once, making it more efficient.
        glad_glTextureStorage2D(texture_id, 1, internal_format, width, height);   // Mip levels = 1 for now.

        // Then, upload data to the base mip level.
        glad_glTextureSubImage2D(texture_id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

        // Generate mipmaps for the immutable texture.
        glad_glGenerateTextureMipmap(texture_id);

        stbi_image_free(data);
        glad_glBindTexture(GL_TEXTURE_2D, 0);    // Unbind texture.
        
        std::cout << "DEBUG: Loaded texture " << file_path << " (ID: " << texture_id <<
        ", " << width << "x" << height << ")" << std::endl;
        
        return new OpenGLTexture(texture_id, width, height);
    }
    



    void OpenGLRenderer::purge_texture(ITexture* texture) {
        if (texture) {
        // The OpenGLTexture destructor automatically calls glDeleteTextures
            delete texture;
        }
    }

    void OpenGLRenderer::draw_texture(ITexture* texture, const Rect& dest_rect) {
        // Create a temporary Transform on the fly from the Rect
        Transform temp_transform;
        temp_transform.set_position({(float)dest_rect.x, (float)dest_rect.y, 0.0f});
        temp_transform.set_scale({(float)dest_rect.w, (float)dest_rect.h, 1.0f});
        
        // Define a default white color
        Color white = { 1.0f, 1.0f, 1.0f, 1.0f };

        // Call the new draw_sprite function with the temporary transform
        draw_sprite(texture, &temp_transform, white, SpriteFlip::None);
    }
    
    void OpenGLRenderer::set_pixels_per_unit(float ppu) {
        pimpl->pixels_per_unit = ppu;
    }
    
    
    float OpenGLRenderer::get_pixels_per_unit() const {
        return pimpl->pixels_per_unit ;
    }


    
    void OpenGLRenderer::draw_sprite(ITexture* texture, const Transform* transform,
                                 const Color& color, SpriteFlip flip) {
        if (!texture || !transform) return;
        OpenGLTexture* opengl_texture = dynamic_cast<OpenGLTexture*>(texture);
        if (!opengl_texture) return;

        // --- State setup for transparency ---
        glad_glDepthMask(GL_FALSE);
        pimpl->texture_shader->use();

        // 1. Get the 3D camera matrices
        if (!pimpl->active_camera) {
            std::cerr << "WARNING: draw_sprite called with no active 3D camera." << std::endl;
            return;
        }
        pimpl->texture_shader->setMat4("view", pimpl->active_camera->get_view_matrix());
        pimpl->texture_shader->setMat4("projection", pimpl->active_camera->get_projection_matrix());

        // 2. Build the Model Matrix directly from the Transform component
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, transform->get_position().to_glm());
        
        // Rotation (around Z for now, can be expanded to full 3D rotation)
        model = glm::rotate(model, glm::radians(transform->get_rotation().z), glm::vec3(0.0f, 0.0f, 1.0f));
        
        const float PIXELS_PER_UNIT = pimpl->pixels_per_unit;
        // Calculate the sprite's size in world units
        float world_width = (float)texture->get_width() / PIXELS_PER_UNIT;
        float world_height = (float)texture->get_height() / PIXELS_PER_UNIT;


        // Handle Flipping via negative scale 
        float scale_x = world_width * transform->get_scale().x;
        float scale_y = world_height * transform->get_scale().y;
        
        if (flip == SpriteFlip::Horizontal || flip == SpriteFlip::Both) {
            scale_x *= -1.0f;
        }
        if (flip == SpriteFlip::Vertical || flip == SpriteFlip::Both) {
            scale_y *= -1.0f;
        }
        
        model = glm::scale(model, glm::vec3(scale_x, scale_y, 1.0f));
        
        pimpl->texture_shader->setMat4("model", model);

        // 3. Set color, bind texture, and draw (same as before)
        pimpl->texture_shader->setVec4("tint_color", glm::vec4(color.r, color.g, color.b, color.a));
        glad_glActiveTexture(GL_TEXTURE0);
        glad_glBindTexture(GL_TEXTURE_2D, opengl_texture->get_id());
        glad_glBindVertexArray(pimpl->quad_vao);
        glad_glDrawArrays(GL_TRIANGLES, 0, 6);
        glad_glBindVertexArray(0);
        glad_glDepthMask(GL_TRUE);
    }
     
       



    void OpenGLRenderer::draw_wire_box(const glm::mat4& model_matrix, const Color& color) {

        if (!pimpl->active_camera) return;
        // Use the same 3D shader as the solid cube
        pimpl->simple_3d_shader->use();
        pimpl->simple_3d_shader->setMat4("model", model_matrix);
        pimpl->simple_3d_shader->setMat4("view", pimpl->active_camera->get_view_matrix());
        pimpl->simple_3d_shader->setMat4("projection", pimpl->active_camera->get_projection_matrix());
        pimpl->simple_3d_shader->setVec4("tint_color", { color.r, color.g, color.b, color.a });
        glad_glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glad_glBindVertexArray(pimpl->cube_vao);
        glad_glDrawArrays(GL_TRIANGLES, 0, 36);
        glad_glBindVertexArray(0);

        glad_glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glad_glUseProgram(0);
    }
    


    void OpenGLRenderer::draw_line(const glm::vec3& start, const glm::vec3& end, const Color& color) {
        // 1. Initial safety check
        if (!pimpl->active_camera) {
            std::cerr << "WARNING: draw_line called with no active camera set." << std::endl;
            return;
        }

        // 2. Use the 3D shader that understands view and projection matrices
        pimpl->simple_3d_shader->use();

        // 3. Set the camera matrices
        pimpl->simple_3d_shader->setMat4("view", pimpl->active_camera->get_view_matrix());
        pimpl->simple_3d_shader->setMat4("projection", pimpl->active_camera->get_projection_matrix());
        
        // 4. The line's vertices are already in world space, so the model matrix is the identity
        pimpl->simple_3d_shader->setMat4("model", glm::mat4(1.0f));
        
        // 5. Set the line color
        pimpl->simple_3d_shader->setVec4("tint_color", { color.r, color.g, color.b, color.a });

        // --- 6. Create geometry on the fly ---
        GLuint line_vao, line_vbo;
        float vertices[] = {
            start.x, start.y, start.z,
            end.x,   end.y,   end.z
        };

        // Create and bind the Vertex Array Object and Vertex Buffer Object
        glad_glGenVertexArrays(1, &line_vao);
        glad_glGenBuffers(1, &line_vbo);

        glad_glBindVertexArray(line_vao);
        glad_glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
        
        // Upload the vertex data to the VBO
        glad_glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Set up the vertex attribute pointer (we only have position for this)
        // Location 0, 3 components (x,y,z), float type, not normalized, stride is 3 floats, offset is 0
        glad_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glad_glEnableVertexAttribArray(0);

        // --- 7. Draw the line ---
        glad_glDrawArrays(GL_LINES, 0, 2);

        // --- 8. Clean up temporary objects ---
        glad_glBindVertexArray(0);
        glad_glDeleteVertexArrays(1, &line_vao);
        glad_glDeleteBuffers(1, &line_vbo);
        glad_glUseProgram(0);
    }



    void OpenGLRenderer::set_clear_color(const Color& color) {

     glad_glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);

    }

    Color OpenGLRenderer::get_clear_color() const {
        float rgba[4];
        // Use glad_glGetFloatv to query the GL_COLOR_CLEAR_VALUE state
        glad_glGetFloatv(GL_COLOR_CLEAR_VALUE, rgba);
        // Convert the float values (0.0-1.0) back to your Color struct's format (0-255 or 0.0-1.0 floats)
    
        return Color(rgba[0], rgba[1], rgba[2], rgba[3]);

    }


    // (Note: draw_rectangle will need to use pimpl->color_shader)
    void OpenGLRenderer::draw_rectangle(const Rect& rect, const Color& color, bool filled) {
        // For drawing a filled rectangle, we can use the same quad geometry
        // and apply a color-only shader.
        pimpl->color_shader->use();

        pimpl->color_shader->setMat4("projection", pimpl->projection_matrix_2d);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(rect.x, rect.y, 0.0f));
        model = glm::scale(model, glm::vec3(rect.w, rect.h, 1.0f));
        pimpl->color_shader->setMat4("model", model);
            
        pimpl->color_shader->setVec4("object_color", glm::vec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f));

        glad_glBindVertexArray(pimpl->quad_vao);
        if (filled) {
            glDrawArrays(GL_TRIANGLES, 0, 6); // Draw 2 triangles for a filled quad
        } else {
            // For an unfilled rectangle, you'd typically draw a GL_LINE_LOOP
            // or use a different VAO/VBO for lines. This is a basic implementation.
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Temporary switch to line mode
            // glDrawArrays(GL_TRIANGLES, 0, 6);
            // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Switch back
        std::cerr << "WARNING: OpenGLRenderer::draw_rectangle - unfilled mode not fully implemented." << std::endl;
        }

        glad_glBindVertexArray(0);
        glad_glUseProgram(0);
    }




    uint32_t OpenGLRenderer::create_framebuffer(int width, int height) {
        Framebuffer fb;

        // 1. Generate a Framebuffer Object (FBO)
        glad_glGenFramebuffers(1, &fb.fbo_id);
        glad_glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo_id);

        // 2. Create a color texture attachment
        glad_glGenTextures(1, &fb.texture_id);
        glad_glBindTexture(GL_TEXTURE_2D, fb.texture_id);
        glad_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Attach the texture to the FBO
        glad_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture_id, 0);

        // 3. Create a Renderbuffer Object (RBO) for depth and stencil testing
        glad_glGenRenderbuffers(1, &fb.rbo_id);
        glad_glBindRenderbuffer(GL_RENDERBUFFER, fb.rbo_id);
        glad_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        // Attach the renderbuffer
        glad_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.rbo_id);

        // 4. Check if the framebuffer is complete
        if (glad_glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            // Clean up on failure
            glad_glDeleteFramebuffers(1, &fb.fbo_id);
            glad_glDeleteTextures(1, &fb.texture_id);
            glad_glDeleteRenderbuffers(1, &fb.rbo_id);
            glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return 0; // Return 0 to indicate failure
        }

        // 5. Unbind the framebuffer to avoid accidentally rendering to it
        glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 6. Store it and return our own ID
        uint32_t our_id = pimpl->next_framebuffer_id++;
        pimpl->framebuffers[our_id] = fb;
        
        std::cout << "Framebuffer created with ID: " << our_id << std::endl;
        return our_id;
    }
        
    
    
    ImTextureID OpenGLRenderer::get_framebuffer_texture_id(uint32_t framebuffer_id) {

        if (pimpl->framebuffers.count(framebuffer_id)) {
            // Get the OpenGL texture handle (GLuint)
            GLuint texture_id = pimpl->framebuffers.at(framebuffer_id).texture_id;
            
            // Cast it to the void* that ImGui expects
            return (ImTextureID)texture_id;
        }

        // Return null if the ID was not found
        return 0;
    }



    void OpenGLRenderer::bind_framebuffer(uint32_t framebuffer_id) {
        if (pimpl->framebuffers.count(framebuffer_id)) {
            GLuint fbo_handle = pimpl->framebuffers.at(framebuffer_id).fbo_id;
            // FIX: Use the glad_ prefix here!
            glad_glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);
            // OR if you made a private helper (which is even better encapsulation):
            // pimpl->bind_gl_framebuffer_internal(GL_FRAMEBUFFER, fbo_handle);
        }
    }
    
    
    void OpenGLRenderer::unbind_framebuffer() {
        // Use the glad_ prefix here!
        glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLRenderer::delete_framebuffer(uint32_t framebuffer_id) {
        if (pimpl->framebuffers.count(framebuffer_id)) {
            const Framebuffer& fb = pimpl->framebuffers.at(framebuffer_id);
            glad_glDeleteRenderbuffers(1, &fb.rbo_id);
            glad_glDeleteTextures(1, &fb.texture_id);
            glad_glDeleteFramebuffers(1, &fb.fbo_id);
            
            pimpl->framebuffers.erase(framebuffer_id);
        }
    }

    GLint OpenGLRenderer::get_current_framebuffer_binding() const {
        GLint binding = 0;
        glad_glGetIntegerv(GL_FRAMEBUFFER_BINDING, &binding);
        return binding;
    }

    void OpenGLRenderer::set_viewport(int x, int y, int width, int height) {
        glad_glViewport(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height)); 
    }

    void OpenGLRenderer::restore_framebuffer_binding(GLint fbo_id) {
        glad_glBindFramebuffer(GL_FRAMEBUFFER, fbo_id); 
    }

    void OpenGLRenderer::begin_render_pass(uint32_t framebuffer_id) {

        // 1. Get and save the current FBO binding by pushing it onto the stack.
        GLint last_bound_fbo = 0;
        glad_glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_bound_fbo);
        pimpl->framebuffer_stack.push(last_bound_fbo);

        // 2. Bind the new framebuffer that the caller requested.
        //    (This calls your existing bind_framebuffer method).
        bind_framebuffer(framebuffer_id);

    }
        
    void OpenGLRenderer::end_render_pass() {
        // 1. Check if the stack is empty to avoid errors.
    if (!pimpl->framebuffer_stack.empty()) {
        // 2. Get the last FBO binding from the top of the stack.
        GLint last_fbo = pimpl->framebuffer_stack.top();
        pimpl->framebuffer_stack.pop();

        // 3. Restore the previous framebuffer binding.
        glad_glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
    }

    }

} // namespace Salix