#include <doctest.h>
#include <Salix/input/IInput.h>
#include <Tests/SalixEngine/mocking/input/MockIInput.h>
#include <memory>
#include <vector>

TEST_SUITE("Salix::input::IInputBatch1347") {
    TEST_CASE("input interface single down query dispatches requested input type") {
        MockIInput input;
        input.down_result = true;
        Salix::IInput* interface_ptr = &input;
        CHECK(interface_ptr->is_down(Salix::InputType::Keycode));
        CHECK(input.last_single_input == Salix::InputType::Keycode);
    }

    TEST_CASE("input interface held query is independent from down query") {
        MockIInput input;
        input.down_result = false;
        input.held_result = true;
        Salix::IInput* interface_ptr = &input;
        CHECK_FALSE(interface_ptr->is_down(Salix::InputType::Button));
        CHECK(interface_ptr->is_held_down(Salix::InputType::Button));
        CHECK(input.last_single_input == Salix::InputType::Button);
    }

    TEST_CASE("input interface held duration forwards exact duration") {
        MockIInput input;
        input.held_for_result = true;
        Salix::IInput* interface_ptr = &input;
        CHECK(interface_ptr->is_held_down_for(Salix::InputType::Axis, 3));
        CHECK(input.last_single_input == Salix::InputType::Axis);
        CHECK(input.last_duration == 3);
    }

    TEST_CASE("input interface distinguishes just released from released") {
        MockIInput input;
        input.just_released_result = true;
        input.released_result = false;
        Salix::IInput* interface_ptr = &input;
        CHECK(interface_ptr->was_just_released(Salix::InputType::Point));
        CHECK_FALSE(interface_ptr->was_released(Salix::InputType::Point));
    }

    TEST_CASE("input interface up state can be configured independently") {
        MockIInput input;
        input.up_result = false;
        Salix::IInput* interface_ptr = &input;
        CHECK_FALSE(interface_ptr->is_up(Salix::InputType::Keycode));
        input.up_result = true;
        CHECK(interface_ptr->is_up(Salix::InputType::Keycode));
    }

    TEST_CASE("input interface multiple down forwards complete input vector") {
        MockIInput input;
        input.multiple_down_result = true;
        Salix::IInput* interface_ptr = &input;
        CHECK(interface_ptr->multiple_are_down({Salix::InputType::Keycode, Salix::InputType::Button}));
        REQUIRE(input.last_multiple_inputs.size() == 2);
        CHECK(input.last_multiple_inputs[0] == Salix::InputType::Keycode);
        CHECK(input.last_multiple_inputs[1] == Salix::InputType::Button);
    }

    TEST_CASE("input interface multiple held duration forwards vector and duration") {
        MockIInput input;
        input.multiple_held_for_result = true;
        Salix::IInput* interface_ptr = &input;
        CHECK(interface_ptr->multiple_are_held_down_for(
            {Salix::InputType::Button, Salix::InputType::Axis}, 5));
        CHECK(input.last_duration == 5);
        REQUIRE(input.last_multiple_inputs.size() == 2);
    }

    TEST_CASE("input interface multiple release contracts remain independently configurable") {
        MockIInput input;
        input.multiple_just_released_result = true;
        input.multiple_released_result = false;
        Salix::IInput* interface_ptr = &input;
        CHECK(interface_ptr->multiple_were_just_released({Salix::InputType::Keycode}));
        CHECK_FALSE(interface_ptr->multiple_were_released({Salix::InputType::Keycode}));
    }

    TEST_CASE("input interface multiple up query preserves supplied ordering") {
        MockIInput input;
        input.multiple_up_result = true;
        Salix::IInput* interface_ptr = &input;
        CHECK(interface_ptr->multiple_are_up({
            Salix::InputType::Point, Salix::InputType::Axis, Salix::InputType::Button}));
        REQUIRE(input.last_multiple_inputs.size() == 3);
        CHECK(input.last_multiple_inputs[0] == Salix::InputType::Point);
        CHECK(input.last_multiple_inputs[2] == Salix::InputType::Button);
    }

    TEST_CASE("input interface supports polymorphic destruction") {
        std::unique_ptr<Salix::IInput> input = std::make_unique<MockIInput>();
        REQUIRE(input != nullptr);
        CHECK(input->is_up(Salix::InputType::Keycode));
    }
}
