#include <doctest.h>
#include <Salix/gui/DialogBox.h>
#include <Salix/gui/IDialog.h>
#include <cstdint>
#include <string>

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
