#pragma once

#include <Salix/gui/IThemeData.h>
#include <string>
#include <utility>

class MockIThemeData : public Salix::IThemeData {
public:
    MockIThemeData(std::string name = "MockThemeData", std::string gui_type = "MockGui")
        : name_(std::move(name)), gui_type_(std::move(gui_type)) {}

    const std::string& get_name() const override { return name_; }
    const std::string& get_gui_type() const override { return gui_type_; }

    void set_name(const std::string& name) { name_ = name; }
    void set_gui_type(const std::string& gui_type) { gui_type_ = gui_type; }

private:
    std::string name_;
    std::string gui_type_;
};
