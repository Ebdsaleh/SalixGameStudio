#include <doctest.h>
#include <Salix/gui/DialogBox.h>
#include <Salix/gui/IDialog.h>
#include <cstdint>
#include <functional>
#include <string>

TEST_SUITE("Salix::gui::DialogBoxCoverage1947") {
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
