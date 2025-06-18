// IIinput.h
#pragma once
// This is an interface that every input-type will be inherited from.
// In my comments I will use k for key, b for button, a for axis, to save line space.
class IInput {
    public:
        virtual ~IInput() = default;
       
        // --- SINGLE INPUT CHECKS ---
        // The moment k, b, or a,  down event is triggered.
        virtual bool is_down(void* input) = 0;

        // The k, b, or a, was down last frame and hasn't been released this frame.   
        virtual bool is_held_down(void* input) = 0;

        // k, b, or a, has been held down for the duration in seconds.
        virtual bool is_held_down_for(void* input, int duration) = 0;

        // The moment k, b, or a, was released/up,  after being held down.
        virtual bool was_just_released(void* input) = 0; 

        // The moment k, b, or a, was released/up from down event being triggered last frame.
        // Think of a key-press when someone is typing, or pressing a button to jump in a platformer-game.
        virtual bool was_released(void* input) = 0;

        // is k, b, or, a up/not_triggered this frame.
        // Think of it as an ' if (!is_down) {} check.
        virtual bool is_up(void* input) = 0;

        // --- MULTIPLE INPUT CHECKS ---
        // So far only supports Two input triggers.

        // The moment multiple k, b, or a,  down events are triggered.
        virtual bool multi_are_down(void* input_a, void* input_b) = 0;

        // The k, b, or a, was down last frame and hasn't been released this frame.
        virtual bool multi_are_held_down(void* input_a, void* input_b) = 0;

        // k, b, or a, has been held down for the duration in seconds.
        virtual bool mulit_are_held_down_for(void* input_a, void* input_b, int duration) = 0;

        // The moment k, b, or a, was released/up,  after being held down.
        virtual bool multi_were_just_released(void* input_a, void* input_b) = 0; 

        // The moment k, b, or a, was released/up from down event being triggered last frame.
        virtual bool multi_were_released(void* input_a, void* input_b) = 0;

        // is k, b, or, a up/not_triggered this frame.
        virtual bool multi_are_up(void* input_a, void* input_b) = 0;

};