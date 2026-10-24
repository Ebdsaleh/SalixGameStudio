#include <doctest.h>
#include <Salix/gui/DialogBox.h>
#include <Salix/gui/IDialog.h>
#include <cstdint>
#include <string>
#include <functional>

TEST_SUITE("Salix::gui::DialogBoxBatch1147") {
    TEST_CASE("default dialog exposes neutral property defaults") {
        Salix::DialogBox dialog;
        CHECK(dialog.get_key().empty());
        CHECK(dialog.get_title().empty());
        CHECK(dialog.get_type() == Salix::DialogType::File);
        CHECK(dialog.get_filters().empty());
        CHECK(dialog.get_default_path() == ".");
        CHECK(dialog.get_default_filename().empty());
        CHECK_FALSE(static_cast<bool>(dialog.get_callback()));
    }

    TEST_CASE("parameterized file dialog initializes modal single-selection configuration") {
        Salix::DialogBox dialog("OpenProject", "Open Project", Salix::DialogType::File, false);
        CHECK(dialog.get_key() == "OpenProject");
        CHECK(dialog.get_title() == "Open Project");
        CHECK(dialog.get_type() == Salix::DialogType::File);
        const auto& config = dialog.get_config();
        CHECK(config.path == ".");
        CHECK(config.fileName.empty());
        CHECK(config.countSelectionMax == 1);
        CHECK(config.userDatas == nullptr);
        CHECK((config.flags & ImGuiFileDialogFlags_Modal) != 0);
        CHECK((config.flags & ImGuiFileDialogFlags_ConfirmOverwrite) == 0);
    }

    TEST_CASE("overwrite constructor enables confirm-overwrite flag without changing dialog identity") {
        Salix::DialogBox dialog("Save", "Save File", Salix::DialogType::File, true);
        CHECK(dialog.get_key() == "Save");
        CHECK(dialog.get_title() == "Save File");
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_Modal) != 0);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) != 0);
    }

    TEST_CASE("folder dialog starts with empty filters and retains folder type") {
        Salix::DialogBox dialog("Folder", "Choose Folder", Salix::DialogType::Folder, false);
        CHECK(dialog.get_type() == Salix::DialogType::Folder);
        CHECK(dialog.get_filters().empty());
        CHECK(dialog.get_default_path() == ".");
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_Modal) != 0);
    }

    TEST_CASE("filter setter stores exact filter expression and returns same dialog for chaining") {
        Salix::DialogBox dialog("File", "File", Salix::DialogType::File, false);
        Salix::IDialog& chained = dialog.set_filters(".png,.jpg,.jpeg");
        CHECK(&chained == &dialog);
        CHECK(dialog.get_filters() == ".png,.jpg,.jpeg");
    }

    TEST_CASE("default path setter synchronizes public property and underlying config") {
        Salix::DialogBox dialog;
        Salix::IDialog& chained = dialog.set_default_path("Assets/Images");
        CHECK(&chained == &dialog);
        CHECK(dialog.get_default_path() == "Assets/Images");
        CHECK(dialog.get_config().path == "Assets/Images");
    }

    TEST_CASE("default filename setter synchronizes public property and underlying config") {
        Salix::DialogBox dialog;
        Salix::IDialog& chained = dialog.set_default_file_name("player.png");
        CHECK(&chained == &dialog);
        CHECK(dialog.get_default_filename() == "player.png");
        CHECK(dialog.get_config().fileName == "player.png");
    }

    TEST_CASE("callback setter stores callable without invoking it") {
        Salix::DialogBox dialog;
        bool invoked = false;
        Salix::IDialog& chained = dialog.set_callback([&](const Salix::FileDialogResult&) {
            invoked = true;
        });
        CHECK(&chained == &dialog);
        CHECK(static_cast<bool>(dialog.get_callback()));
        CHECK_FALSE(invoked);
    }

    TEST_CASE("overwrite setter toggles confirm-overwrite flag in both directions") {
        Salix::DialogBox dialog("Save", "Save", Salix::DialogType::File, false);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) == 0);
        dialog.set_overwrite(true);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) != 0);
        dialog.set_overwrite(false);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) == 0);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_Modal) != 0);
    }

    TEST_CASE("mutable config remains owned by dialog and preserves independent custom fields") {
        Salix::DialogBox dialog;
        Salix::DialogConfig& config = dialog.get_config();
        config.countSelectionMax = 4;
        config.userDatas = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x1234));
        dialog.set_default_path("Root");
        dialog.set_default_file_name("name.txt");
        CHECK(&dialog.get_config() == &config);
        CHECK(config.countSelectionMax == 4);
        CHECK(config.userDatas == reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x1234)));
        CHECK(config.path == "Root");
        CHECK(config.fileName == "name.txt");
    }
}

TEST_SUITE("Salix::gui::DialogBoxBatch1947") {
    TEST_CASE("default dialog config uses neutral ImGuiFileDialog flags") {
        Salix::DialogBox dialog;
        const auto& config = dialog.get_config();
        CHECK(config.path == ".");
        CHECK(config.fileName.empty());
        CHECK(config.countSelectionMax == 1);
        CHECK(config.userDatas == nullptr);
        CHECK(config.flags == ImGuiFileDialogFlags_None);
    }

    TEST_CASE("folder overwrite constructor combines modal and confirm overwrite flags") {
        Salix::DialogBox dialog("FolderSave", "Folder Save", Salix::DialogType::Folder, true);
        CHECK(dialog.get_type() == Salix::DialogType::Folder);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_Modal) != 0);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) != 0);
    }

    TEST_CASE("fluent configuration chain always returns the same dialog object") {
        Salix::DialogBox dialog;
        Salix::IDialog* base = &dialog;
        CHECK(&base->set_filters(".txt") == base);
        CHECK(&base->set_default_path("Assets") == base);
        CHECK(&base->set_default_file_name("notes.txt") == base);
        CHECK(&base->set_overwrite(true) == base);
    }

    TEST_CASE("empty path and filename assignments synchronize the backing config") {
        Salix::DialogBox dialog;
        dialog.set_default_path("Assets").set_default_file_name("file.txt");
        dialog.set_default_path("").set_default_file_name("");
        CHECK(dialog.get_default_path().empty());
        CHECK(dialog.get_default_filename().empty());
        CHECK(dialog.get_config().path.empty());
        CHECK(dialog.get_config().fileName.empty());
    }

    TEST_CASE("callback can be replaced without invoking either callable") {
        Salix::DialogBox dialog;
        int first_calls = 0;
        int second_calls = 0;
        dialog.set_callback([&](const Salix::FileDialogResult&) { ++first_calls; });
        dialog.set_callback([&](const Salix::FileDialogResult&) { ++second_calls; });
        CHECK(static_cast<bool>(dialog.get_callback()));
        CHECK(first_calls == 0);
        CHECK(second_calls == 0);
    }

    TEST_CASE("callback can be explicitly cleared after assignment") {
        Salix::DialogBox dialog;
        dialog.set_callback([](const Salix::FileDialogResult&) {});
        REQUIRE(static_cast<bool>(dialog.get_callback()));
        dialog.set_callback(Salix::FileDialogCallback{});
        CHECK_FALSE(static_cast<bool>(dialog.get_callback()));
    }

    TEST_CASE("folder dialog filter setter stores caller value without changing folder type") {
        Salix::DialogBox dialog("Folder", "Folder", Salix::DialogType::Folder, false);
        dialog.set_filters(".should-remain-stored");
        CHECK(dialog.get_type() == Salix::DialogType::Folder);
        CHECK(dialog.get_filters() == ".should-remain-stored");
    }

    TEST_CASE("manual config fields survive unrelated fluent setters") {
        Salix::DialogBox dialog;
        auto& config = dialog.get_config();
        config.countSelectionMax = 9;
        config.userDatas = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x55AA));
        dialog.set_filters(".png").set_overwrite(true);
        CHECK(config.countSelectionMax == 9);
        CHECK(config.userDatas == reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x55AA)));
    }

    TEST_CASE("overwrite enable and disable operations are individually idempotent") {
        Salix::DialogBox dialog("Save", "Save", Salix::DialogType::File, false);
        dialog.set_overwrite(true);
        const auto enabled = dialog.get_config().flags;
        dialog.set_overwrite(true);
        CHECK(dialog.get_config().flags == enabled);
        dialog.set_overwrite(false);
        const auto disabled = dialog.get_config().flags;
        dialog.set_overwrite(false);
        CHECK(dialog.get_config().flags == disabled);
    }

    TEST_CASE("dialog identity fields remain immutable under all configuration setters") {
        Salix::DialogBox dialog("StableKey", "Stable Title", Salix::DialogType::File, false);
        dialog.set_filters(".bin").set_default_path("Root").set_default_file_name("data.bin").set_overwrite(true);
        CHECK(dialog.get_key() == "StableKey");
        CHECK(dialog.get_title() == "Stable Title");
        CHECK(dialog.get_type() == Salix::DialogType::File);
    }
}

// ---- Relocated from src/tests/SalixEngine/gui/IDialog.test.cpp; canonical tests mirror production .cpp files ----
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
