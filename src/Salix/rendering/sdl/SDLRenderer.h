// Salix/rendering/sdl/SDLRenderer.h
#pragma once
#include <Salix/rendering/IRenderer.h> // We need to include the interface we are implementing.
#include <SDL.h>
#include <Salix/math/Color.h>
#include <memory>
// SDLRenderer is concrete implementation of the IRenderer interface.
typedef uint64_t ImTextureID;
typedef int GLint;
namespace Salix{
    class Transform;
    class SDLRenderer : public IRenderer {
        public:
            SDLRenderer();
            virtual ~SDLRenderer() override;  // Good practice to mark overridan destructers as virtual.

            // We use the 'override' keyword to make clear we are implmenting a method from the base class.
            // The compiler will error if get the function signature incorrect, this is a great safety feature.


            // The Renderer's lifecycle methods
            bool initialize(const WindowConfig& config) override;
            void shutdown() override;

            // The core rendering commands
            void begin_frame() override;
            void end_frame() override;
            void clear() override;
            void clear_depth_buffer() override { return; }
            Color get_clear_color() const override;
            void on_window_resize(int width, int height) override;

            void set_active_camera(ICamera* camera) override { (void)camera; }

            // --- Native Renderer Specific handles ---
            SDL_GLContext get_sdl_gl_context() const override;  // This will just return nullptr.
            SDL_Window* get_sdl_window() const override;


            // Frame buffering (not needed for this type of renderer).
            uint32_t create_framebuffer(int width, int height) override;
            ImTextureID get_framebuffer_texture_id(uint32_t framebuffer_id) override;
            void bind_framebuffer(uint32_t framebuffer_id) override;
            void unbind_framebuffer() override;
            void delete_framebuffer(uint32_t framebuffer_id) override; 
            GLint get_current_framebuffer_binding() const override { return 0;}
            void set_viewport(int x, int y, int width, int height) override { x, y, width, height = 0; return;}
            // No-op for SDLRenderer as it doesn't use OpenGL framebuffers
            void SDLRenderer::restore_framebuffer_binding(GLint fbo_id) { (void) fbo_id;   }

            // Not compatible with this renderer implementation.
            void begin_render_pass(uint32_t framebuffer_id) override { (void)framebuffer_id; }
            void end_render_pass() override {}

            void set_pixels_per_unit(float ppu) override {(void) ppu;}
            float get_pixels_per_unit() const override { return 1.0f;}
            // Delcare Texture loading.
            ITexture* load_texture(const char* file_path) override;

            // Declare Texture Drawing
            void draw_texture(ITexture* texture, const Rect& dest_rect) override;

            void draw_sprite(ITexture* texture, const Transform* transform, const Color& color, SpriteFlip flip) override; 
            void draw_wire_box(const glm::mat4& model_matrix, const Color& color) override {
                (void) model_matrix; 
                (void) color;
            }

            void draw_line(const glm::vec3& start, const glm::vec3& end, const Color& color) override {
                (void) start; (void) end; (void) color;
            };
            // A method to access the IWindow-inherited object.
            IWindow* get_window() override;

            void* get_native_handle() override;

            SDL_Renderer* get_sdl_renderer();

        private:
            struct Pimpl;
            std::unique_ptr<Pimpl> pimpl;
        
    };
} // namespace Salix