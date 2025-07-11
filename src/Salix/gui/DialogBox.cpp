// Salix/gui/DialogBox.cpp
#include <Salix/gui/DialogBox.h>
#include <Salix/gui/IDialog.h>
#include <iostream> // For debug output
#include <imgui.h> // For ImGui::GetIO().DisplaySize (in set_common_dialog_properties)
#include <ImGuiFileDialog.h> // For ImGuiFileDialog::Instance()
#include <memory>

namespace Salix {

    struct DialogBox::Pimpl {
        std::string key = "";
        std::string title = "";
        DialogType type = DialogType::File;
        bool overwrite = false; // Store overwrite flag
        std::string filters = "";
        std::string default_path = ".";
        std::string default_filename ="";
        FileDialogCallback callback = nullptr;

        DialogConfig config; // Stores the actual ImGuiFileDialog config
    };

    DialogBox::DialogBox() : pimpl(std::make_unique<Pimpl>()) {

    }
    DialogBox::DialogBox(const std::string& key, const std::string& title, DialogType type, bool overwrite) : 
     pimpl(std::make_unique<Pimpl>()) {
       
        pimpl->key = key;
        pimpl->title = title;
        pimpl->type = type;
        pimpl->overwrite = overwrite;
        pimpl->filters = ""; // Initialize with empty filters
        pimpl->default_path = "."; // Initialize with default path
        pimpl->default_filename = ""; // Initialize with empty filename
        pimpl->callback  = nullptr; // Initialize with null callback
        // Populate the IGFD::FileDialogConfig struct based on initial parameters
        pimpl->config.path = pimpl->default_path;
        pimpl->config.fileName = pimpl->default_filename;
        pimpl->config.countSelectionMax = 1; // Default to single selection
        pimpl->config.userDatas = nullptr;
        pimpl->config.flags = ImGuiFileDialogFlags_Modal; // Always modal initially
        
        // Adjust flags based on initial overwrite_flag
        if (pimpl->overwrite) {
            pimpl->config.flags |= ImGuiFileDialogFlags_ConfirmOverwrite;
        }

        // Adjust filters based on initial type
        if (pimpl->type == DialogType::Folder) {
            pimpl->filters = ""; // Ensure filters are empty for folder mode
        }
    }

    DialogBox::~DialogBox() = default;


    // --- IDialog overrides (Getters) ---
    const std::string& DialogBox::get_key() const { return pimpl->key; }
    const std::string& DialogBox::get_title() const { return pimpl->title; }
    DialogType DialogBox::get_type() const { return pimpl->type; }
    const std::string& DialogBox::get_filters() const { return pimpl->filters; }
    const std::string& DialogBox::get_default_path() const { return pimpl->default_path; }
    const std::string& DialogBox::get_default_filename() const { return pimpl->default_filename; }
    const FileDialogCallback& DialogBox::get_callback() const { return pimpl->callback; }

    // --- IDialog overrides (Fluent Setters) ---
    IDialog& DialogBox::SetFilters(const std::string& filters) {
        pimpl->filters = filters;
        
        // Update m_config if filters are directly used by IGFD::FileDialogConfig
        // For IGFD, filters are passed to OpenDialog, not stored in config, so this is fine.
        return *this;
    }

    IDialog& DialogBox::SetDefaultPath(const std::string& path) {
        pimpl->default_path = path;
        pimpl->config.path = pimpl->default_path; // Update config
        return *this;
    }

    IDialog& DialogBox::SetDefaultFileName(const std::string& filename) {
        pimpl->default_filename = filename;
        pimpl->config.fileName = pimpl->default_filename; // Update config
        return *this;
    }

    IDialog& DialogBox::SetCallback(FileDialogCallback callback) {
        pimpl->callback = callback;
        return *this;
    }

    IDialog& DialogBox::SetOverwrite(bool overwrite) {
        pimpl->overwrite = overwrite;
        if (pimpl->overwrite) {
            pimpl->config.flags |= ImGuiFileDialogFlags_ConfirmOverwrite;
        } else {
            pimpl->config.flags &= ~ImGuiFileDialogFlags_ConfirmOverwrite;
        }
        return *this;
    }

    // --- IDialog override (Action Method) ---
    void DialogBox::Open() {
        // This method is called by IGui::show_dialog_by_key()
        // It prepares the ImGuiFileDialog instance to be displayed.

        // Apply common dialog properties (size/pos) - this is handled by SDLImGui::set_common_dialog_properties()
        // which is called by SDLImGui::show_dialog_by_key().

        // Open the dialog using ImGuiFileDialog

         IGFD::FileDialogConfig temp_config;

        temp_config.path = pimpl->config.path.c_str();

        temp_config.fileName = pimpl->config.fileName.c_str();

        temp_config.countSelectionMax = pimpl->config.countSelectionMax;

        temp_config.userDatas = pimpl->config.userDatas;

        temp_config.flags = pimpl->config.flags;
        // ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;


        // The filters string is passed directly to OpenDialog.
        if (pimpl->type == DialogType::File && pimpl->overwrite) {  // create or save a file
            ImGuiFileDialog::Instance()->OpenDialog(pimpl->key, pimpl->title, pimpl->filters.c_str(), temp_config);
        } else if (pimpl->type == DialogType::File && !pimpl->overwrite) {  // open a file
            ImGuiFileDialog::Instance()->OpenDialog(pimpl->key, pimpl->title, pimpl->filters.c_str(), temp_config);
        } else if (pimpl->type == DialogType::Folder && pimpl->overwrite) { // create a new folder!
            ImGuiFileDialog::Instance()->OpenDialog(pimpl->key, pimpl->title, "", temp_config); // Empty filters for folder
        } else if (pimpl->type == DialogType::Folder && !pimpl->overwrite) { // open folder!
            ImGuiFileDialog::Instance()->OpenDialog(pimpl->key, pimpl->title, "", temp_config); // Empty filters for folder
        } else {
            std::cerr << "DialogBox::Open - Unknown DialogType for key '" <<pimpl->key << "'." << std::endl;
        }
        // SDLImGui will mark it as active in its map.
    }

    // Get the underlying ImGuiFileDialog config (for SDLImGui to use)
    DialogConfig& DialogBox::get_config() { return pimpl->config; }

} // namespace Salix