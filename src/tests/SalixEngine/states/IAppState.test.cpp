#include <doctest.h>
#include <Salix/states/IAppState.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/states/MockIAppState.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <memory>

TEST_SUITE("Salix::states::IAppStateBatch1547") {
    TEST_CASE("app state starts with no lifecycle calls recorded") {
        MockIAppState state;
        CHECK(state.enter_calls == 0);
        CHECK(state.exit_calls == 0);
        CHECK(state.update_calls == 0);
        CHECK(state.render_calls == 0);
    }

    TEST_CASE("app state enter dispatch receives exact initialization context") {
        MockIAppState state;
        Salix::InitContext context{};
        Salix::IAppState* base = &state;
        base->on_enter(context);
        CHECK(state.enter_calls == 1);
        CHECK(state.last_context == &context);
    }

    TEST_CASE("app state exit dispatch remains independently observable") {
        MockIAppState state;
        Salix::IAppState* base = &state;
        base->on_exit();
        base->on_exit();
        CHECK(state.exit_calls == 2);
        CHECK(state.enter_calls == 0);
    }

    TEST_CASE("app state update forwards positive delta time exactly") {
        MockIAppState state;
        Salix::IAppState* base = &state;
        base->update(0.016f);
        CHECK(state.update_calls == 1);
        CHECK(state.last_delta_time == doctest::Approx(0.016f));
    }

    TEST_CASE("app state update forwards zero delta time without substitution") {
        MockIAppState state;
        Salix::IAppState* base = &state;
        base->update(0.0f);
        CHECK(state.last_delta_time == doctest::Approx(0.0f));
    }

    TEST_CASE("app state update forwards negative delta for explicit edge testing") {
        MockIAppState state;
        state.update(-1.0f);
        CHECK(state.update_calls == 1);
        CHECK(state.last_delta_time == doctest::Approx(-1.0f));
    }

    TEST_CASE("app state render forwards exact renderer pointer") {
        MockIAppState state;
        MockIRenderer renderer;
        Salix::IAppState* base = &state;
        base->render(&renderer);
        CHECK(state.render_calls == 1);
        CHECK(state.last_renderer == &renderer);
    }

    TEST_CASE("app state render accepts null renderer in isolated state contract") {
        MockIAppState state;
        Salix::IAppState* base = &state;
        base->render(nullptr);
        CHECK(state.render_calls == 1);
        CHECK(state.last_renderer == nullptr);
    }

    TEST_CASE("app state complete lifecycle preserves independent call accounting") {
        MockIAppState state;
        Salix::InitContext context{};
        MockIRenderer renderer;
        Salix::IAppState* base = &state;
        base->on_enter(context);
        base->update(0.5f);
        base->render(&renderer);
        base->on_exit();
        CHECK(state.enter_calls == 1);
        CHECK(state.update_calls == 1);
        CHECK(state.render_calls == 1);
        CHECK(state.exit_calls == 1);
        CHECK(state.last_context == &context);
        CHECK(state.last_renderer == &renderer);
    }

    TEST_CASE("app state interface supports polymorphic destruction") {
        std::unique_ptr<Salix::IAppState> state = std::make_unique<MockIAppState>();
        REQUIRE(state != nullptr);
        state->update(1.0f);
        state->render(nullptr);
    }
}

TEST_SUITE("Salix::states::IAppStateBatch6647") {
    TEST_CASE("batch6647 repeated enter calls keep latest context pointer") {
        MockIAppState state;
        Salix::InitContext first{};
        Salix::InitContext second{};
        state.on_enter(first);
        state.on_enter(second);
        CHECK(state.enter_calls == 2);
        CHECK(state.last_context == &second);
    }

    TEST_CASE("batch6647 repeated updates keep latest delta and call count") {
        MockIAppState state;
        state.update(1.0f);
        state.update(-6.647f);
        CHECK(state.update_calls == 2);
        CHECK(state.last_delta_time == doctest::Approx(-6.647f));
    }

    TEST_CASE("batch6647 repeated renders keep latest renderer pointer") {
        MockIAppState state;
        MockIRenderer first;
        MockIRenderer second;
        state.render(&first);
        state.render(&second);
        CHECK(state.render_calls == 2);
        CHECK(state.last_renderer == &second);
    }

    TEST_CASE("batch6647 renderer may transition from nonnull to null") {
        MockIAppState state;
        MockIRenderer renderer;
        state.render(&renderer);
        REQUIRE(state.last_renderer == &renderer);
        state.render(nullptr);
        CHECK(state.last_renderer == nullptr);
        CHECK(state.render_calls == 2);
    }

    TEST_CASE("batch6647 null render before enter does not create context pointer") {
        MockIAppState state;
        state.render(nullptr);
        CHECK(state.last_context == nullptr);
        CHECK(state.last_renderer == nullptr);
        CHECK(state.render_calls == 1);
        CHECK(state.enter_calls == 0);
    }

    TEST_CASE("batch6647 exit calls do not clear last context observation") {
        MockIAppState state;
        Salix::InitContext context{};
        state.on_enter(context);
        state.on_exit();
        CHECK(state.last_context == &context);
        CHECK(state.exit_calls == 1);
    }

    TEST_CASE("batch6647 exit calls do not rewrite last delta observation") {
        MockIAppState state;
        state.update(6.647f);
        state.on_exit();
        CHECK(state.last_delta_time == doctest::Approx(6.647f));
        CHECK(state.exit_calls == 1);
    }

    TEST_CASE("batch6647 update calls do not rewrite last renderer pointer") {
        MockIAppState state;
        MockIRenderer renderer;
        state.render(&renderer);
        state.update(0.25f);
        CHECK(state.last_renderer == &renderer);
        CHECK(state.last_delta_time == doctest::Approx(0.25f));
    }

    TEST_CASE("batch6647 two app states maintain completely independent lifecycle accounting") {
        MockIAppState first;
        MockIAppState second;
        Salix::InitContext context{};
        first.on_enter(context);
        first.update(1.0f);
        second.on_exit();
        CHECK(first.enter_calls == 1);
        CHECK(first.update_calls == 1);
        CHECK(first.exit_calls == 0);
        CHECK(second.enter_calls == 0);
        CHECK(second.update_calls == 0);
        CHECK(second.exit_calls == 1);
    }

    TEST_CASE("batch6647 polymorphic ownership preserves exact lifecycle observations") {
        auto concrete = std::make_unique<MockIAppState>();
        MockIAppState* raw = concrete.get();
        std::unique_ptr<Salix::IAppState> state = std::move(concrete);
        Salix::InitContext context{};
        state->on_enter(context);
        state->update(0.6647f);
        state->render(nullptr);
        CHECK(raw->last_context == &context);
        CHECK(raw->last_delta_time == doctest::Approx(0.6647f));
        CHECK(raw->render_calls == 1);
    }
}
