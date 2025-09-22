// Tests/SalixEngine/mocking/rendering/MockIRenderer.h
#pragma once
#include <Salix/rendering/IRenderer.h>
#include <Salix/window/WindowConfig.h>
#include <Tests/SalixEngine/mocking/rendering/MockITexture.h>
#include <iostream>
class MockIRenderer : public Salix::IRenderer {
public:
    // For Sprite2D testing
    int draw_sprite_call_count = 0;
    Salix::SpriteFlip last_flip_state = Salix::SpriteFlip::None;
    bool should_texture_load_fail = false;

    // This is the core function your AssetManager test depends on.
    Salix::ITexture* load_texture(const char* file_path) override {
        if (should_texture_load_fail) {
            return nullptr;
        }
        // We're simulating a successful texture load here.
        std::cout << "MockIRenderer: Loading texture from path: " << file_path << std::endl;
        return new MockITexture();
    }

    // You must implement all other pure virtual functions from IRenderer.
    bool initialize(const Salix::WindowConfig& config) override { 
        (void)config; // Use (void) to suppress unused parameter warnings.
        return true; 
    }
    void shutdown() override {}
    void begin_frame() override {}
    void end_frame() override {}
    void clear_depth_buffer() override {}
    void set_pixels_per_unit(float ppu) override { (void)ppu; }
    float get_pixels_per_unit() const override { return 100.0f; }
    SDL_Window* get_sdl_window() const override { return nullptr; }
    SDL_GLContext get_sdl_gl_context() const override { return nullptr; }
    Salix::OpenGLRenderer* as_opengl_renderer() { return nullptr; }
    uint32_t create_framebuffer(int width, int height) override { (void)width; (void)height; return 0; }
    ImTextureID get_framebuffer_texture_id(uint32_t framebuffer_id) override { (void)framebuffer_id; return 0; }
    void bind_framebuffer(uint32_t framebuffer_id) override { (void)framebuffer_id; }
    void unbind_framebuffer() override {}
    GLint get_current_framebuffer_binding() const override { return 0; }
    void delete_framebuffer(uint32_t framebuffer_id) override { (void)framebuffer_id; }
    void restore_framebuffer_binding(GLint fbo_id) override { (void)fbo_id; }
    void begin_render_pass(uint32_t framebuffer_id) override { (void)framebuffer_id; }
    void end_render_pass() override {}
    void set_viewport(int x, int y, int width, int height) override { (void)x; (void)y; (void)width; (void)height; }
    void on_window_resize(int width, int height) override { (void)width; (void)height; }
    void set_active_camera(Salix::ICamera* camera) override { (void)camera; }
    Salix::IWindow* get_window() override { return nullptr; }
    void* get_native_handle() override { return nullptr; }
    void clear() override {}
    Salix::Color get_clear_color() const override { return {}; }
    void draw_texture(Salix::ITexture* texture, const Salix::Rect& dest_rect) override { (void)texture; (void)dest_rect; }
    void draw_sprite(Salix::ITexture* texture, const Salix::Transform* transform, const Salix::Color& color, Salix::SpriteFlip flip) override {
         (void)texture; (void)transform; (void)color; (void)flip;
          draw_sprite_call_count++;
            last_flip_state = flip;
        }
    void draw_wire_box(const glm::mat4& model_matrix, const Salix::Color& color) override { (void)model_matrix; (void)color; }
    void draw_line(const glm::vec3& start, const glm::vec3& end, const Salix::Color& color) override { (void)start; (void)end; (void)color; }
    const float get_line_width() const override { return 1.0f; }
    void draw_sphere(const glm::vec3& center, float radius, const Salix::Color& color, int segments = 16) override { (void)center; (void)radius; (void)color; (void)segments; }
    void set_line_width(float line_width) override { (void)line_width; }
};