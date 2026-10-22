#pragma once

#include <Salix/gui/IFont.h>
#include <Tests/SalixEngine/mocking/gui/MockIFontData.h>
#include <memory>
#include <string>
#include <utility>

struct ImFont;

class MockIFont : public Salix::IFont {
public:
    MockIFont(
        const std::string& name = "MockFont",
        const std::string& family = "MockFamily",
        float size = 16.0f)
        : data_(std::make_unique<MockIFontData>(name, name + ".ttf", size, "MockGui")),
          family_(family) {}

    const std::string& get_name() const override { return data_->get_name(); }
    Salix::IFontData* get_data() const override { return data_.get(); }
    const std::string& get_family() const override { return family_; }
    void set_family(const std::string& font_family) override { family_ = font_family; }

    void set_data(Salix::IFontData* new_data) override {
        if (!new_data) {
            return;
        }
        data_ = std::make_unique<MockIFontData>(
            new_data->get_name(),
            new_data->get_file_path(),
            new_data->get_font_size(),
            new_data->get_gui_type());
    }

    ImFont* get_imgui_font_ptr() const override { return imgui_font_ptr_; }
    void set_imgui_font_ptr(ImFont* ptr) { imgui_font_ptr_ = ptr; }

private:
    std::unique_ptr<MockIFontData> data_;
    std::string family_;
    ImFont* imgui_font_ptr_ = nullptr;
};
