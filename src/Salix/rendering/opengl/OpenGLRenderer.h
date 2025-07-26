// =================================================================================
// Filename:    Salix/rendering/OpenGLRenderer.h
// Author:      SalixGameStudio
// Description: Implements the IRenderer interface using modern OpenGL.
// =================================================================================
#pragma once
#include <Salix/core/Core.h>
#include <Salix/rendering/IRenderer.h>
#include <Salix/math/Rect.h>
#include <Salix/events/IEventListener.h>
#include <memory>
#include <glad/glad.h>  // GLAD provides GLuint, etc.
#include <glm/glm.hpp>  // For glm::mat4, etc.
// Forward declare SDL types to avoid including SDL headers here
struct SDL_Window;
typedef void* SDL_GLContext;
typedef uint64_t ImTextureID;
namespace Salix {

    // Forward declaration for our internal ShaderProgram helper class
    class OpenGLShaderProgram; 
    class ICamera;
    class SALIX_API OpenGLRenderer : public IRenderer, public IEventListener {
    public:
        OpenGLRenderer();
        virtual ~OpenGLRenderer() override;

        // --- IEventListener implementation ---
        void on_event(IEvent& event) override;

        // --- IRenderer Interface Implementation ---
        bool initialize(const WindowConfig& config) override;
        void shutdown() override;
        void begin_frame() override;
        void end_frame() override;
        void clear_depth_buffer() override;
        void clear() override;
        IWindow* get_window() override;
        void* get_native_handle() override;

        // --- Frame Buffering ---
        uint32_t create_framebuffer(int width, int height) override;
        ImTextureID get_framebuffer_texture_id(uint32_t framebuffer_id) override;
        void bind_framebuffer(uint32_t framebuffer_id) override;
        void unbind_framebuffer() override;


        void purge_texture(ITexture* texture);
        ITexture* load_texture(const char* file_path) override;
        void draw_texture(ITexture* texture, const Rect& dest_rect) override;
        void draw_sprite(ITexture* texture, const Rect& dest_rect, double angle, const Point* pivot, const Color& color, SpriteFlip flip) override;

        void set_clear_color(const Color& color);
        void draw_rectangle(const Rect& rect, const Color& color, bool filled);
        void on_window_resize(int width, int height) override;
        // --- 3D-SPECIFIC METHODS ---

        // Sets the active camera that the renderer will use to get view/projection matrices.
        void set_active_camera(ICamera* camera);

        // A test function to draw a simple colored cube.
        void draw_cube(const glm::mat4& model_matrix, const Color& color);

        
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        struct Framebuffer {
            GLuint fbo_id = 0; // Framebuffer Object ID
            GLuint texture_id = 0; // The color texture we render to
            GLuint rbo_id = 0; // Renderbuffer Object for depth/stencil
        };
        
    };

} // namespace Salix
