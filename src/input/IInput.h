// IIinput.h
#pragma once
#include <vector>

// This is an interface that every input-type will be inherited from.
// In my comments I will use k for key, b for button, a for axis, to save line space.

enum class InputDeviceType {
    Keyboard,           // This is for Keyboard-Only.
    KeyboardAndMouse,   // This is for a typical Computer input-setup, Keyboard and Mouse.
    Mouse,              // This is for Mouse-Only.
    TouchScreen,        // This covers smart phones and tablets, drawing tablets, touch-enabled monitors and kioks.
    GamePad,            // This also includes racing wheels and flight yokes, fight-sticks / retro-arcade-inputs.
    Joystick,           // This is for Joystick
    Stylus              // May be covered under TouchScreen.
};

enum class InputType {
    Keycode,            // for keyboard
    Button,             // for mouse buttons, game pad buttons, joystick buttons, stylus buttons.
    Axis,               // for mouse cursor movements, game pad analog sticks, mobile accelerometers.
    Point,              // for mobile's, tablet's, and graphic tablet's touch screens and stylus sensors.
};

class IInput {
    public:
        virtual ~IInput() = default;
       
        // --- SINGLE INPUT CHECKS ---
        // The moment k, b, or a,  down event is triggered.
        virtual bool is_down(InputType input_type) = 0;

        // The k, b, or a, was down last frame and hasn't been released this frame.   
        virtual bool is_held_down(InputType input_type) = 0;

        // k, b, or a, has been held down for the duration in seconds.
        virtual bool is_held_down_for(InputType input_type, int duration) = 0;

        // The moment k, b, or a, was released/up,  after being held down.
        virtual bool was_just_released(InputType input_type) = 0; 

        // The moment k, b, or a, was released/up from down event being triggered last frame.
        // Think of a key-press when someone is typing, or pressing a button to jump in a platformer-game.
        virtual bool was_released(InputType input_type) = 0;

        // is k, b, or, a up/not_triggered this frame.
        // Think of it as an ' if (!is_down) {} check.
        virtual bool is_up(InputType input_type) = 0;

        // --- MULTIPLE INPUT CHECKS ---

        // The moment multiple k, b, or a,  down events are triggered.
        virtual bool multi_are_down(std::vector<InputType> input_type) = 0;

        // The k, b, or a, was down last frame and hasn't been released this frame.
        virtual bool multi_are_held_down(std::vector<InputType> input_type) = 0;

        // k, b, or a, has been held down for the duration in seconds.
        virtual bool mulit_are_held_down_for(std::vector<InputType> input_type, int duration) = 0;

        // The moment k, b, or a, was released/up,  after being held down.
        virtual bool multi_were_just_released(std::vector<InputType> input_type) = 0; 

        // The moment k, b, or a, was released/up from down event being triggered last frame.
        virtual bool multi_were_released(std::vector<InputType> input_type) = 0;

        // is k, b, or, a up/not_triggered this frame.
        virtual bool multi_are_up(std::vector<InputType> input_type) = 0;

};