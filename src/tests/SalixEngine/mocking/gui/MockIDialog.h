#pragma once

#include <Salix/gui/IDialog.h>
#include <utility>

class MockIDialog : public Salix::IDialog {
public:
    MockIDialog(
        std::string key = "MockDialog",
        std::string title = "Mock Dialog",
        Salix::DialogType type = Salix::DialogType::File)
        : key_(std::move(key)), title_(std::move(title)), type_(type) {}

    const std::string& get_key() const override { return key_; }
    const std::string& get_title() const override { return title_; }
    Salix::DialogType get_type() const override { return type_; }
    const std::string& get_filters() const override { return filters_; }
    const std::string& get_default_path() const override { return config_.path; }
    const std::string& get_default_filename() const override { return config_.fileName; }
    const Salix::FileDialogCallback& get_callback() const override { return callback_; }
    Salix::DialogConfig& get_config() override { return config_; }

    Salix::IDialog& set_filters(const std::string& filters) override {
        filters_ = filters;
        return *this;
    }

    Salix::IDialog& set_default_path(const std::string& path) override {
        config_.path = path;
        return *this;
    }

    Salix::IDialog& set_default_file_name(const std::string& filename) override {
        config_.fileName = filename;
        return *this;
    }

    Salix::IDialog& set_callback(Salix::FileDialogCallback callback) override {
        callback_ = std::move(callback);
        return *this;
    }

    Salix::IDialog& set_overwrite(bool overwrite) override {
        overwrite_ = overwrite;
        return *this;
    }

    void open() override { ++open_calls; }

    bool overwrite_enabled() const { return overwrite_; }

    int open_calls = 0;

private:
    std::string key_;
    std::string title_;
    Salix::DialogType type_;
    std::string filters_;
    Salix::DialogConfig config_;
    Salix::FileDialogCallback callback_;
    bool overwrite_ = false;
};
