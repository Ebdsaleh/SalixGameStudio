#pragma once

#include <Salix/gui/IFontData.h>
#include <string>
#include <utility>

class MockIFontData : public Salix::IFontData {
public:
    MockIFontData(
        std::string name = "MockFont",
        std::string file_path = "mock.ttf",
        float size = 16.0f,
        std::string gui_type = "MockGui")
        : name_(std::move(name)),
          file_path_(std::move(file_path)),
          size_(size),
          gui_type_(std::move(gui_type)) {}

    const std::string& get_name() const override { return name_; }
    const std::string& get_file_path() const override { return file_path_; }
    float get_font_size() const override { return size_; }
    const std::string& get_gui_type() const override { return gui_type_; }
    void set_font_size(float font_size) override { size_ = font_size; }

    void set_name(const std::string& name) { name_ = name; }
    void set_file_path(const std::string& file_path) { file_path_ = file_path; }
    void set_gui_type(const std::string& gui_type) { gui_type_ = gui_type; }

private:
    std::string name_;
    std::string file_path_;
    float size_;
    std::string gui_type_;
};
