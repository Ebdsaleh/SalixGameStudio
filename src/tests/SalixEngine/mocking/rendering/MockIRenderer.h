// Tests/SalixEngine/mocking/rendering/MockIRenderer.h
#pragma once
#include <Salix/rendering/IRenderer.h>
#include <Salix/window/WindowConfig.h>
#include <Tests/SalixEngine/mocking/rendering/MockITexture.h>
#include <iostream>
#include <string>

class MockIRenderer : public Salix::IRenderer {
public:
    int draw_sprite_call_count = 0;
    Salix::SpriteFlip last_flip_state = Salix::SpriteFlip::None;
    bool should_texture_load_fail = false;

    Salix::ITexture* load_texture(const char* file_path) override {
        last_texture_path = file_path ? file_path : "";
        ++load_texture_calls;
        if (should_texture_load_fail) {
            return nullptr;
        }
        std::cout << "MockIRenderer: Loading texture from path: " << last_texture_path << std::endl;
        return new MockITexture();
    }

    bool initialize(const Salix::WindowConfig& config) override {
        last_window_config = config;
        initialized = true;
        ++initialize_calls;
        return initialize_result;
    }

    void shutdown() override { initialized = false; ++shutdown_calls; }
    void begin_frame() override { ++begin_frame_calls; }
    void end_frame() override { ++end_frame_calls; }
    void clear_depth_buffer() override { ++clear_depth_calls; }

    void set_pixels_per_unit(float ppu) override { pixels_per_unit = ppu; }
    float get_pixels_per_unit() const override { return pixels_per_unit; }

    SDL_Window* get_sdl_window() const override { return nullptr; }
    SDL_GLContext get_sdl_gl_context() const override { return nullptr; }
    Salix::OpenGLRenderer* as_opengl_renderer() override { return nullptr; }

    uint32_t create_framebuffer(int width, int height) override {
        last_framebuffer_width = width;
        last_framebuffer_height = height;
        return next_framebuffer_id++;
    }

    ImTextureID get_framebuffer_texture_id(uint32_t framebuffer_id) override {
        return static_cast<ImTextureID>(framebuffer_id + 1000u);
    }

    void bind_framebuffer(uint32_t framebuffer_id) override { current_framebuffer = static_cast<GLint>(framebuffer_id); }
    void unbind_framebuffer() override { current_framebuffer = 0; }
    GLint get_current_framebuffer_binding() const override { return current_framebuffer; }
    void delete_framebuffer(uint32_t framebuffer_id) override { last_deleted_framebuffer = framebuffer_id; }
    void restore_framebuffer_binding(GLint fbo_id) override { current_framebuffer = fbo_id; }
    void begin_render_pass(uint32_t framebuffer_id) override { bind_framebuffer(framebuffer_id); ++begin_render_pass_calls; }
    void end_render_pass() override { ++end_render_pass_calls; }

    void set_viewport(int x, int y, int width, int height) override {
        viewport_x = x; viewport_y = y; viewport_width = width; viewport_height = height;
    }

    void on_window_resize(int width, int height) override {
        resized_width = width; resized_height = height;
    }

    void set_active_camera(Salix::ICamera* camera) override { active_camera = camera; }
    Salix::ICamera* get_active_camera() override { return active_camera; }

    Salix::IWindow* get_window() override { return nullptr; }
    void* get_native_handle() override { return native_handle; }

    void clear() override { ++clear_calls; }
    Salix::Color get_clear_color() const override { return clear_color; }

    void draw_texture(Salix::ITexture* texture, const Salix::Rect& dest_rect) override {
        last_texture = texture;
        last_dest_rect = dest_rect;
        ++draw_texture_calls;
    }

    void draw_sprite(Salix::ITexture* texture, const Salix::Transform* transform, const Salix::Color& color, Salix::SpriteFlip flip) override {
        last_texture = texture;
        last_transform = transform;
        last_color = color;
        last_flip_state = flip;
        ++draw_sprite_call_count;
    }

    void draw_wire_box(const glm::mat4&, const Salix::Color&) override { ++draw_wire_box_calls; }
    void draw_line(const glm::vec3&, const glm::vec3&, const Salix::Color&) override { ++draw_line_calls; }

    const float get_line_width() const override { return line_width; }
    void draw_sphere(const glm::vec3&, float radius, const Salix::Color&, int segments = 16) override {
        last_sphere_radius = radius;
        last_sphere_segments = segments;
        ++draw_sphere_calls;
    }
    void set_line_width(float value) override { line_width = value; }

    bool initialize_result = true;
    bool initialized = false;
    int initialize_calls = 0;
    int shutdown_calls = 0;
    int begin_frame_calls = 0;
    int end_frame_calls = 0;
    int clear_depth_calls = 0;
    int clear_calls = 0;
    int load_texture_calls = 0;
    int draw_texture_calls = 0;
    int draw_wire_box_calls = 0;
    int draw_line_calls = 0;
    int draw_sphere_calls = 0;
    int begin_render_pass_calls = 0;
    int end_render_pass_calls = 0;
    float pixels_per_unit = 100.0f;
    float line_width = 1.0f;
    float last_sphere_radius = 0.0f;
    int last_sphere_segments = 0;
    uint32_t next_framebuffer_id = 1;
    uint32_t last_deleted_framebuffer = 0;
    int last_framebuffer_width = 0;
    int last_framebuffer_height = 0;
    GLint current_framebuffer = 0;
    int viewport_x = 0;
    int viewport_y = 0;
    int viewport_width = 0;
    int viewport_height = 0;
    int resized_width = 0;
    int resized_height = 0;
    Salix::ICamera* active_camera = nullptr;
    Salix::ITexture* last_texture = nullptr;
    const Salix::Transform* last_transform = nullptr;
    Salix::Color last_color{};
    Salix::Rect last_dest_rect{};
    Salix::Color clear_color{};
    Salix::WindowConfig last_window_config{};
    std::string last_texture_path;
    void* native_handle = nullptr;
};
