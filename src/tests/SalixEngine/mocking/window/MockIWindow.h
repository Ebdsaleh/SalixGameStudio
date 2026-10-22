#pragma once

#include <Salix/window/IWindow.h>

class MockIWindow : public Salix::IWindow {
public:
    bool initialize(const Salix::WindowConfig& window_config) override {
        config = window_config;
        width = window_config.width;
        height = window_config.height;
        initialized = initialize_result;
        ++initialize_calls;
        return initialize_result;
    }

    void shutdown() override {
        initialized = false;
        ++shutdown_calls;
    }

    void* get_native_handle() const override { return native_handle; }

    void query_dimensions(int& out_width, int& out_height) const override {
        out_width = width;
        out_height = height;
    }

    int get_width() const override { return width; }
    int get_height() const override { return height; }

    void set_size(int new_width, int new_height) override {
        width = new_width;
        height = new_height;
        ++set_size_calls;
    }

    float get_dpi_scale() const override { return dpi_scale; }

    bool initialize_result = true;
    bool initialized = false;
    int initialize_calls = 0;
    int shutdown_calls = 0;
    int set_size_calls = 0;
    int width = 0;
    int height = 0;
    float dpi_scale = 1.0f;
    void* native_handle = nullptr;
    Salix::WindowConfig config{};
};
