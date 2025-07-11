// Salix/gui/IDialog.h
#pragma once
#include <Salix/core/Core.h> // For SALIX_API
#include <string>             // For std::string
#include <functional>         // For std::function (for callbacks)
#include <ImGuiFileDialog/ImGuiFileDialog.h>


namespace Salix { struct FileDialogResult; }

namespace Salix {

    // Enum for dialog types (copy from IGui.h if not already there)
    enum class DialogType {
        File,
        Folder,
        Custom,
        Message,
        Confirm,
        ColorPicker
        // Add other types like Message, Confirm, etc.
    };
    
    // NEW: Our own DialogConfig struct
    struct DialogConfig {
        std::string path = ".";
        std::string fileName = "";
        int32_t countSelectionMax = 1;
        void* userDatas = nullptr;
        ImGuiFileDialogFlags flags = ImGuiFileDialogFlags_None; // Use ImGuiFileDialogFlags for flags
        // No sidePane, sidePaneWidth, userFileAttributes (these are IGFD specific)
    };
    
    // Callback type for dialog results
    using FileDialogCallback = std::function<void(const FileDialogResult&)>;

    class SALIX_API IDialog {
        public: 
            virtual ~IDialog() = default;

            // --- Core Dialog Properties (Getters) ---
            virtual const std::string& get_key() const = 0;
            virtual const std::string& get_title() const = 0;
            virtual DialogType get_type() const = 0;
            virtual const std::string& get_filters() const = 0;
            virtual const std::string& get_default_path() const = 0;
            virtual const std::string& get_default_filename() const = 0;
            virtual const FileDialogCallback& get_callback() const = 0;
            virtual DialogConfig& get_config() = 0;
            // --- Fluent Setters for Configuration ---
            // These return a reference to *this (IDialog&) to allow chaining.
            virtual IDialog& SetFilters(const std::string& filters) = 0;
            virtual IDialog& SetDefaultPath(const std::string& path) = 0;
            virtual IDialog& SetDefaultFileName(const std::string& filename) = 0;
            virtual IDialog& SetCallback(FileDialogCallback callback) = 0;
            // Optional: Add setters for other flags if needed (e.g., SetOverwrite(bool))
            virtual IDialog& SetOverwrite(bool overwrite) = 0; // For SaveFile dialogs

            // --- Action Method ---
            // This method is called by IGui::show_dialog_by_key() to make the dialog appear.
            virtual void Open() = 0;
    };

} // namespace Salix