// Tests/mocking/rendering/MockITexture.h
#pragma once
#include <Salix/rendering/ITexture.h>
class MockITexture : public Salix::ITexture {
    public:
        // We only need to provide a default constructor and destructor for the mock.
        MockITexture() = default;
        ~MockITexture() override = default;
        int get_width() const override { return width;}
        int get_height() const override {return height;}
        ImTextureID get_imgui_texture_id() const {return texture_id;}
    private:
    int width = 16;
    int height = 16;
    ImTextureID texture_id = ImTextureID(18);
    };