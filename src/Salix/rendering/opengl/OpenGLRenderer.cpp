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

        // --- Shader Programs ---
        // 2D Shaders
        const std::string texture_vertex_file = "Assets/Shaders/OpenGL/2D/textured.vert";
        const std::string texture_fragment_file = "Assets/Shaders/OpenGL/2D/textured.frag";
        const std::string color_vertex_file = "Assets/Shaders/OpenGL/2D/color.vert";
        const std::string color_fragment_file = "Assets/Shaders/OpenGL/2D/color.frag";
        std::unique_ptr<OpenGLShaderProgram> texture_shader; // For drawing textures/sprites.
        std::unique_ptr<OpenGLShaderProgram> color_shader;   // For drawing colored rectangles.

        // 3D Shader
        const std::string simple_3d_vertex_file = "Assets/Shaders/OpenGL/3D/simple.vert";
        const std::string simple_3d_fragment_file = "Assets/Shaders/OpenGL/3D/color_only.frag";
        std::unique_ptr<OpenGLShaderProgram> simple_3d_shader;


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
            0.0f, 1.0f,         0.0f, 1.0f, // Top-left
            1.0f, 0.0f,         1.0f, 0.0f, // Bottom-right
            0.0f, 0.0f,         0.0f, 0.0f, // Bottom-left
            
            0.0f, 1.0f,         0.0f, 1.0f, // Top-left
            1.0f, 1.0f,         1.0f, 1.0f, // Top-right
            1.0f, 0.0f,         1.0f, 0.0f  // Bottom-right
        };

        // 1. Create VAO and VBO directly using DSA functions
        glCreateVertexArrays(1, &quad_vao); // Creates a VAO and stores its ID in quad_vao
        glCreateBuffers(1, &quad_vbo);      // Creates a VBO and stores its ID in quad_vbo

        // 2. Upload data to the VBO directly using DSA (glNamedBufferData)
        // No need to bind GL_ARRAY_BUFFER globally.
        glNamedBufferData(quad_vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // 3. Configure VAO attributes and link to VBO directly using DSA functions
        // These functions operate directly on 'quad_vao' by its ID.

        // --- Configure Vertex Attribute 0 (Position: layout (location = 0) in shader) ---
        // Specify which vertex buffer binding point this attribute uses (e.g., binding point 0)
        glVertexArrayAttribBinding(quad_vao, 0, 0); 
        // Specify the format of vertex attribute 0: 2 floats, not normalized, offset 0 from start of vertex data
        glVertexArrayAttribFormat(quad_vao, 0, 2, GL_FLOAT, GL_FALSE, 0); 
        // Enable vertex attribute 0 for the VAO
        glEnableVertexArrayAttrib(quad_vao, 0);

        // --- Configure Vertex Attribute 1 (Texture Coordinates: layout (location = 1) in shader) ---
        // Specify which vertex buffer binding point this attribute uses (e.g., binding point 0, as it's from the same VBO)
        glVertexArrayAttribBinding(quad_vao, 1, 0); 
        // Specify the format of vertex attribute 1: 2 floats, not normalized, offset 2*sizeof(float)
        glVertexArrayAttribFormat(quad_vao, 1, 2, GL_FLOAT, GL_FALSE, (2 * sizeof(float))); 
        // Enable vertex attribute 1 for the VAO
        glEnableVertexArrayAttrib(quad_vao, 1);

        // 4. Link the VBO (quad_vbo) to the VAO's specified binding point (binding point 0)
        // Parameters: VAO ID, binding index, VBO ID, offset into VBO, stride
        glVertexArrayVertexBuffer(quad_vao, 0, quad_vbo, 0, 4 * sizeof(float)); 
        
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

        glCreateVertexArrays(1, &cube_vao);
        glCreateBuffers(1, &cube_vbo);
        glNamedBufferData(cube_vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attritbutes (location = 0).
        glEnableVertexArrayAttrib(cube_vao, 0);
        glVertexArrayAttribBinding(cube_vao, 0, 0);
        glVertexArrayAttribFormat(cube_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

        // Color attribute (location = 1)
        glEnableVertexArrayAttrib(cube_vao, 1);
        glVertexArrayAttribBinding(cube_vao, 1, 0);
        glVertexArrayAttribFormat(cube_vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

        glVertexArrayVertexBuffer(cube_vao, 0, cube_vbo, 0, 6 * sizeof(float));
        std::cout << "DEBUG: Cube geometry setup complete." << std::endl;
    }



    void OpenGLRenderer::Pimpl::setup_shaders() {

        texture_shader = std::make_unique<OpenGLShaderProgram>(texture_vertex_file, texture_fragment_file);
        color_shader = std::make_unique<OpenGLShaderProgram>(color_vertex_file, color_fragment_file); 
        // Set the texture sampler uniform once (it refers to texture unit 0)
        texture_shader->use();
        texture_shader->setInt("texture_sampler", 0); // Ensure the sampler is set to texture unit 0
        glUseProgram(0); // Unuse shader

        // load the 3D shader
        simple_3d_shader = std::make_unique<OpenGLShaderProgram>(simple_3d_vertex_file, simple_3d_fragment_file);



        GLint modelLoc = glGetUniformLocation(texture_shader->ID, "model");
        GLint projLoc = glGetUniformLocation(texture_shader->ID, "projection");
        GLint texSamplerLoc = glGetUniformLocation(texture_shader->ID, "texture_sampler");
        GLint tintColorLoc = glGetUniformLocation(texture_shader->ID, "tint_color");
        
        
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
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Disable depth testing for 2D rendering unless explicitly needed
        // glDisable(GL_DEPTH_TEST); 

        // Enable depth testing for 3D
        glEnable(GL_DEPTH_TEST);

        // Set initial clear color
        // glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Removed as it's set in clear() or set_clear_color()
        // Set viewport to cover the entire window
        glViewport(0, 0, window_width, window_height);

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
        stbi_set_flip_vertically_on_load(true);

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
        log_file << "[DEBUG] OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

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
            glDeleteVertexArrays(1, &pimpl->quad_vao);
            pimpl->quad_vao = 0;
        }

        if (pimpl->quad_vbo != 0) {
            glDeleteBuffers(1, &pimpl->quad_vbo); 
            pimpl->quad_vbo = 0;
        }

        if (pimpl->cube_vao != 0) {
            glDeleteVertexArrays(1, &pimpl->cube_vao);
            pimpl->cube_vao = 0;
        }
        if (pimpl->cube_vbo != 0) {
            glDeleteBuffers(1, &pimpl->cube_vbo);
            pimpl->cube_vbo = 0;
        }

        pimpl->simple_3d_shader.reset();    // Calls destructor, glDeleteProgram
        pimpl->texture_shader.reset();      // Calls destructor, glDeleteProgram
        pimpl->color_shader.reset();        // Calls destructor, glDeleteProgram

        // --- Clean up any created framebuffers ---
        for (const auto& pair : pimpl->framebuffers) {
            const Framebuffer& fb = pair.second;
            glDeleteRenderbuffers(1, &fb.rbo_id);
            glDeleteTextures(1, &fb.texture_id);
            glDeleteFramebuffers(1, &fb.fbo_id);
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
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);    // for testing
        // Clear both color and depth buffers, which is crucial for 3D
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear both color and depth buffers
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
            glViewport(0, 0, width, height);
            
            // 2. Update the stored dimensions and recreate the 2D projection matrix
            pimpl->create_2D_projection_matrix(width, height);
        }
    }

    // --- 3D Method Implementations ---

    void OpenGLRenderer::set_active_camera(ICamera* camera) {
        pimpl->active_camera = camera;
    }

    void OpenGLRenderer::clear_depth_buffer() {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRenderer::draw_cube(const glm::mat4& model_matrix, const Color& color) {
        if (!pimpl->active_camera) {
            std::cerr << "WARNING: Attempted to draw_cube with no active camera set." << std::endl;
            return;
        }

        // Explicitly enable depth testing right before drawing a 3D object.
        glEnable(GL_DEPTH_TEST);
        pimpl->simple_3d_shader->use();


        // Pass the matrices to the shader.
        pimpl->simple_3d_shader->setMat4("model", model_matrix);
        pimpl->simple_3d_shader->setMat4("view", pimpl->active_camera->get_view_matrix());
        pimpl->simple_3d_shader->setMat4("projection", pimpl->active_camera->get_projection_matrix());

        // Create the glm::vec4 directly from your Color's float members. No division needed.
        glm::vec4 tint = { color.r, color.g, color.b, color.a };
        pimpl->simple_3d_shader->setVec4("tint_color", tint);


        glBindVertexArray(pimpl->cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);      // 36 vertices for a cube made of triangles.
        glBindVertexArray(0);
        glUseProgram(0);
    }


    // IRenderer Interface methods (getters)
    IWindow* OpenGLRenderer::get_window() { return pimpl->window.get(); }

    void* OpenGLRenderer::get_native_handle() {
        // The "native handle" for this renderer is its GL context
        return pimpl->gl_context;
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
        glGenTextures(1, &texture_id);
        // While glGenTextures creates the ID, using glBindTexture here is still necessary
        // for glTextureStorage2D and glTextureSubImage2D in some older GL versions (pre-4.5)
        // or if not using the full DSA path for setup. However, for 4.5, direct access is preferred.
        // The glTextureStorage2D and glTextureSubImage2D functions below are DSA.
        glBindTexture(GL_TEXTURE_2D, texture_id); 
        
        // These are not DSA texture parameter setting, but valid.
        // For DSA equivalents: glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

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
            glDeleteTextures(1, &texture_id);
            return nullptr;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // <--- TEST CODE
        // Use glTextureStorage2D for immutable storage (OpenGL 4.5 feature)
        // This allocates the memory once, making it more efficient.
        glTextureStorage2D(texture_id, 1, internal_format, width, height);   // Mip levels = 1 for now.

        // Then, upload data to the base mip level.
        glTextureSubImage2D(texture_id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

        // Generate mipmaps for the immutable texture.
        glGenerateTextureMipmap(texture_id);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);    // Unbind texture.
        
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
        // draw_sprite handles draw_texture as a special case where angle=0, pivot=nullptr, color=white, no flip
        draw_sprite(texture, dest_rect, 0.0, nullptr, Color(255, 255, 255, 255), SpriteFlip::None);
    }
    
   


    
    void OpenGLRenderer::draw_sprite(ITexture* texture, const Rect& dest_rect, double angle,
        const Point* pivot, const Color& color, SpriteFlip flip) {
        if (!texture) {
            std::cerr << "WARNING: Attempted to draw null sprite texture." << std::endl;
            return;
        }

        OpenGLTexture* opengl_texture = dynamic_cast<OpenGLTexture*>(texture);
        if (!opengl_texture) {
            std::cerr << "ERROR: Invalid texture type passed to OpenGLRenderer::draw_sprite." << std::endl;
            return;
        }
        if (opengl_texture->get_id() == 0) {
            std::cerr << "ERROR: Attempting to draw with a deleted/invalid OpenGLTexture ID (ID is 0)!" << std::endl;
            return; // Don't try to draw with a bad ID
        }
        
        // --- ADD THESE DEBUG CHECKS IMMEDIATELY HERE ---
        GLboolean is_blending_enabled;
        glGetBooleanv(GL_BLEND, &is_blending_enabled);
        GLint blend_src_rgb, blend_dest_rgb;
        glGetIntegerv(GL_BLEND_SRC_RGB, &blend_src_rgb); // GL_SRC_ALPHA is 0x0302
        glGetIntegerv(GL_BLEND_DST_RGB, &blend_dest_rgb); // GL_ONE_MINUS_SRC_ALPHA is 0x0303

        // std::cout << "DEBUG: draw_sprite - Blending State: BLEND_ENABLED=" << (is_blending_enabled ? "TRUE" : "FALSE")
        //       << ", BLEND_SRC_RGB=" << blend_src_rgb << " (0x" << std::hex << blend_src_rgb << std::dec << ")"
        //       << ", BLEND_DST_RGB=" << blend_dest_rgb << " (0x" << std::hex << blend_dest_rgb << std::dec << ")" << std::endl;
        // --- END ADDITION ---

        //std::cout << "DEBUG: Drawing sprite - Texture ID: " << opengl_texture->get_id()
        //      << " DestRect: x=" << dest_rect.x << " y=" << dest_rect.y
        //      << " w=" << dest_rect.w << " h=" << dest_rect.h 
        //      << " Angle: " << angle << " Color: " << color.r << "," << color.g << "," << color.b << "," << color.a << std::endl;
        // --- END DEBUG LINE ---
        
        glDisable(GL_DEPTH_TEST);
        pimpl->texture_shader->use();
        pimpl->texture_shader->setMat4("projection", pimpl->projection_matrix_2d);
        
        
        // Pass tint color to shader
        pimpl->texture_shader->setVec4(
            "tint_color", glm::vec4(
            color.r, color.g , color.b , color.a )
        ); 

        // --- Calculate Model Matrix for Sprite ---
        glm::mat4 model = glm::mat4(1.0f);
        
        // 1. Translate to the destination position (top-left of rect)
        model = glm::translate(model, glm::vec3(dest_rect.x, dest_rect.y, 0.0f));

        
        // 2. Handle pivot point (translate origin to pivot, rotate, translate back)
        // If pivot is provided, translate to pivot, rotate, then translate back from pivot.
        // Pivot point is relative to the top-left of the destination rectangle.
        if (pivot) {
            // Translate to pivot before rotation
            model = glm::translate(model, glm::vec3(pivot->x, pivot->y, 0.0f));
            // Rotate around the Z-axis (for 2D)
            model = glm::rotate(model, glm::radians((float)angle), glm::vec3(0.0f, 0.0f, 1.0f));
            // Translate back from pivot after rotation
            model = glm::translate(model, glm::vec3(-pivot->x, -pivot->y, 0.0f));
        } else {
            // No pivot, rotate around the top-left corner of the rect
            model = glm::rotate(model, glm::radians((float)angle), glm::vec3(0.0f, 0.0f, 1.0f));
        }
        
        // 3. Handle flipping (apply scale factors for flip)
        float scale_x = (float)dest_rect.w;
        float scale_y = (float)dest_rect.h;

        if (flip == SpriteFlip::Horizontal) {
            scale_x *= -1.0f; // Flip horizontally
        } else if (flip == SpriteFlip::Vertical) {
            scale_y *= -1.0f; // Flip vertically
        } else if (flip == SpriteFlip::Both) {
            scale_x *= -1.0f;
            scale_y *= -1.0f;
        }

        // Note: For flipping with a pivot, the translation after scaling might need adjustment
        // based on where the pivot is. A simple scale like this flips around the quad's origin (0,0).
        // For accurate flipping around the rectangle's center, you'd translate to center, scale, then translate back.
        // For simplicity here, it flips the quad in its local space before being moved to dest_rect.

        // 4. Scale to the width and height of the destination rectangle
        model = glm::scale(model, glm::vec3(scale_x, scale_y, 1.0f));

        pimpl->texture_shader->setMat4("model", model);
        // --- Texture Binding ---
        glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
        glBindTexture(GL_TEXTURE_2D, opengl_texture->get_id());
        // 'texture_sampler' uniform was already set to 0 in ShaderProgram setup.

        // --- Draw Call ---
        glBindVertexArray(pimpl->quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw 2 triangles (a quad)

        // --- Cleanup (Optional but good practice) ---
        glBindTexture(GL_TEXTURE_2D, 0);        // Unbind texture
        glBindVertexArray(0);                   // Unbind VAO
        glUseProgram(0);                        // Unuse shader program
    }
     
       

    

    void OpenGLRenderer::set_clear_color(const Color& color) {

     glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);

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

        glBindVertexArray(pimpl->quad_vao);
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

        glBindVertexArray(0);
        glUseProgram(0);
    }




    uint32_t OpenGLRenderer::create_framebuffer(int width, int height) {
        Framebuffer fb;

        // 1. Generate a Framebuffer Object (FBO)
        glGenFramebuffers(1, &fb.fbo_id);
        glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo_id);

        // 2. Create a color texture attachment
        glGenTextures(1, &fb.texture_id);
        glBindTexture(GL_TEXTURE_2D, fb.texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Attach the texture to the FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture_id, 0);

        // 3. Create a Renderbuffer Object (RBO) for depth and stencil testing
        glGenRenderbuffers(1, &fb.rbo_id);
        glBindRenderbuffer(GL_RENDERBUFFER, fb.rbo_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        // Attach the renderbuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.rbo_id);

        // 4. Check if the framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            // Clean up on failure
            glDeleteFramebuffers(1, &fb.fbo_id);
            glDeleteTextures(1, &fb.texture_id);
            glDeleteRenderbuffers(1, &fb.rbo_id);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return 0; // Return 0 to indicate failure
        }

        // 5. Unbind the framebuffer to avoid accidentally rendering to it
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
            // Get the actual OpenGL FBO handle
            GLuint fbo_handle = pimpl->framebuffers.at(framebuffer_id).fbo_id;
            // Bind it, so all subsequent drawing goes here
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);
        }
    }
    
    
    void OpenGLRenderer::unbind_framebuffer() {
        // Binding to 0 tells OpenGL to go back to drawing to the main window
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLRenderer::delete_framebuffer(uint32_t framebuffer_id) {
        if (pimpl->framebuffers.count(framebuffer_id)) {
            const Framebuffer& fb = pimpl->framebuffers.at(framebuffer_id);
            glDeleteRenderbuffers(1, &fb.rbo_id);
            glDeleteTextures(1, &fb.texture_id);
            glDeleteFramebuffers(1, &fb.fbo_id);
            
            pimpl->framebuffers.erase(framebuffer_id);
        }
    }

} // namespace Salix