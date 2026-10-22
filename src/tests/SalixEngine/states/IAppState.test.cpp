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
