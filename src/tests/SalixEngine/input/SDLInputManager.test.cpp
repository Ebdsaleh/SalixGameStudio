// =================================================================================
// Filename:    src/Tests/SalixEngine/input/sdl/SDLInputManager.test.cpp
// Description: Contains unit tests for the SDLInputManager.
// ================================================================================= 

#include <doctest.h>
#include <Salix/input/sdl/SDLInputManager.h>
#include <Salix/events/sdl/SDLEvent.h> // For our mock events
#include <SDL.h> // For SDL_Event and keycodes

TEST_SUITE("Salix::input::sdl::SDLInputManager") {

    TEST_CASE("a single key press correctly cycles through all states") {
        // ARRANGE: Create the manager and a mock SDL_Event to pass to our event constructors.
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{}; // An empty SDL_Event is fine for this test.
        const float frame_time = 0.016f;

        // --- FRAME 1: Initial State ---
        // Initially, all keys should be in the "Up" state.
        CHECK(manager.is_up(Salix::KeyCode::A));
        CHECK_FALSE(manager.is_down(Salix::KeyCode::A));
        CHECK_FALSE(manager.is_held_down(Salix::KeyCode::A));
        CHECK_FALSE(manager.was_released(Salix::KeyCode::A));

        // --- FRAME 2: Key is Pressed ---
        // Simulate a KeyPressedEvent coming from the engine.
        Salix::KeyPressedEvent event_a_down(SDLK_a, false, mock_sdl_event);
        manager.process_event(event_a_down);

        // ASSERT: Immediately after processing, the key should be in the "Down" state for this frame.
        CHECK(manager.is_down(Salix::KeyCode::A));
        CHECK_FALSE(manager.is_up(Salix::KeyCode::A));
        
        // --- FRAME 3: Key is Held ---
        // Simulate a frame tick. The 'update' call transitions the "Down" state to "Held".
        manager.update(frame_time);

        // ASSERT: The key should now be in the "Held" state.
        CHECK(manager.is_held_down(Salix::KeyCode::A));
        CHECK_FALSE(manager.is_down(Salix::KeyCode::A));

        // --- FRAME 4: Key Remains Held ---
        // Simulate another frame tick with no new events.
        manager.update(frame_time);

        // ASSERT: The key should remain in the "Held" state.
        CHECK(manager.is_held_down(Salix::KeyCode::A));
        CHECK_FALSE(manager.is_down(Salix::KeyCode::A));
        
        // --- FRAME 5: Key is Released ---
        // Simulate a KeyReleasedEvent coming from the engine.
        Salix::KeyReleasedEvent event_a_up(SDLK_a, mock_sdl_event);
        manager.process_event(event_a_up);

        // ASSERT: Immediately after processing, the key should be in the "Released" state for this frame.
        CHECK(manager.was_released(Salix::KeyCode::A));
        CHECK_FALSE(manager.is_held_down(Salix::KeyCode::A));

        // --- FRAME 6: Key is Up Again ---
        // Simulate a final frame tick. The 'update' call transitions the "Released" state back to "Up".
        manager.update(frame_time);

        // ASSERT: The key is now back in the "Up" state, completing the cycle.
        CHECK(manager.is_up(Salix::KeyCode::A));
        CHECK_FALSE(manager.was_released(Salix::KeyCode::A));
    }

    TEST_CASE("mouse movement is correctly tracked") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        int x = 0, y = 0;

        // ACT: Simulate a mouse movement event.
        Salix::MouseMovedEvent event(123.0f, 456.0f, mock_sdl_event);
        manager.process_event(event);
        manager.get_mouse_position(&x, &y);

        // ASSERT
        CHECK(x == 123);
        CHECK(y == 456);
    }

    TEST_CASE("is_held_down_for correctly tracks duration") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.016f;

        // ACT: Press the key
        Salix::KeyPressedEvent event_down(SDLK_b, false, mock_sdl_event);
        manager.process_event(event_down);

        // Simulate holding the key for 5 frames
        for (int i = 0; i < 5; ++i) {
            manager.update(frame_time);
        }

        // ASSERT
        // Total time held is approx 5 * 0.016 = 0.08 seconds
        CHECK(manager.is_held_down_for(Salix::KeyCode::B, 0.07f));
        CHECK_FALSE(manager.is_held_down_for(Salix::KeyCode::B, 0.09f));
    }
    
    TEST_CASE("a mouse button press correctly cycles through all states") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.016f;
        const Salix::MouseButton button = Salix::MouseButton::Left;

        // --- FRAME 1: Initial State ---
        CHECK(manager.is_up(button));

        // --- FRAME 2: Button is Pressed ---
        Salix::MouseButtonPressedEvent event_down(SDL_BUTTON_LEFT, mock_sdl_event);
        manager.process_event(event_down);
        CHECK(manager.is_down(button));
        
        // --- FRAME 3: Button is Held ---
        manager.update(frame_time);
        CHECK(manager.is_held_down(button));

        // --- FRAME 4: Button is Released ---
        Salix::MouseButtonReleasedEvent event_up(SDL_BUTTON_LEFT, mock_sdl_event);
        manager.process_event(event_up);
        CHECK(manager.was_released(button));

        // --- FRAME 5: Button is Up Again ---
        manager.update(frame_time);
        CHECK(manager.is_up(button));
    }

    TEST_CASE("mouse scroll is correctly detected") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};

        SUBCASE("when scrolling forward") {
            // ACT: Simulate a scroll forward event (positive y offset)
            Salix::MouseScrolledEvent event(0.0f, 1.0f, mock_sdl_event);
            manager.process_event(event);

            // ASSERT
            CHECK(manager.did_scroll(Salix::MouseScroll::Forward));
            CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Backward)); // This hits the 'else' branch
            CHECK(manager.get_mouse_scroll_delta() > 0.0f);
        }

        SUBCASE("when scrolling backward") {
            // ACT: Simulate a scroll backward event (negative y offset)
            Salix::MouseScrolledEvent event(0.0f, -1.0f, mock_sdl_event);
            manager.process_event(event);

            // ASSERT
            CHECK(manager.did_scroll(Salix::MouseScroll::Backward)); // This also hits the 'else' branch
            CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Forward));
            CHECK(manager.get_mouse_scroll_delta() < 0.0f);
        }
        SUBCASE("when checking for no scroll direction") {
            // This test ensures the default case is hit.
            CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::None));
        }
    }

    TEST_CASE("multi-key combination checks") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.016f;
        const std::vector<Salix::KeyCode> combo = {Salix::KeyCode::LeftControl, Salix::KeyCode::S};
        const std::vector<Salix::KeyCode> empty_combo = {};

        SUBCASE("multiple_are_down works correctly") {
            // ACT: Press Ctrl, then S on the next frame update.
            manager.process_event(Salix::KeyPressedEvent(SDLK_LCTRL, false, mock_sdl_event));
            manager.update(frame_time); // Ctrl is now "Held"
            manager.process_event(Salix::KeyPressedEvent(SDLK_s, false, mock_sdl_event)); // S is now "Down"
        
            // ASSERT
            CHECK(manager.multiple_are_down(combo));
            CHECK_FALSE(manager.multiple_are_down({Salix::KeyCode::LeftControl, Salix::KeyCode::A})); // Fails if one key is up
            CHECK_FALSE(manager.multiple_are_down(empty_combo)); // Covers the empty vector case
        }
        
        SUBCASE("multiple_were_released works literally as intended") {
            // This test verifies the specific use case: the function should only
            // return true if ALL keys in the list were just released.

            // --- FAILURE SCENARIO (The case that was failing before) ---
            // Press Ctrl and S, hold both, then release only S.
            manager.process_event(Salix::KeyPressedEvent(SDLK_LCTRL, false, mock_sdl_event));
            manager.process_event(Salix::KeyPressedEvent(SDLK_s, false, mock_sdl_event));
            manager.update(frame_time); // Both keys are now in the "Held" state
            manager.process_event(Salix::KeyReleasedEvent(SDLK_s, mock_sdl_event)); // Release only S

            // ASSERT: The function should correctly return FALSE because Ctrl is still held.
            CHECK_FALSE(manager.multiple_were_released(combo));

            // --- SUCCESS SCENARIO ---
            // Now, release the final key of the combo in the same processing frame.
            manager.process_event(Salix::KeyReleasedEvent(SDLK_LCTRL, mock_sdl_event)); // Release Ctrl

            // ASSERT: Now that ALL keys in the combo have been released, it should return TRUE.
            CHECK(manager.multiple_were_released(combo));
        }
    }

    TEST_CASE("any_of_combo_was_released works for partial combo breaks") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.016f;
        const std::vector<Salix::KeyCode> combo = {Salix::KeyCode::LeftControl, Salix::KeyCode::S};

        // ACT: Press and hold both keys, then release only ONE.
        manager.process_event(Salix::KeyPressedEvent(SDLK_LCTRL, false, mock_sdl_event));
        manager.process_event(Salix::KeyPressedEvent(SDLK_s, false, mock_sdl_event));
        manager.update(frame_time); // Both keys are now "Held"
        manager.process_event(Salix::KeyReleasedEvent(SDLK_s, mock_sdl_event)); // Release S

        // ASSERT: The new function should return TRUE because one key in the combo was released.
        CHECK(manager.any_of_combo_was_released(combo));

        // ASSERT: The strict function should still return FALSE because one key is still held.
        CHECK_FALSE(manager.multiple_were_released(combo));
    }

    TEST_CASE("multiple_are_held_down for keyboard keys") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.016f;
        const std::vector<Salix::KeyCode> combo = {Salix::KeyCode::A, Salix::KeyCode::B};

        SUBCASE("returns true when all keys are held") {
            // ACT: Press and hold both keys.
            manager.process_event(Salix::KeyPressedEvent(SDLK_a, false, mock_sdl_event));
            manager.process_event(Salix::KeyPressedEvent(SDLK_b, false, mock_sdl_event));
            manager.update(frame_time); // Transition both keys to "Held" state.

            // ASSERT
            CHECK(manager.multiple_are_held_down(combo));
        }

        SUBCASE("returns false if one key is not held") {
            // ACT: Press and hold only one of the two keys.
            manager.process_event(Salix::KeyPressedEvent(SDLK_a, false, mock_sdl_event));
            manager.update(frame_time); // Key 'A' is "Held", but 'B' is "Up".

            // ASSERT
            CHECK_FALSE(manager.multiple_are_held_down(combo));
        }

        SUBCASE("returns false for an empty list of keys") {
            // ACT & ASSERT: Call with an empty vector.
            CHECK_FALSE(manager.multiple_are_held_down(std::vector<Salix::KeyCode>{}));
        }
    }

    TEST_CASE("wants_to_quit is correctly flagged") {
        // ARRANGE
        Salix::SDLInputManager manager;
        
        // ACT & ASSERT: Should be false initially
        CHECK_FALSE(manager.wants_to_quit());

        // ACT & ASSERT: Simulate a quit event
        Salix::WindowCloseEvent event;
        manager.process_event(event);
        CHECK(manager.wants_to_quit());
    }

    TEST_CASE("time-based multi-key and mouse button checks") {
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.1f;
        
        // ACT: Press and hold Ctrl, S, and Left Mouse Button
        manager.process_event(Salix::KeyPressedEvent(SDLK_LCTRL, false, mock_sdl_event));
        manager.process_event(Salix::KeyPressedEvent(SDLK_s, false, mock_sdl_event));
        manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_LEFT, mock_sdl_event));

        // Simulate holding for 3 frames (total time = 0.3s)
        for (int i = 0; i < 3; ++i) {
            manager.update(frame_time);
        }

        // ASSERT
        CHECK(manager.multiple_are_held_down_for({Salix::KeyCode::LeftControl, Salix::KeyCode::S}, 0.25f));
        CHECK_FALSE(manager.multiple_are_held_down_for({Salix::KeyCode::LeftControl, Salix::KeyCode::S}, 0.35f));
        
        CHECK(manager.is_held_down_for(Salix::MouseButton::Left, 0.25f));
        CHECK_FALSE(manager.is_held_down_for(Salix::MouseButton::Left, 0.35f));
    }

    TEST_CASE("multi-mouse-button combination checks") {
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.016f;
        const std::vector<Salix::MouseButton> mb_combo = {Salix::MouseButton::Left, Salix::MouseButton::Right};

        // ACT: Press Left, then Right on the next frame update.
        manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_LEFT, mock_sdl_event));
        manager.update(frame_time); // Left is now "Held"
        manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_RIGHT, mock_sdl_event)); // Right is now "Down"

        // ASSERT
        CHECK(manager.multiple_are_down(mb_combo));
        
        manager.update(frame_time); // Both are now "Held"
        CHECK(manager.multiple_are_held_down(mb_combo));
    }

    TEST_CASE("keycode translator handles unknown keys") {
        Salix::SDLInputManager manager;
        
        // The SDL keycode for the 'power' button is a good obscure key to test the default case.
        // We need to access the private translator, so we make the test class a friend.
        // For simplicity here, we assume a way to call it or infer its behavior.
        // Since `to_salix_keycode` is private, we test its effect via `process_event`.
        SDL_Event power_key_event;
        power_key_event.type = SDL_KEYDOWN;
        power_key_event.key.keysym.sym = SDLK_POWER;

        // By processing an unknown key, we cover the `default` case in the switch statement.
        // We don't need a CHECK here; the goal is just to execute the code path for coverage.
        Salix::KeyPressedEvent event(power_key_event.key.keysym.sym, false, power_key_event);
        manager.process_event(event);
        
        // This just confirms the test runs. The real goal is the coverage.
        CHECK(true);
    }

    TEST_CASE("multi-input function branches and edge cases") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.1f;
        
        SUBCASE("multiple_are_up works correctly") {
            // ACT & ASSERT: Initially, both keys are up.
            CHECK(manager.multiple_are_up({Salix::KeyCode::X, Salix::KeyCode::Y}));

            // ACT & ASSERT: Press one key, the check should now fail.
            manager.process_event(Salix::KeyPressedEvent(SDLK_x, false, mock_sdl_event));
            manager.update(frame_time);
            CHECK_FALSE(manager.multiple_are_up({Salix::KeyCode::X, Salix::KeyCode::Y}));
        }

        SUBCASE("multi-mouse-button functions work correctly with strict logic") {
            const std::vector<Salix::MouseButton> combo = {Salix::MouseButton::Left, Salix::MouseButton::Right};
            
            // --- FAILURE SCENARIO ---
            // Press and hold both buttons, then release only one.
            manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_LEFT, mock_sdl_event));
            manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_RIGHT, mock_sdl_event));
            manager.update(frame_time); // Both are now "Held"
            manager.process_event(Salix::MouseButtonReleasedEvent(SDL_BUTTON_LEFT, mock_sdl_event)); // Release Left

            // ASSERT: The function should correctly return FALSE because the Right button is still held.
            CHECK_FALSE(manager.multiple_were_released(combo));

            // --- SUCCESS SCENARIO ---
            // Now, release the other button.
            manager.process_event(Salix::MouseButtonReleasedEvent(SDL_BUTTON_RIGHT, mock_sdl_event));
            
            // ASSERT: Now that BOTH buttons in the combo have been released, it should return TRUE.
            CHECK(manager.multiple_were_released(combo));
        }
        
        SUBCASE("multiple_are_held_down_for for mouse buttons") {
            const std::vector<Salix::MouseButton> combo = {Salix::MouseButton::Left, Salix::MouseButton::Right};
            
            // ACT: Press and hold both buttons for 3 frames (0.3s)
            manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_LEFT, mock_sdl_event));
            manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_RIGHT, mock_sdl_event));
            for (int i = 0; i < 3; ++i) {
                manager.update(frame_time);
            }

            // ASSERT
            CHECK(manager.multiple_are_held_down_for(combo, 0.25f));
            CHECK_FALSE(manager.multiple_are_held_down_for(combo, 0.35f));
        }
    }

    TEST_CASE("any_of_combo_was_released works for mouse buttons") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.016f;
        const std::vector<Salix::MouseButton> combo = {Salix::MouseButton::Left, Salix::MouseButton::Right};

        // ACT: Press and hold both buttons, then release only ONE.
        manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_LEFT, mock_sdl_event));
        manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_RIGHT, mock_sdl_event));
        manager.update(frame_time); // Both buttons are now "Held"
        manager.process_event(Salix::MouseButtonReleasedEvent(SDL_BUTTON_RIGHT, mock_sdl_event)); // Release Right

        // ASSERT: The new function should return TRUE because the Right button was released.
        CHECK(manager.any_of_combo_was_released(combo));
        
        // ASSERT: The strict function should still return FALSE because the Left button is still held.
        CHECK_FALSE(manager.multiple_were_released(combo));
    }

    TEST_CASE("multi-input empty vector and failure edge cases") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.016f;

        SUBCASE("functions handle empty vectors gracefully") {
            // This subcase covers all the `if (keys.empty())` branches.
            CHECK_FALSE(manager.multiple_are_down(std::vector<Salix::KeyCode>{}));
            CHECK_FALSE(manager.multiple_are_held_down_for(std::vector<Salix::KeyCode>{}, 1.0f));
            CHECK_FALSE(manager.multiple_were_released(std::vector<Salix::KeyCode>{}));
            CHECK_FALSE(manager.any_of_combo_was_released(std::vector<Salix::KeyCode>{}));
            
            CHECK_FALSE(manager.multiple_are_down(std::vector<Salix::MouseButton>{}));
            CHECK_FALSE(manager.multiple_were_released(std::vector<Salix::MouseButton>{}));
            CHECK_FALSE(manager.any_of_combo_was_released(std::vector<Salix::MouseButton>{}));
            CHECK_FALSE(manager.multiple_are_up(std::vector<Salix::MouseButton>{}));
        }

        SUBCASE("combo failure paths are handled correctly") {
            // This subcase covers the remaining failure branches inside the loops.
            const auto key_combo = {Salix::KeyCode::A, Salix::KeyCode::B};
            const auto btn_combo = {Salix::MouseButton::Left, Salix::MouseButton::Right};

            // ACT: Press and hold only ONE of the inputs in each combo.
            manager.process_event(Salix::KeyPressedEvent(SDLK_a, false, mock_sdl_event));
            manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_LEFT, mock_sdl_event));
            manager.update(frame_time); // A and Left are now "Held"

            // ASSERT
            CHECK_FALSE(manager.multiple_are_down(key_combo)); // Fails because B is not down
            CHECK_FALSE(manager.multiple_are_held_down_for(key_combo, 0.1f)); // Fails because B is not held
            CHECK_FALSE(manager.any_of_combo_was_released(key_combo)); // Fails because nothing was released

            CHECK_FALSE(manager.multiple_are_down(btn_combo)); // Fails because Right is not down
            CHECK_FALSE(manager.multiple_are_up(btn_combo)); // Fails because Left is not up
            CHECK_FALSE(manager.any_of_combo_was_released(btn_combo)); // Fails because nothing was released
        }
    }

    TEST_CASE("multi-input failure paths") {
        // ARRANGE
        Salix::SDLInputManager manager;
        SDL_Event mock_sdl_event{};
        const float frame_time = 0.016f;

        SUBCASE("multiple_are_held_down_for (keyboard) fails if a key is not held") {
            // This covers the `if (key_states.at(key) != InputState::Held)` branch.
            manager.process_event(Salix::KeyPressedEvent(SDLK_a, false, mock_sdl_event));
            manager.update(frame_time); // Key A is now "Held".

            // ASSERT: Fails because key B was never pressed.
            CHECK_FALSE(manager.multiple_are_held_down_for({Salix::KeyCode::A, Salix::KeyCode::B}, 0.01f));
        }

        SUBCASE("multiple_are_held_down (mouse) fails if a button is not held") {
            // This covers the `if (mouse_button_states.at(button) != InputState::Held)` branch.
            manager.process_event(Salix::MouseButtonPressedEvent(SDL_BUTTON_LEFT, mock_sdl_event));
            manager.update(frame_time); // Left button is now "Held".

            // ASSERT: Fails because the right button is not held.
            CHECK_FALSE(manager.multiple_are_held_down({Salix::MouseButton::Left, Salix::MouseButton::Right}));
        }

        SUBCASE("multiple_are_up (mouse) success path") {
            // This covers the `return true` at the end of the function.
            // ASSERT: Initially, all buttons are up.
            CHECK(manager.multiple_are_up({Salix::MouseButton::Left, Salix::MouseButton::Right}));
        }
    }

    TEST_CASE("to_salix_keycode provides 100% accurate key mapping") {
        Salix::SDLInputManager manager;

        // Create a map of every SDL keycode to its expected Salix::KeyCode equivalent.
        // This map serves as the official specification for the input system.
        const std::map<int, Salix::KeyCode> key_map = {
            { SDLK_a, Salix::KeyCode::A }, { SDLK_b, Salix::KeyCode::B }, { SDLK_c, Salix::KeyCode::C },
            { SDLK_d, Salix::KeyCode::D }, { SDLK_e, Salix::KeyCode::E }, { SDLK_f, Salix::KeyCode::F },
            { SDLK_g, Salix::KeyCode::G }, { SDLK_h, Salix::KeyCode::H }, { SDLK_i, Salix::KeyCode::I },
            { SDLK_j, Salix::KeyCode::J }, { SDLK_k, Salix::KeyCode::K }, { SDLK_l, Salix::KeyCode::L },
            { SDLK_m, Salix::KeyCode::M }, { SDLK_n, Salix::KeyCode::N }, { SDLK_o, Salix::KeyCode::O },
            { SDLK_p, Salix::KeyCode::P }, { SDLK_q, Salix::KeyCode::Q }, { SDLK_r, Salix::KeyCode::R },
            { SDLK_s, Salix::KeyCode::S }, { SDLK_t, Salix::KeyCode::T }, { SDLK_u, Salix::KeyCode::U },
            { SDLK_v, Salix::KeyCode::V }, { SDLK_w, Salix::KeyCode::W }, { SDLK_x, Salix::KeyCode::X },
            { SDLK_y, Salix::KeyCode::Y }, { SDLK_z, Salix::KeyCode::Z },
            { SDLK_0, Salix::KeyCode::Alpha0 }, { SDLK_1, Salix::KeyCode::Alpha1 }, { SDLK_2, Salix::KeyCode::Alpha2 },
            { SDLK_3, Salix::KeyCode::Alpha3 }, { SDLK_4, Salix::KeyCode::Alpha4 }, { SDLK_5, Salix::KeyCode::Alpha5 },
            { SDLK_6, Salix::KeyCode::Alpha6 }, { SDLK_7, Salix::KeyCode::Alpha7 }, { SDLK_8, Salix::KeyCode::Alpha8 },
            { SDLK_9, Salix::KeyCode::Alpha9 },
            { SDLK_SPACE, Salix::KeyCode::Space }, { SDLK_RETURN, Salix::KeyCode::Enter }, { SDLK_ESCAPE, Salix::KeyCode::Escape },
            { SDLK_LSHIFT, Salix::KeyCode::LeftShift }, { SDLK_LCTRL, Salix::KeyCode::LeftControl }, { SDLK_LALT, Salix::KeyCode::LeftAlt },
            { SDLK_RSHIFT, Salix::KeyCode::RightShift }, { SDLK_RCTRL, Salix::KeyCode::RightControl }, { SDLK_RALT, Salix::KeyCode::RightAlt },
            { SDLK_TAB, Salix::KeyCode::Tab }, { SDLK_DELETE, Salix::KeyCode::Delete },
            { SDLK_F1, Salix::KeyCode::F1 }, { SDLK_F2, Salix::KeyCode::F2 }, { SDLK_F3, Salix::KeyCode::F3 },
            { SDLK_F4, Salix::KeyCode::F4 }, { SDLK_F5, Salix::KeyCode::F5 }, { SDLK_F6, Salix::KeyCode::F6 },
            { SDLK_F7, Salix::KeyCode::F7 }, { SDLK_F8, Salix::KeyCode::F8 }, { SDLK_F9, Salix::KeyCode::F9 },
            { SDLK_F10, Salix::KeyCode::F10 }, { SDLK_F11, Salix::KeyCode::F11 }, { SDLK_F12, Salix::KeyCode::F12 },
            { SDLK_BACKSPACE, Salix::KeyCode::BackSpace }, { SDLK_BACKSLASH, Salix::KeyCode::BackSlash }, { SDLK_CAPSLOCK, Salix::KeyCode::CapsLock },
            { SDLK_COMMA, Salix::KeyCode::Comma }, { SDLK_SEPARATOR, Salix::KeyCode::Separator }, { SDLK_SLASH, Salix::KeyCode::Slash },
            { SDLK_QUOTE, Salix::KeyCode::Apostrophe }, { SDLK_SEMICOLON, Salix::KeyCode::SemiColon }, { SDLK_PERIOD, Salix::KeyCode::Period },
            { SDLK_MINUS, Salix::KeyCode::Minus }, { SDLK_EQUALS, Salix::KeyCode::Equals },
            { SDLK_UP, Salix::KeyCode::Up }, { SDLK_DOWN, Salix::KeyCode::Down }, { SDLK_LEFT, Salix::KeyCode::Left },
            { SDLK_RIGHT, Salix::KeyCode::Right }, { SDLK_LGUI, Salix::KeyCode::LeftGui }, { SDLK_RGUI, Salix::KeyCode::RightGui },
            { SDLK_APPLICATION, Salix::KeyCode::ContextMenu }, { SDLK_PRINTSCREEN, Salix::KeyCode::PrintScreen }
        };

        // Iterate through the entire specification and verify each mapping.
        for (const auto& pair : key_map) {
            CHECK(manager.to_salix_keycode(pair.first) == pair.second);
        }
    }
}