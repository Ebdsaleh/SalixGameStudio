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
