#include <doctest.h>
#include <Salix/gui/DialogBox.h>
#include <Salix/gui/IDialog.h>
#include <Salix/gui/IGui.h>
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

TEST_SUITE("Salix::gui::DialogBoxBatch2447") {
    TEST_CASE("batch2447 file dialog without overwrite is modal without confirmation flag") {
        Salix::DialogBox dialog("Open2447", "Open File", Salix::DialogType::File, false);
        CHECK(dialog.get_type() == Salix::DialogType::File);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_Modal) != 0);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) == 0);
    }

    TEST_CASE("batch2447 file dialog with overwrite combines modal and confirmation flags") {
        Salix::DialogBox dialog("Save2447", "Save File", Salix::DialogType::File, true);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_Modal) != 0);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) != 0);
    }

    TEST_CASE("batch2447 folder dialog starts with empty filters and modal configuration") {
        Salix::DialogBox dialog("Folder2447", "Choose Folder", Salix::DialogType::Folder, false);
        CHECK(dialog.get_filters().empty());
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_Modal) != 0);
    }

    TEST_CASE("batch2447 folder overwrite constructor enables confirmation without changing type") {
        Salix::DialogBox dialog("FolderSave2447", "Create Folder", Salix::DialogType::Folder, true);
        CHECK(dialog.get_type() == Salix::DialogType::Folder);
        CHECK((dialog.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) != 0);
    }

    TEST_CASE("batch2447 filter assignment leaves path and filename configuration untouched") {
        Salix::DialogBox dialog;
        dialog.set_default_path("Assets/Original");
        dialog.set_default_file_name("original.txt");
        dialog.set_filters(".png,.jpg");
        CHECK(dialog.get_filters() == ".png,.jpg");
        CHECK(dialog.get_config().path == "Assets/Original");
        CHECK(dialog.get_config().fileName == "original.txt");
    }

    TEST_CASE("batch2447 repeated default path assignments replace backing config completely") {
        Salix::DialogBox dialog;
        dialog.set_default_path("First");
        dialog.set_default_path("Second/Nested");
        CHECK(dialog.get_default_path() == "Second/Nested");
        CHECK(dialog.get_config().path == "Second/Nested");
    }

    TEST_CASE("batch2447 repeated filename assignments replace backing config completely") {
        Salix::DialogBox dialog;
        dialog.set_default_file_name("first.txt");
        dialog.set_default_file_name("second.data");
        CHECK(dialog.get_default_filename() == "second.data");
        CHECK(dialog.get_config().fileName == "second.data");
    }

    TEST_CASE("batch2447 overwrite toggling preserves unrelated config fields") {
        Salix::DialogBox dialog("Stable2447", "Stable", Salix::DialogType::File, false);
        dialog.get_config().countSelectionMax = 7;
        dialog.get_config().userDatas = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x2447));
        dialog.set_default_path("Root2447");
        dialog.set_default_file_name("file2447.bin");
        dialog.set_overwrite(true);
        dialog.set_overwrite(false);
        CHECK(dialog.get_config().countSelectionMax == 7);
        CHECK(dialog.get_config().userDatas == reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x2447)));
        CHECK(dialog.get_config().path == "Root2447");
        CHECK(dialog.get_config().fileName == "file2447.bin");
    }

    TEST_CASE("batch2447 separate dialogs own independent mutable configuration") {
        Salix::DialogBox first;
        Salix::DialogBox second;
        first.set_default_path("FirstRoot").set_default_file_name("one.txt").set_overwrite(true);
        second.set_default_path("SecondRoot").set_default_file_name("two.txt").set_overwrite(false);
        CHECK(first.get_config().path == "FirstRoot");
        CHECK(second.get_config().path == "SecondRoot");
        CHECK((first.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) != 0);
        CHECK((second.get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) == 0);
    }

    TEST_CASE("batch2447 IDialog polymorphic setters return the same concrete dialog object") {
        Salix::DialogBox dialog("Poly2447", "Poly", Salix::DialogType::File, false);
        Salix::IDialog* base = &dialog;
        CHECK(&base->set_filters(".yaml") == base);
        CHECK(&base->set_default_path("Themes") == base);
        CHECK(&base->set_default_file_name("theme.yaml") == base);
        CHECK(&base->set_overwrite(true) == base);
        CHECK(base->get_key() == "Poly2447");
        CHECK(base->get_title() == "Poly");
    }
}

TEST_SUITE("Salix::gui::DialogBoxBatch3347") {
    TEST_CASE("batch3347 parameterized file dialog preserves immutable identity through configuration") {
        Salix::DialogBox dialog("file3347","Open 3347",Salix::DialogType::File,false); dialog.set_filters(".cpp").set_default_path("src").set_default_file_name("main.cpp");
        CHECK(dialog.get_key()=="file3347"); CHECK(dialog.get_title()=="Open 3347"); CHECK(dialog.get_type()==Salix::DialogType::File);
    }
    TEST_CASE("batch3347 parameterized folder dialog preserves folder identity after filter assignment") {
        Salix::DialogBox dialog("folder3347","Folder 3347",Salix::DialogType::Folder,false); dialog.set_filters("ignored-filter");
        CHECK(dialog.get_type()==Salix::DialogType::Folder); CHECK(dialog.get_filters()=="ignored-filter"); CHECK(dialog.get_key()=="folder3347");
    }
    TEST_CASE("batch3347 default path setter keeps backing config synchronized after replacements") {
        Salix::DialogBox dialog("k","t",Salix::DialogType::File,false); dialog.set_default_path("A").set_default_path("B/C");
        CHECK(dialog.get_default_path()=="B/C"); CHECK(dialog.get_config().path=="B/C");
    }
    TEST_CASE("batch3347 filename setter keeps backing config synchronized after replacements") {
        Salix::DialogBox dialog("k","t",Salix::DialogType::File,false); dialog.set_default_file_name("first.txt").set_default_file_name("second.txt");
        CHECK(dialog.get_default_filename()=="second.txt"); CHECK(dialog.get_config().fileName=="second.txt");
    }
    TEST_CASE("batch3347 overwrite toggle changes only confirmation bit and preserves modal bit") {
        Salix::DialogBox dialog("k","t",Salix::DialogType::File,false); REQUIRE((dialog.get_config().flags&ImGuiFileDialogFlags_Modal)!=0); dialog.set_overwrite(true);
        CHECK((dialog.get_config().flags&ImGuiFileDialogFlags_Modal)!=0); CHECK((dialog.get_config().flags&ImGuiFileDialogFlags_ConfirmOverwrite)!=0); dialog.set_overwrite(false); CHECK((dialog.get_config().flags&ImGuiFileDialogFlags_Modal)!=0); CHECK((dialog.get_config().flags&ImGuiFileDialogFlags_ConfirmOverwrite)==0);
    }
    TEST_CASE("batch3347 overwrite toggling preserves configured path filename and selection count") {
        Salix::DialogBox dialog("k","t",Salix::DialogType::File,false); dialog.set_default_path("Assets").set_default_file_name("save.realm"); dialog.get_config().countSelectionMax=7; dialog.set_overwrite(true); dialog.set_overwrite(false);
        CHECK(dialog.get_config().path=="Assets"); CHECK(dialog.get_config().fileName=="save.realm"); CHECK(dialog.get_config().countSelectionMax==7);
    }
    TEST_CASE("batch3347 callback assignment stores callable without invoking it") {
        Salix::DialogBox dialog("k","t",Salix::DialogType::File,false); int calls=0; dialog.set_callback([&](const Salix::FileDialogResult&){++calls;});
        CHECK(static_cast<bool>(dialog.get_callback())); CHECK(calls==0);
    }
    TEST_CASE("batch3347 callback clearing restores empty callable state") {
        Salix::DialogBox dialog("k","t",Salix::DialogType::File,false); dialog.set_callback([](const Salix::FileDialogResult&){}); REQUIRE(static_cast<bool>(dialog.get_callback())); dialog.set_callback(nullptr); CHECK_FALSE(static_cast<bool>(dialog.get_callback()));
    }
    TEST_CASE("batch3347 fluent setters dispatched through IDialog return same concrete instance") {
        Salix::DialogBox dialog("k","t",Salix::DialogType::File,false); Salix::IDialog& base=dialog;
        CHECK(&base.set_filters(".yaml")==&dialog); CHECK(&base.set_default_path("Themes")==&dialog); CHECK(&base.set_default_file_name("dark.yaml")==&dialog); CHECK(&base.set_overwrite(true)==&dialog);
    }
    TEST_CASE("batch3347 separate dialogs keep all mutable configuration isolated") {
        Salix::DialogBox a("a","A",Salix::DialogType::File,false),b("b","B",Salix::DialogType::Folder,true); a.set_filters(".png").set_default_path("APath"); b.set_filters(".realm").set_default_path("BPath");
        CHECK(a.get_filters()==".png"); CHECK(b.get_filters()==".realm"); CHECK(a.get_config().path=="APath"); CHECK(b.get_config().path=="BPath"); CHECK((a.get_config().flags&ImGuiFileDialogFlags_ConfirmOverwrite)==0); CHECK((b.get_config().flags&ImGuiFileDialogFlags_ConfirmOverwrite)!=0);
    }
}
TEST_SUITE("Salix::gui::DialogBoxBatch3647") {
    TEST_CASE("batch3647 mutable config reference remains stable through full fluent configuration") {
        Salix::DialogBox dialog("Stable3647", "Stable", Salix::DialogType::File, false);
        Salix::DialogConfig* config = &dialog.get_config();
        dialog.set_filters(".realm").set_default_path("Scenes").set_default_file_name("Main.realm").set_overwrite(true);
        CHECK(&dialog.get_config() == config);
    }

    TEST_CASE("batch3647 direct config path mutation remains separate from public default path state") {
        Salix::DialogBox dialog;
        dialog.set_default_path("Public3647");
        dialog.get_config().path = "ConfigOnly3647";
        CHECK(dialog.get_default_path() == "Public3647");
        CHECK(dialog.get_config().path == "ConfigOnly3647");
    }

    TEST_CASE("batch3647 direct config filename mutation remains separate from public filename state") {
        Salix::DialogBox dialog;
        dialog.set_default_file_name("public3647.txt");
        dialog.get_config().fileName = "config-only3647.txt";
        CHECK(dialog.get_default_filename() == "public3647.txt");
        CHECK(dialog.get_config().fileName == "config-only3647.txt");
    }

    TEST_CASE("batch3647 filter replacement preserves all backing config fields") {
        Salix::DialogBox dialog("Filter3647", "Filter", Salix::DialogType::File, true);
        dialog.set_default_path("Root3647").set_default_file_name("file3647.bin");
        dialog.get_config().countSelectionMax = 9;
        dialog.get_config().userDatas = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x3647));
        const auto flags = dialog.get_config().flags;
        dialog.set_filters(".png,.jpg").set_filters(".realm");
        CHECK(dialog.get_filters() == ".realm");
        CHECK(dialog.get_config().path == "Root3647");
        CHECK(dialog.get_config().fileName == "file3647.bin");
        CHECK(dialog.get_config().countSelectionMax == 9);
        CHECK(dialog.get_config().userDatas == reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x3647)));
        CHECK(dialog.get_config().flags == flags);
    }

    TEST_CASE("batch3647 clearing filters preserves dialog identity and file type") {
        Salix::DialogBox dialog("ClearFilter3647", "Clear Filter", Salix::DialogType::File, false);
        dialog.set_filters(".txt");
        dialog.set_filters("");
        CHECK(dialog.get_filters().empty());
        CHECK(dialog.get_key() == "ClearFilter3647");
        CHECK(dialog.get_title() == "Clear Filter");
        CHECK(dialog.get_type() == Salix::DialogType::File);
    }

    TEST_CASE("batch3647 path setter preserves embedded null bytes in public and config strings") {
        Salix::DialogBox dialog;
        const std::string path("Root\0Hidden", 11);
        dialog.set_default_path(path);
        CHECK(dialog.get_default_path() == path);
        CHECK(dialog.get_default_path().size() == path.size());
        CHECK(dialog.get_config().path == path);
    }

    TEST_CASE("batch3647 filename setter preserves embedded null bytes in public and config strings") {
        Salix::DialogBox dialog;
        const std::string filename("file\0name.realm", 15);
        dialog.set_default_file_name(filename);
        CHECK(dialog.get_default_filename() == filename);
        CHECK(dialog.get_default_filename().size() == filename.size());
        CHECK(dialog.get_config().fileName == filename);
    }

    TEST_CASE("batch3647 overwrite changes preserve caller controlled selection and user data fields") {
        Salix::DialogBox dialog("Overwrite3647", "Overwrite", Salix::DialogType::File, false);
        dialog.get_config().countSelectionMax = 3647;
        dialog.get_config().userDatas = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xABCD));
        dialog.set_overwrite(true);
        dialog.set_overwrite(false);
        CHECK(dialog.get_config().countSelectionMax == 3647);
        CHECK(dialog.get_config().userDatas == reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xABCD)));
    }

    TEST_CASE("batch3647 callbacks remain independent across separate dialog instances") {
        Salix::DialogBox first, second;
        int first_calls = 0, second_calls = 0;
        first.set_callback([&](const Salix::FileDialogResult&) { ++first_calls; });
        second.set_callback([&](const Salix::FileDialogResult&) { ++second_calls; });
        REQUIRE(static_cast<bool>(first.get_callback()));
        REQUIRE(static_cast<bool>(second.get_callback()));
        Salix::FileDialogResult result{};
        first.get_callback()(result);
        CHECK(first_calls == 1);
        CHECK(second_calls == 0);
        second.get_callback()(result);
        CHECK(first_calls == 1);
        CHECK(second_calls == 1);
    }

    TEST_CASE("batch3647 callback replacement routes invocation only to latest callable") {
        Salix::DialogBox dialog;
        int old_calls = 0, new_calls = 0;
        dialog.set_callback([&](const Salix::FileDialogResult&) { ++old_calls; });
        dialog.set_callback([&](const Salix::FileDialogResult&) { ++new_calls; });
        REQUIRE(static_cast<bool>(dialog.get_callback()));
        Salix::FileDialogResult result{};
        dialog.get_callback()(result);
        CHECK(old_calls == 0);
        CHECK(new_calls == 1);
    }
}

