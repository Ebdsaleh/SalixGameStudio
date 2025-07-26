// Salix/gui/DialogBox.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/gui/IDialog.h> // Inherit from IDialog
#include <ImGuiFileDialog.h> // Needed for IGFD::FileDialogConfig and ImGuiFileDialogFlags
#include <string>
#include <functional>
#include <memory>


namespace Salix {
    
    class SALIX_API DialogBox : public IDialog {
    public:
        DialogBox();
        // Constructor: Takes core dialog parameters
        DialogBox(
            const std::string& key,
            const std::string& title,
            DialogType type,
            bool overwrite // Initial overwrite flag
        );

        ~DialogBox() override;

        // --- IDialog overrides (Getters) ---
        const std::string& get_key() const override;
        const std::string& get_title() const override;
        DialogType get_type() const override;
        const std::string& get_filters() const override;
        const std::string& get_default_path() const override;
        const std::string& get_default_filename() const override;
        const FileDialogCallback& get_callback() const override;
        DialogConfig& get_config() override;
        // --- IDialog overrides (Fluent Setters) ---
        IDialog& set_filters(const std::string& filters) override;
        IDialog& set_default_path(const std::string& path) override;
        IDialog& set_default_file_name(const std::string& filename) override;
        IDialog& set_callback(FileDialogCallback callback) override;
        IDialog& set_overwrite(bool overwrite) override;

        // --- IDialog override (Action Method) ---
        void open() override;

        // Get the underlying ImGuiFileDialog config (for SDLImGui to use)
        

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

} // namespace Salix