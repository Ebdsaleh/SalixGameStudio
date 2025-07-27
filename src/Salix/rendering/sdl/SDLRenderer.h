// Salix/rendering/sdl/SDLRenderer.h
#pragma once
#include <Salix/rendering/IRenderer.h> // We need to include the interface we are implementing.
#include <SDL.h>
#include <memory>
// SDLRenderer is concrete implementation of the IRenderer interface.
typedef uint64_t ImTextureID;
typedef int GLint;
namespace Salix{

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
            void on_window_resize(int width, int height) override;


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

            // Delcare Texture loading.
            ITexture* load_texture(const char* file_path) override;

            // Declare Texture Drawing
            void draw_texture(ITexture* texture, const Rect& dest_rect) override;

            // Declare Sprite2D drawing
            void draw_sprite(ITexture* texture, const Rect& dest_rect, double angle, const Point* pivot, const Color& color, SpriteFlip flip) override;
            
            // A method to access the IWindow-inherited object.
            IWindow* get_window() override;

            void* get_native_handle() override;

            SDL_Renderer* get_sdl_renderer();

        private:
            struct Pimpl;
            std::unique_ptr<Pimpl> pimpl;
        
    };
} // namespace Salix