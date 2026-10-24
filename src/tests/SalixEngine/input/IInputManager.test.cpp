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
