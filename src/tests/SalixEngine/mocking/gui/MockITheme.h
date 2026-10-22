#pragma once

#include <Salix/gui/ITheme.h>
#include <Salix/gui/IGui.h>
#include <string>

class MockITheme : public Salix::ITheme {
public:
    MockITheme(std::string name = "MockTheme", std::string file_path = "mock.theme")
        : name_(std::move(name)), file_path_(std::move(file_path)) {}

    const std::string& get_name() const override { return name_; }
    const std::string& get_file_path() const override { return file_path_; }

    void apply(Salix::IGui* gui_system) override {
        ++apply_count;
        last_gui = gui_system;
    }

    bool load_from_yaml(const std::string& file_path) override {
        ++load_count;
        last_load_path = file_path;
        if (load_result) {
            file_path_ = file_path;
        }
        return load_result;
    }

    bool save_to_yaml(const std::string& file_path) const override {
        ++save_count;
        last_save_path = file_path;
        return save_result;
    }

    int apply_count = 0;
    int load_count = 0;
    mutable int save_count = 0;
    Salix::IGui* last_gui = nullptr;
    std::string last_load_path;
    mutable std::string last_save_path;
    bool load_result = true;
    mutable bool save_result = true;

private:
    std::string name_;
    std::string file_path_;
};
