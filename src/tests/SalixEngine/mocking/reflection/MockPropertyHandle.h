#pragma once

#include <Salix/reflection/PropertyHandle.h>
#include <utility>

class MockPropertyHandle : public Salix::PropertyHandle {
public:
    explicit MockPropertyHandle(
        const Salix::Property& property,
        Salix::PropertyValue initial_value = 0)
        : Salix::PropertyHandle(property), value_(std::move(initial_value)) {}

    Salix::PropertyValue get_value() const override {
        ++get_calls;
        return value_;
    }

    void set_value(const Salix::PropertyValue& value) override {
        ++set_calls;
        value_ = value;
    }

    mutable int get_calls = 0;
    int set_calls = 0;

private:
    Salix::PropertyValue value_;
};
