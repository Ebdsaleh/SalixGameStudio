// Salix/rendering/IRenderer.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/core/InitEnums.h>
#include <Salix/math/Color.h>  // Need the Color definition to allow tinting while rendering sprites.
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>
#include <Salix/ecs/Transform.h>
#include <Salix/rendering/ITexture.h>  // Need this to use our own renderer agnostic Texture.
#include <Salix/window/IWindow.h>
#include <Salix/rendering/ICamera.h>
#include <SDL.h>
#include <cstdint>

// Forward declare ImTextureID instead of including all of imgui.h
// This is safe because we know it's just an alias for a pointer type.
typedef uint64_t ImTextureID;


namespace Salix {

    // A renderer agnostic enum for flipping a sprite.
    enum class SpriteFlip {
        None,
        Horizontal,
        Vertical,
        Both
    };

    class OpenGLRenderer; // Forward declaration
    class SDLRenderer;

    // This is an abstract base class that defines the "contract" for any renderer.

    class SALIX_API IRenderer {
        public:
        // A virtual desctructor is essential for any class intended for polymorphism.
        virtual ~IRenderer() = default;

        // The renderer creates its own window from a config.
        virtual bool initialize(const WindowConfig& config) = 0;
        virtual void shutdown() = 0;
        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;
        virtual void clear_depth_buffer() = 0;
        virtual void set_pixels_per_unit(float ppu) = 0;
        virtual float get_pixels_per_unit() const = 0;
        // New: Get the underlying SDL_Window handle
        virtual SDL_Window* get_sdl_window() const = 0;
        virtual SDL_GLContext get_sdl_gl_context() const = 0;

        virtual OpenGLRenderer* as_opengl_renderer() { return nullptr;}
        // Returns a unique ID for the new framebuffer
        virtual uint32_t create_framebuffer(int width, int height) = 0;
        virtual ImTextureID get_framebuffer_texture_id(uint32_t framebuffer_id) = 0;
        virtual void bind_framebuffer(uint32_t framebuffer_id) = 0;
        virtual void unbind_framebuffer() = 0;
        virtual GLint get_current_framebuffer_binding() const = 0;
        virtual void delete_framebuffer(uint32_t framebuffer_id) = 0;
        // A method to restore a framebuffer binding (useful for saving/restoring state)
        virtual void restore_framebuffer_binding(GLint fbo_id) = 0;
        // High-level, stateful render pass management

        virtual void begin_render_pass(uint32_t framebuffer_id) = 0;
        virtual void end_render_pass() = 0;
        

        virtual void set_viewport(int x, int y, int width, int height) = 0;

        virtual void on_window_resize(int width, int height) = 0;
        virtual void set_active_camera(ICamera* camera) = 0;

        // Provide access to the window it owns, without giving up ownership.
        virtual IWindow* get_window() = 0;
        virtual void* get_native_handle() = 0;
        // A contract that all renderers must know how to load a texture.
        virtual ITexture* load_texture(const char* file_path) = 0;

        // This is essential to prevent drawing artifacts from previous frames.
        virtual void clear() = 0; 
        virtual Color get_clear_color() const = 0;
        // A contract for drawing a texture at a specific location.
        virtual void draw_texture(ITexture* texture, const Rect& dest_rect) = 0;

        virtual void draw_sprite(ITexture* texture, const Transform* transform, const Color& color, SpriteFlip flip) = 0;
        virtual void draw_wire_box(const glm::mat4& model_matrix, const Color& color) = 0;
        virtual void draw_line(const glm::vec3& start, const glm::vec3& end, const Color& color) = 0; 
        virtual const float get_line_width() const { return 1.0f; }
        virtual void draw_sphere(const glm::vec3& center, 
            float radius, 
            const Color& color,
            int segments = 16) {(void)center, (void)radius, (void)color, (void)segments;}
        virtual void set_line_width(float line_width) {(void)line_width;}
    };
} // namespace Salix