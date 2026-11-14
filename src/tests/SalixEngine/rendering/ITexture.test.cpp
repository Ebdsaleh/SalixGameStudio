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

TEST_SUITE("Salix::rendering::ITextureBatch6547") {
    TEST_CASE("batch6547 width may change independently from height") {
        MockITexture texture(16, 32, ImTextureID(1));
        texture.set_dimensions(65, texture.get_height());
        CHECK(texture.get_width() == 65);
        CHECK(texture.get_height() == 32);
    }

    TEST_CASE("batch6547 height may change independently from width") {
        MockITexture texture(16, 32, ImTextureID(1));
        texture.set_dimensions(texture.get_width(), 47);
        CHECK(texture.get_width() == 16);
        CHECK(texture.get_height() == 47);
    }

    TEST_CASE("batch6547 repeated dimension changes keep latest pair") {
        MockITexture texture;
        texture.set_dimensions(1, 2);
        texture.set_dimensions(65, 47);
        CHECK(texture.get_width() == 65);
        CHECK(texture.get_height() == 47);
    }

    TEST_CASE("batch6547 repeated texture identifier changes keep latest handle") {
        MockITexture texture;
        texture.set_texture_id(ImTextureID(1));
        texture.set_texture_id(ImTextureID(6547));
        CHECK(texture.get_imgui_texture_id() == ImTextureID(6547));
    }

    TEST_CASE("batch6547 zero texture identifier does not alter nonzero dimensions") {
        MockITexture texture(65, 47, ImTextureID(6547));
        texture.set_texture_id(ImTextureID(0));
        CHECK(texture.get_imgui_texture_id() == ImTextureID(0));
        CHECK(texture.get_width() == 65);
        CHECK(texture.get_height() == 47);
    }

    TEST_CASE("batch6547 mixed sign dimensions remain representable") {
        MockITexture texture(-65, 47, ImTextureID(6));
        CHECK(texture.get_width() == -65);
        CHECK(texture.get_height() == 47);
    }

    TEST_CASE("batch6547 dimension mutation never changes texture identifier") {
        MockITexture texture(1, 2, ImTextureID(6547));
        texture.set_dimensions(65, 47);
        CHECK(texture.get_imgui_texture_id() == ImTextureID(6547));
    }

    TEST_CASE("batch6547 texture identifier mutation never changes dimensions") {
        MockITexture texture(65, 47, ImTextureID(1));
        texture.set_texture_id(ImTextureID(6547));
        CHECK(texture.get_width() == 65);
        CHECK(texture.get_height() == 47);
    }

    TEST_CASE("batch6547 two textures may share identifier while retaining independent dimensions") {
        MockITexture first(65, 47, ImTextureID(6547));
        MockITexture second(47, 65, ImTextureID(6547));
        CHECK(first.get_imgui_texture_id() == second.get_imgui_texture_id());
        CHECK(first.get_width() != second.get_width());
        CHECK(first.get_height() != second.get_height());
    }

    TEST_CASE("batch6547 base interface reflects later concrete mutations") {
        MockITexture texture(1, 2, ImTextureID(3));
        Salix::ITexture& interface = texture;
        texture.set_dimensions(65, 47);
        texture.set_texture_id(ImTextureID(6547));
        CHECK(interface.get_width() == 65);
        CHECK(interface.get_height() == 47);
        CHECK(interface.get_imgui_texture_id() == ImTextureID(6547));
    }
}
