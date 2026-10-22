#include <doctest.h>
#include <Salix/rendering/ITexture.h>
#include <Tests/SalixEngine/mocking/rendering/MockITexture.h>
#include <memory>

TEST_SUITE("Salix::rendering::ITextureBatch1347") {
    TEST_CASE("texture interface exposes default mock dimensions") {
        MockITexture texture;
        Salix::ITexture* interface_ptr = &texture;
        CHECK(interface_ptr->get_width() == 16);
        CHECK(interface_ptr->get_height() == 16);
    }

    TEST_CASE("texture interface exposes default mock ImGui texture identifier") {
        MockITexture texture;
        Salix::ITexture* interface_ptr = &texture;
        CHECK(interface_ptr->get_imgui_texture_id() == ImTextureID(18));
    }

    TEST_CASE("texture interface preserves custom dimensions from mock construction") {
        MockITexture texture(1920, 1080, ImTextureID(99));
        Salix::ITexture* interface_ptr = &texture;
        CHECK(interface_ptr->get_width() == 1920);
        CHECK(interface_ptr->get_height() == 1080);
    }

    TEST_CASE("texture interface preserves custom texture identifier") {
        MockITexture texture(32, 32, ImTextureID(777));
        Salix::ITexture* interface_ptr = &texture;
        CHECK(interface_ptr->get_imgui_texture_id() == ImTextureID(777));
    }

    TEST_CASE("texture dimensions can represent zero sized unloaded resources") {
        MockITexture texture(0, 0, ImTextureID(0));
        Salix::ITexture* interface_ptr = &texture;
        CHECK(interface_ptr->get_width() == 0);
        CHECK(interface_ptr->get_height() == 0);
        CHECK(interface_ptr->get_imgui_texture_id() == ImTextureID(0));
    }

    TEST_CASE("texture mock preserves negative dimensions for explicit edge testing") {
        MockITexture texture(-1, -2, ImTextureID(3));
        Salix::ITexture* interface_ptr = &texture;
        CHECK(interface_ptr->get_width() == -1);
        CHECK(interface_ptr->get_height() == -2);
    }

    TEST_CASE("texture dimension changes remain observable through base interface") {
        MockITexture texture;
        Salix::ITexture* interface_ptr = &texture;
        texture.set_dimensions(128, 256);
        CHECK(interface_ptr->get_width() == 128);
        CHECK(interface_ptr->get_height() == 256);
    }

    TEST_CASE("texture identifier changes do not alter dimensions") {
        MockITexture texture(64, 48, ImTextureID(1));
        texture.set_texture_id(ImTextureID(500));
        Salix::ITexture* interface_ptr = &texture;
        CHECK(interface_ptr->get_imgui_texture_id() == ImTextureID(500));
        CHECK(interface_ptr->get_width() == 64);
        CHECK(interface_ptr->get_height() == 48);
    }

    TEST_CASE("separate texture implementations maintain independent resource state") {
        MockITexture first(10, 20, ImTextureID(30));
        MockITexture second(40, 50, ImTextureID(60));
        first.set_dimensions(11, 22);
        CHECK(first.get_width() == 11);
        CHECK(first.get_height() == 22);
        CHECK(second.get_width() == 40);
        CHECK(second.get_height() == 50);
        CHECK(second.get_imgui_texture_id() == ImTextureID(60));
    }

    TEST_CASE("texture interface supports polymorphic destruction") {
        std::unique_ptr<Salix::ITexture> texture =
            std::make_unique<MockITexture>(320, 200, ImTextureID(42));
        REQUIRE(texture != nullptr);
        CHECK(texture->get_width() == 320);
        CHECK(texture->get_height() == 200);
        CHECK(texture->get_imgui_texture_id() == ImTextureID(42));
    }
}
