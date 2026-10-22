// Tests/SalixEngine/mocking/rendering/MockITexture.h
#pragma once
#include <Salix/rendering/ITexture.h>

class MockITexture : public Salix::ITexture {
public:
    MockITexture(int width = 16, int height = 16, ImTextureID texture_id = ImTextureID(18))
        : width(width), height(height), texture_id(texture_id) {}

    ~MockITexture() override = default;

    int get_width() const override { return width; }
    int get_height() const override { return height; }
    ImTextureID get_imgui_texture_id() const override { return texture_id; }

    void set_dimensions(int new_width, int new_height) {
        width = new_width;
        height = new_height;
    }

    void set_texture_id(ImTextureID new_texture_id) {
        texture_id = new_texture_id;
    }

private:
    int width;
    int height;
    ImTextureID texture_id;
};
