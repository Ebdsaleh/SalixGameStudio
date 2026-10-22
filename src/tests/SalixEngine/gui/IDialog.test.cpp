#include <doctest.h>
#include <Salix/gui/IDialog.h>
#include <Salix/gui/IGui.h>
#include <Tests/SalixEngine/mocking/gui/MockIDialog.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::gui::IDialogBatch1447") {
    TEST_CASE("dialog config defaults to current path single selection and no flags") {
        Salix::DialogConfig config;
        CHECK(config.path == ".");
        CHECK(config.fileName.empty());
        CHECK(config.countSelectionMax == 1);
        CHECK(config.userDatas == nullptr);
        CHECK(config.flags == ImGuiFileDialogFlags_None);
    }

    TEST_CASE("dialog interface exposes construction identity and type") {
        MockIDialog dialog("save-key", "Save Project", Salix::DialogType::File);
        Salix::IDialog* interface_ptr = &dialog;
        CHECK(interface_ptr->get_key() == "save-key");
        CHECK(interface_ptr->get_title() == "Save Project");
        CHECK(interface_ptr->get_type() == Salix::DialogType::File);
    }

    TEST_CASE("dialog fluent filter setter returns same interface object") {
        MockIDialog dialog;
        Salix::IDialog* interface_ptr = &dialog;
        Salix::IDialog& returned = interface_ptr->set_filters(".salix,.yaml");
        CHECK(&returned == interface_ptr);
        CHECK(interface_ptr->get_filters() == ".salix,.yaml");
    }

    TEST_CASE("dialog path and filename setters remain independent") {
        MockIDialog dialog;
        Salix::IDialog* interface_ptr = &dialog;
        interface_ptr->set_default_path("Projects/Test");
        interface_ptr->set_default_file_name("realm.yaml");
        CHECK(interface_ptr->get_default_path() == "Projects/Test");
        CHECK(interface_ptr->get_default_filename() == "realm.yaml");
    }

    TEST_CASE("dialog mutable config is the same storage exposed by getters") {
        MockIDialog dialog;
        Salix::IDialog* interface_ptr = &dialog;
        auto& config = interface_ptr->get_config();
        config.path = "Assets";
        config.fileName = "sprite.png";
        config.countSelectionMax = 4;
        CHECK(interface_ptr->get_default_path() == "Assets");
        CHECK(interface_ptr->get_default_filename() == "sprite.png");
        CHECK(interface_ptr->get_config().countSelectionMax == 4);
    }

    TEST_CASE("dialog overwrite state is controlled through fluent interface") {
        MockIDialog dialog;
        Salix::IDialog* interface_ptr = &dialog;
        Salix::IDialog& returned = interface_ptr->set_overwrite(true);
        CHECK(&returned == interface_ptr);
        CHECK(dialog.overwrite_enabled());
        interface_ptr->set_overwrite(false);
        CHECK_FALSE(dialog.overwrite_enabled());
    }

    TEST_CASE("dialog callback setter stores invokable callback") {
        MockIDialog dialog;
        bool called = false;
        std::string file;
        dialog.set_callback([&](const Salix::FileDialogResult& result) {
            called = result.is_ok;
            file = result.file_name;
        });
        Salix::FileDialogResult result;
        result.is_ok = true;
        result.file_name = "scene.salix";
        REQUIRE(static_cast<bool>(dialog.get_callback()));
        dialog.get_callback()(result);
        CHECK(called);
        CHECK(file == "scene.salix");
    }

    TEST_CASE("dialog callback can be replaced without changing identity") {
        MockIDialog dialog("key", "Title", Salix::DialogType::Folder);
        int value = 0;
        dialog.set_callback([&](const Salix::FileDialogResult&) { value = 1; });
        dialog.set_callback([&](const Salix::FileDialogResult&) { value = 2; });
        Salix::FileDialogResult result;
        dialog.get_callback()(result);
        CHECK(value == 2);
        CHECK(dialog.get_key() == "key");
        CHECK(dialog.get_type() == Salix::DialogType::Folder);
    }

    TEST_CASE("dialog open dispatch is independently observable") {
        MockIDialog dialog;
        Salix::IDialog* interface_ptr = &dialog;
        interface_ptr->open();
        interface_ptr->open();
        CHECK(dialog.open_calls == 2);
    }

    TEST_CASE("dialog interface supports polymorphic destruction") {
        std::unique_ptr<Salix::IDialog> dialog =
            std::make_unique<MockIDialog>("poly", "Polymorphic", Salix::DialogType::Custom);
        REQUIRE(dialog != nullptr);
        CHECK(dialog->get_key() == "poly");
        dialog->open();
    }
}
