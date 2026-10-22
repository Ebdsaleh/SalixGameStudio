#pragma once

#include <Salix/input/IInput.h>
#include <vector>
#include <utility>

class MockIInput : public Salix::IInput {
public:
    bool is_down(Salix::InputType input_type) override {
        last_single_input = input_type;
        return down_result;
    }

    bool is_held_down(Salix::InputType input_type) override {
        last_single_input = input_type;
        return held_result;
    }

    bool is_held_down_for(Salix::InputType input_type, int duration) override {
        last_single_input = input_type;
        last_duration = duration;
        return held_for_result;
    }

    bool was_just_released(Salix::InputType input_type) override {
        last_single_input = input_type;
        return just_released_result;
    }

    bool was_released(Salix::InputType input_type) override {
        last_single_input = input_type;
        return released_result;
    }

    bool is_up(Salix::InputType input_type) override {
        last_single_input = input_type;
        return up_result;
    }

    bool multiple_are_down(std::vector<Salix::InputType> input_type) override {
        last_multiple_inputs = std::move(input_type);
        return multiple_down_result;
    }

    bool multiple_are_held_down(std::vector<Salix::InputType> input_type) override {
        last_multiple_inputs = std::move(input_type);
        return multiple_held_result;
    }

    bool multiple_are_held_down_for(std::vector<Salix::InputType> input_type, int duration) override {
        last_multiple_inputs = std::move(input_type);
        last_duration = duration;
        return multiple_held_for_result;
    }

    bool multiple_were_just_released(std::vector<Salix::InputType> input_type) override {
        last_multiple_inputs = std::move(input_type);
        return multiple_just_released_result;
    }

    bool multiple_were_released(std::vector<Salix::InputType> input_type) override {
        last_multiple_inputs = std::move(input_type);
        return multiple_released_result;
    }

    bool multiple_are_up(std::vector<Salix::InputType> input_type) override {
        last_multiple_inputs = std::move(input_type);
        return multiple_up_result;
    }

    Salix::InputType last_single_input = Salix::InputType::Keycode;
    std::vector<Salix::InputType> last_multiple_inputs;
    int last_duration = 0;

    bool down_result = false;
    bool held_result = false;
    bool held_for_result = false;
    bool just_released_result = false;
    bool released_result = false;
    bool up_result = true;
    bool multiple_down_result = false;
    bool multiple_held_result = false;
    bool multiple_held_for_result = false;
    bool multiple_just_released_result = false;
    bool multiple_released_result = false;
    bool multiple_up_result = true;
};
