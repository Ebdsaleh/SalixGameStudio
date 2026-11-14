#include <doctest.h>
#include <Salix/input/IInputManager.h>
#include <Tests/SalixEngine/mocking/input/MockIInputManager.h>
#include <Tests/SalixEngine/mocking/events/MockEventSystem.h>
#include <memory>
#include <vector>

TEST_SUITE("Salix::input::IInputManagerBatch1347") {
    TEST_CASE("input manager update forwards delta time through interface") {
        MockIInputManager manager;
        Salix::IInputManager* interface_ptr = &manager;
        interface_ptr->update(0.125f);
        CHECK(manager.update_calls == 1);
        CHECK(manager.last_delta_time == doctest::Approx(0.125f));
    }

    TEST_CASE("input manager process event retains exact event identity") {
        MockIInputManager manager;
        Salix::MockEvent event;
        Salix::IInputManager* interface_ptr = &manager;
        interface_ptr->process_event(event);
        CHECK(manager.process_event_calls == 1);
        CHECK(manager.last_event == &event);
    }

    TEST_CASE("input manager key state distinguishes down held released and up") {
        MockIInputManager manager;
        manager.set_key_state(Salix::KeyCode::A, Salix::InputState::Down);
        manager.set_key_state(Salix::KeyCode::B, Salix::InputState::Held);
        manager.set_key_state(Salix::KeyCode::C, Salix::InputState::Released);
        CHECK(manager.is_down(Salix::KeyCode::A));
        CHECK(manager.is_held_down(Salix::KeyCode::B));
        CHECK(manager.was_released(Salix::KeyCode::C));
        CHECK(manager.is_up(Salix::KeyCode::D));
    }

    TEST_CASE("input manager key held duration uses configured elapsed seconds") {
        MockIInputManager manager;
        manager.set_key_state(Salix::KeyCode::Space, Salix::InputState::Held, 2.5f);
        CHECK(manager.is_held_down_for(Salix::KeyCode::Space, 2.0f));
        CHECK(manager.is_held_down_for(Salix::KeyCode::Space, 2.5f));
        CHECK_FALSE(manager.is_held_down_for(Salix::KeyCode::Space, 3.0f));
    }

    TEST_CASE("input manager key combo requires every key to match requested state") {
        MockIInputManager manager;
        manager.set_key_state(Salix::KeyCode::LeftControl, Salix::InputState::Down);
        manager.set_key_state(Salix::KeyCode::S, Salix::InputState::Down);
        CHECK(manager.multiple_are_down({Salix::KeyCode::LeftControl, Salix::KeyCode::S}));
        manager.set_key_state(Salix::KeyCode::S, Salix::InputState::Held);
        CHECK_FALSE(manager.multiple_are_down({Salix::KeyCode::LeftControl, Salix::KeyCode::S}));
    }

    TEST_CASE("input manager any released key differs from all released combo") {
        MockIInputManager manager;
        manager.set_key_state(Salix::KeyCode::A, Salix::InputState::Released);
        manager.set_key_state(Salix::KeyCode::B, Salix::InputState::Held);
        const std::vector<Salix::KeyCode> keys{Salix::KeyCode::A, Salix::KeyCode::B};
        CHECK(manager.any_of_combo_was_released(keys));
        CHECK_FALSE(manager.multiple_were_released(keys));
    }

    TEST_CASE("input manager mouse button contracts mirror keyboard state semantics") {
        MockIInputManager manager;
        manager.set_mouse_state(Salix::MouseButton::Left, Salix::InputState::Held, 1.5f);
        CHECK(manager.is_held_down(Salix::MouseButton::Left));
        CHECK(manager.is_held_down_for(Salix::MouseButton::Left, 1.0f));
        CHECK_FALSE(manager.is_down(Salix::MouseButton::Left));
        CHECK(manager.is_up(Salix::MouseButton::Right));
    }

    TEST_CASE("input manager mouse combo reports all-up and all-down independently") {
        MockIInputManager manager;
        CHECK(manager.multiple_are_up({Salix::MouseButton::Left, Salix::MouseButton::Right}));
        manager.set_mouse_state(Salix::MouseButton::Left, Salix::InputState::Down);
        manager.set_mouse_state(Salix::MouseButton::Right, Salix::InputState::Down);
        CHECK(manager.multiple_are_down({Salix::MouseButton::Left, Salix::MouseButton::Right}));
        CHECK_FALSE(manager.multiple_are_up({Salix::MouseButton::Left, Salix::MouseButton::Right}));
    }

    TEST_CASE("input manager transports mouse position scroll and quit state") {
        MockIInputManager manager;
        manager.mouse_x = 320;
        manager.mouse_y = 180;
        manager.scroll_delta = 2.0f;
        manager.scroll_direction = Salix::MouseScroll::Forward;
        manager.quit_requested = true;
        int x = 0;
        int y = 0;
        manager.get_mouse_position(&x, &y);
        CHECK(x == 320);
        CHECK(y == 180);
        CHECK(manager.get_mouse_scroll_delta() == doctest::Approx(2.0f));
        CHECK(manager.did_scroll(Salix::MouseScroll::Forward));
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Backward));
        CHECK(manager.wants_to_quit());
    }

    TEST_CASE("input manager supports polymorphic destruction") {
        std::unique_ptr<Salix::IInputManager> manager = std::make_unique<MockIInputManager>();
        REQUIRE(manager != nullptr);
        CHECK(manager->is_up(Salix::KeyCode::Escape));
        CHECK(manager->is_up(Salix::MouseButton::Middle));
    }
}

TEST_SUITE("Salix::input::IInputManagerBatch6647") {
    TEST_CASE("batch6647 update accepts zero delta and records one call") {
        MockIInputManager manager;
        manager.update(0.0f);
        CHECK(manager.update_calls == 1);
        CHECK(manager.last_delta_time == doctest::Approx(0.0f));
    }

    TEST_CASE("batch6647 update accepts negative delta and keeps latest value") {
        MockIInputManager manager;
        manager.update(1.0f);
        manager.update(-6.647f);
        CHECK(manager.update_calls == 2);
        CHECK(manager.last_delta_time == doctest::Approx(-6.647f));
    }

    TEST_CASE("batch6647 process event tracks latest exact event pointer") {
        MockIInputManager manager;
        Salix::MockEvent first;
        Salix::MockEvent second;
        manager.process_event(first);
        manager.process_event(second);
        CHECK(manager.process_event_calls == 2);
        CHECK(manager.last_event == &second);
    }

    TEST_CASE("batch6647 empty key combos satisfy all-state predicates") {
        MockIInputManager manager;
        const std::vector<Salix::KeyCode> keys;
        CHECK(manager.multiple_are_down(keys));
        CHECK(manager.multiple_are_held_down(keys));
        CHECK(manager.multiple_are_held_down_for(keys, 6647.0f));
        CHECK(manager.multiple_were_released(keys));
        CHECK(manager.multiple_are_up(keys));
        CHECK_FALSE(manager.any_of_combo_was_released(keys));
    }

    TEST_CASE("batch6647 empty mouse combos satisfy all-state predicates") {
        MockIInputManager manager;
        const std::vector<Salix::MouseButton> buttons;
        CHECK(manager.multiple_are_down(buttons));
        CHECK(manager.multiple_are_held_down(buttons));
        CHECK(manager.multiple_are_held_down_for(buttons, 6647.0f));
        CHECK(manager.multiple_were_released(buttons));
        CHECK(manager.multiple_are_up(buttons));
        CHECK_FALSE(manager.any_of_combo_was_released(buttons));
    }

    TEST_CASE("batch6647 held key duration exact boundary remains true") {
        MockIInputManager manager;
        manager.set_key_state(Salix::KeyCode::A, Salix::InputState::Held, 6.647f);
        CHECK(manager.is_held_down_for(Salix::KeyCode::A, 6.647f));
        CHECK_FALSE(manager.is_held_down_for(Salix::KeyCode::A, 6.648f));
    }

    TEST_CASE("batch6647 held mouse duration exact boundary remains true") {
        MockIInputManager manager;
        manager.set_mouse_state(Salix::MouseButton::Left, Salix::InputState::Held, 6.647f);
        CHECK(manager.is_held_down_for(Salix::MouseButton::Left, 6.647f));
        CHECK_FALSE(manager.is_held_down_for(Salix::MouseButton::Left, 6.648f));
    }

    TEST_CASE("batch6647 mouse position getter tolerates null output pointers") {
        MockIInputManager manager;
        manager.mouse_x = 66;
        manager.mouse_y = -47;
        CHECK_NOTHROW(manager.get_mouse_position(nullptr, nullptr));
        int x = 0;
        manager.get_mouse_position(&x, nullptr);
        CHECK(x == 66);
    }

    TEST_CASE("batch6647 scroll requires matching direction and nonzero delta") {
        MockIInputManager manager;
        manager.scroll_direction = Salix::MouseScroll::Forward;
        manager.scroll_delta = 0.0f;
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Forward));
        manager.scroll_delta = -6.647f;
        CHECK(manager.did_scroll(Salix::MouseScroll::Forward));
        CHECK_FALSE(manager.did_scroll(Salix::MouseScroll::Backward));
    }

    TEST_CASE("batch6647 quit state is independent from input state mutation") {
        MockIInputManager manager;
        manager.quit_requested = true;
        manager.set_key_state(Salix::KeyCode::Escape, Salix::InputState::Down);
        manager.set_mouse_state(Salix::MouseButton::Right, Salix::InputState::Released);
        CHECK(manager.wants_to_quit());
        CHECK(manager.is_down(Salix::KeyCode::Escape));
        CHECK(manager.was_released(Salix::MouseButton::Right));
    }
}
