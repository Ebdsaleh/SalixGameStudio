#include <doctest.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Entity.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/ecs/MockRenderableElement.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::ecs::RenderableElementBatch1547") {
    TEST_CASE("renderable element defaults visible through abstract base contract") {
        MockRenderableElementContract element;
        Salix::RenderableElement* base = &element;
        CHECK(base->is_visible());
    }

    TEST_CASE("renderable element visibility setter toggles inherited visibility flag") {
        MockRenderableElementContract element;
        Salix::RenderableElement* base = &element;
        base->set_visibility(false);
        CHECK_FALSE(base->is_visible());
        base->set_visibility(true);
        CHECK(base->is_visible());
    }

    TEST_CASE("renderable element render dispatch receives exact renderer pointer") {
        MockRenderableElementContract element;
        MockIRenderer renderer;
        Salix::RenderableElement* base = &element;
        base->render(&renderer);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == &renderer);
    }

    TEST_CASE("renderable element render contract accepts null renderer for isolated call") {
        MockRenderableElementContract element;
        Salix::RenderableElement* base = &element;
        base->render(nullptr);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == nullptr);
    }

    TEST_CASE("renderable element on load receives exact initialization context") {
        MockRenderableElementContract element;
        Salix::InitContext context{};
        Salix::RenderableElement* base = &element;
        base->on_load(context);
        CHECK(element.load_calls == 1);
        CHECK(element.last_context == &context);
    }

    TEST_CASE("renderable element retains inherited element display name independently") {
        MockRenderableElementContract element;
        element.set_name("Renderable Probe");
        CHECK(element.get_name() == "Renderable Probe");
        CHECK(std::string(element.get_class_name()) == "MockRenderableElementContract");
    }

    TEST_CASE("renderable element owner transport remains inherited from Element") {
        MockRenderableElementContract element;
        Salix::Entity owner;
        element.set_owner(&owner);
        CHECK(element.get_owner() == &owner);
        element.set_owner(nullptr);
        CHECK(element.get_owner() == nullptr);
    }

    TEST_CASE("renderable element identifier remains valid across render and load") {
        MockRenderableElementContract element;
        MockIRenderer renderer;
        Salix::InitContext context{};
        const auto id = element.get_id();
        element.render(&renderer);
        element.on_load(context);
        CHECK(element.get_id() == id);
        CHECK(id.is_valid());
    }

    TEST_CASE("separate renderable elements maintain independent visibility and call state") {
        MockRenderableElementContract first;
        MockRenderableElementContract second;
        first.set_visibility(false);
        first.render(nullptr);
        CHECK_FALSE(first.is_visible());
        CHECK(first.render_calls == 1);
        CHECK(second.is_visible());
        CHECK(second.render_calls == 0);
    }

    TEST_CASE("renderable element supports polymorphic destruction through Element base") {
        std::unique_ptr<Salix::Element> element = std::make_unique<MockRenderableElementContract>();
        REQUIRE(element != nullptr);
        CHECK(std::string(element->get_class_name()) == "MockRenderableElementContract");
        CHECK(element->is_visible());
    }
}

TEST_SUITE("Salix::ecs::RenderableElementBatch6747") {
    TEST_CASE("batch6747 repeated render calls keep latest renderer and exact count") {
        MockRenderableElementContract element;
        MockIRenderer first;
        MockIRenderer second;
        element.render(&first);
        element.render(&second);
        CHECK(element.render_calls == 2);
        CHECK(element.last_renderer == &second);
    }

    TEST_CASE("batch6747 renderer may transition from nonnull to null") {
        MockRenderableElementContract element;
        MockIRenderer renderer;
        element.render(&renderer);
        REQUIRE(element.last_renderer == &renderer);
        element.render(nullptr);
        CHECK(element.last_renderer == nullptr);
        CHECK(element.render_calls == 2);
    }

    TEST_CASE("batch6747 repeated load calls keep latest context and exact count") {
        MockRenderableElementContract element;
        Salix::InitContext first{};
        Salix::InitContext second{};
        element.on_load(first);
        element.on_load(second);
        CHECK(element.load_calls == 2);
        CHECK(element.last_context == &second);
    }

    TEST_CASE("batch6747 visibility survives render and load sequence") {
        MockRenderableElementContract element;
        Salix::InitContext context{};
        element.set_visibility(false);
        element.render(nullptr);
        element.on_load(context);
        CHECK_FALSE(element.is_visible());
    }

    TEST_CASE("batch6747 display name survives render and load sequence") {
        MockRenderableElementContract element;
        Salix::InitContext context{};
        element.set_name("Renderable6747");
        element.render(nullptr);
        element.on_load(context);
        CHECK(element.get_name() == "Renderable6747");
    }

    TEST_CASE("batch6747 owner survives render and load sequence") {
        Salix::Entity owner;
        MockRenderableElementContract element;
        Salix::InitContext context{};
        element.set_owner(&owner);
        element.render(nullptr);
        element.on_load(context);
        CHECK(element.get_owner() == &owner);
    }

    TEST_CASE("batch6747 identifier replacement survives render and load sequence") {
        MockRenderableElementContract element;
        element.set_id(Salix::SimpleGuid::from_value(6747003ULL));
        Salix::InitContext context{};
        element.render(nullptr);
        element.on_load(context);
        CHECK(element.get_id().get_value() == 6747003ULL);
    }

    TEST_CASE("batch6747 render never changes load accounting") {
        MockRenderableElementContract element;
        element.render(nullptr);
        element.render(nullptr);
        CHECK(element.render_calls == 2);
        CHECK(element.load_calls == 0);
    }

    TEST_CASE("batch6747 load never changes render accounting") {
        MockRenderableElementContract element;
        Salix::InitContext context{};
        element.on_load(context);
        element.on_load(context);
        CHECK(element.load_calls == 2);
        CHECK(element.render_calls == 0);
    }

    TEST_CASE("batch6747 two renderables maintain independent lifecycle observations") {
        MockRenderableElementContract first;
        MockRenderableElementContract second;
        Salix::InitContext context{};
        first.set_visibility(false);
        first.render(nullptr);
        second.on_load(context);
        CHECK_FALSE(first.is_visible());
        CHECK(first.render_calls == 1);
        CHECK(first.load_calls == 0);
        CHECK(second.is_visible());
        CHECK(second.render_calls == 0);
        CHECK(second.load_calls == 1);
    }
}
