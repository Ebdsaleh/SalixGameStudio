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

TEST_SUITE("Salix::gui::IDialogBatch6547") {
    TEST_CASE("batch6547 default mock identity remains canonical") {
        MockIDialog dialog;
        CHECK(dialog.get_key() == "MockDialog");
        CHECK(dialog.get_title() == "Mock Dialog");
        CHECK(dialog.get_type() == Salix::DialogType::File);
    }

    TEST_CASE("batch6547 empty filter text is preserved exactly") {
        MockIDialog dialog;
        Salix::IDialog& returned = dialog.set_filters("");
        CHECK(&returned == &dialog);
        CHECK(dialog.get_filters().empty());
    }

    TEST_CASE("batch6547 empty default path replaces dot default") {
        MockIDialog dialog;
        REQUIRE(dialog.get_default_path() == ".");
        dialog.set_default_path("");
        CHECK(dialog.get_default_path().empty());
    }

    TEST_CASE("batch6547 filename setter preserves spaces and punctuation") {
        MockIDialog dialog;
        dialog.set_default_file_name("My Realm #6547.salix");
        CHECK(dialog.get_default_filename() == "My Realm #6547.salix");
    }

    TEST_CASE("batch6547 fluent setters may be chained through one interface reference") {
        MockIDialog dialog;
        Salix::IDialog& first = dialog.set_filters(".realm");
        Salix::IDialog& second = first.set_default_path("Assets/Realms");
        Salix::IDialog& third = second.set_default_file_name("Start.realm");
        CHECK(&first == &dialog);
        CHECK(&second == &dialog);
        CHECK(&third == &dialog);
        CHECK(dialog.get_filters() == ".realm");
        CHECK(dialog.get_default_path() == "Assets/Realms");
        CHECK(dialog.get_default_filename() == "Start.realm");
    }

    TEST_CASE("batch6547 mutable config transports exact user data pointer") {
        MockIDialog dialog;
        int payload = 6547;
        dialog.get_config().userDatas = &payload;
        CHECK(dialog.get_config().userDatas == &payload);
    }

    TEST_CASE("batch6547 mutable config preserves zero selection maximum") {
        MockIDialog dialog;
        dialog.get_config().countSelectionMax = 0;
        CHECK(dialog.get_config().countSelectionMax == 0);
    }

    TEST_CASE("batch6547 mutable config preserves explicit flag value") {
        MockIDialog dialog;
        const auto flags = static_cast<ImGuiFileDialogFlags>(6547);
        dialog.get_config().flags = flags;
        CHECK(dialog.get_config().flags == flags);
    }

    TEST_CASE("batch6547 callback begins empty and may be cleared after assignment") {
        MockIDialog dialog;
        CHECK_FALSE(static_cast<bool>(dialog.get_callback()));
        dialog.set_callback([](const Salix::FileDialogResult&) {});
        REQUIRE(static_cast<bool>(dialog.get_callback()));
        dialog.set_callback(Salix::FileDialogCallback{});
        CHECK_FALSE(static_cast<bool>(dialog.get_callback()));
    }

    TEST_CASE("batch6547 overwrite setting remains independent from open accounting") {
        MockIDialog dialog;
        dialog.set_overwrite(true);
        dialog.open();
        dialog.open();
        CHECK(dialog.overwrite_enabled());
        CHECK(dialog.open_calls == 2);
        dialog.set_overwrite(false);
        CHECK_FALSE(dialog.overwrite_enabled());
        CHECK(dialog.open_calls == 2);
    }
}
