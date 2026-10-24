#include <doctest.h>
#include <Salix/events/sdl/SDLEventBase.h>
#include <Salix/events/IEvent.h>
#include <SDL.h>
#include <memory>
#include <string>

namespace {
class Batch1047SDLEventProbe : public Salix::SDLEventBase {
public:
    explicit Batch1047SDLEventProbe(const SDL_Event& event)
        : Salix::SDLEventBase(event) {}

    Salix::EventType get_event_type() const override { return Salix::EventType::AppTick; }
    const char* get_name() const override { return "Batch1047SDLEventProbe"; }
    int get_category_flags() const override { return static_cast<int>(Salix::EventCategory::Application); }
    std::unique_ptr<Salix::IEvent> clone() const override {
        return std::make_unique<Batch1047SDLEventProbe>(*this);
    }
};
}

TEST_SUITE("Salix::events::SDLEventBaseBatch1047") {
    TEST_CASE("native handle points at the exact SDL event object supplied to constructor") {
        SDL_Event raw{};
        raw.type = SDL_USEREVENT;
        Batch1047SDLEventProbe event(raw);
        CHECK(event.get_native_handle() == &raw);
    }

    TEST_CASE("native handle exposes original event type without copying SDL event storage") {
        SDL_Event raw{};
        raw.type = SDL_KEYDOWN;
        Batch1047SDLEventProbe event(raw);
        auto* native = static_cast<SDL_Event*>(event.get_native_handle());
        REQUIRE(native != nullptr);
        CHECK(native->type == SDL_KEYDOWN);
    }

    TEST_CASE("wrapper observes mutations made to the original SDL event after construction") {
        SDL_Event raw{};
        raw.type = SDL_KEYDOWN;
        Batch1047SDLEventProbe event(raw);
        raw.type = SDL_KEYUP;
        raw.key.keysym.sym = SDLK_ESCAPE;
        auto* native = static_cast<SDL_Event*>(event.get_native_handle());
        REQUIRE(native != nullptr);
        CHECK(native->type == SDL_KEYUP);
        CHECK(native->key.keysym.sym == SDLK_ESCAPE);
    }

    TEST_CASE("mutable native handle aliases original SDL event storage") {
        SDL_Event raw{};
        raw.type = SDL_USEREVENT;
        raw.user.code = 1;
        Batch1047SDLEventProbe event(raw);
        auto* native = static_cast<SDL_Event*>(event.get_native_handle());
        REQUIRE(native != nullptr);
        native->user.code = 77;
        CHECK(raw.user.code == 77);
    }

    TEST_CASE("wrappers around different SDL events retain distinct native identities") {
        SDL_Event first{};
        SDL_Event second{};
        first.type = SDL_MOUSEMOTION;
        second.type = SDL_MOUSEWHEEL;
        Batch1047SDLEventProbe first_wrapper(first);
        Batch1047SDLEventProbe second_wrapper(second);
        CHECK(first_wrapper.get_native_handle() == &first);
        CHECK(second_wrapper.get_native_handle() == &second);
        CHECK(first_wrapper.get_native_handle() != second_wrapper.get_native_handle());
    }

    TEST_CASE("multiple wrappers around one SDL event expose the same native handle") {
        SDL_Event raw{};
        raw.type = SDL_QUIT;
        Batch1047SDLEventProbe first(raw);
        Batch1047SDLEventProbe second(raw);
        CHECK(first.get_native_handle() == second.get_native_handle());
        CHECK(first.get_native_handle() == &raw);
    }

    TEST_CASE("native handle dispatches correctly through IEvent interface") {
        SDL_Event raw{};
        raw.type = SDL_WINDOWEVENT;
        Batch1047SDLEventProbe concrete(raw);
        Salix::IEvent* base = &concrete;
        CHECK(base->get_native_handle() == &raw);
        CHECK(base->get_event_type() == Salix::EventType::AppTick);
        CHECK(std::string(base->get_name()) == "Batch1047SDLEventProbe");
    }

    TEST_CASE("SDL event base inherits default unhandled and nonblocking event state") {
        SDL_Event raw{};
        Batch1047SDLEventProbe event(raw);
        CHECK_FALSE(event.handled);
        CHECK_FALSE(event.should_block());
        CHECK(event.is_application_event());
        CHECK_FALSE(event.is_input_event());
    }

    TEST_CASE("default set block implementation does not change SDL event blocking behavior") {
        SDL_Event raw{};
        Batch1047SDLEventProbe event(raw);
        event.set_block(true);
        CHECK_FALSE(event.should_block());
        event.set_block(false);
        CHECK_FALSE(event.should_block());
    }

    TEST_CASE("cloned SDL wrapper keeps reference to same native event and copies handled state") {
        SDL_Event raw{};
        raw.type = SDL_USEREVENT;
        Batch1047SDLEventProbe source(raw);
        source.handled = true;
        auto clone = source.clone();
        REQUIRE(clone != nullptr);
        CHECK(clone->get_native_handle() == &raw);
        CHECK(clone->handled);
        raw.type = SDL_QUIT;
        auto* clone_native = static_cast<SDL_Event*>(clone->get_native_handle());
        REQUIRE(clone_native != nullptr);
        CHECK(clone_native->type == SDL_QUIT);
        clone->handled = false;
        CHECK(source.handled);
    }
}
