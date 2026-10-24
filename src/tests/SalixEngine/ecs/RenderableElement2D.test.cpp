#include <doctest.h>
#include <Salix/ecs/RenderableElement2D.h>
#include <Salix/ecs/Entity.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/ecs/MockRenderableElement.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::ecs::RenderableElement2DBatch1547") {
    TEST_CASE("2D renderable defaults visible through its own base layer") {
        MockRenderableElement2D element;
        Salix::RenderableElement2D* base = &element;
        CHECK(base->is_visible());
    }

    TEST_CASE("2D renderable visibility writes remain observable through RenderableElement base") {
        MockRenderableElement2D element;
        Salix::RenderableElement* parent = &element;
        element.set_visibility(false);
        CHECK_FALSE(parent->is_visible());
        parent->set_visibility(true);
        CHECK(element.is_visible());
    }

    TEST_CASE("2D renderable render dispatch receives exact renderer pointer") {
        MockRenderableElement2D element;
        MockIRenderer renderer;
        Salix::RenderableElement2D* base = &element;
        base->render(&renderer);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == &renderer);
    }

    TEST_CASE("2D renderable on load receives exact initialization context") {
        MockRenderableElement2D element;
        Salix::InitContext context{};
        Salix::RenderableElement2D* base = &element;
        base->on_load(context);
        CHECK(element.load_calls == 1);
        CHECK(element.last_context == &context);
    }

    TEST_CASE("2D renderable identity remains distinct from generic renderable mock") {
        MockRenderableElement2D two_d;
        MockRenderableElementContract generic;
        CHECK(std::string(two_d.get_class_name()) == "MockRenderableElement2D");
        CHECK(std::string(generic.get_class_name()) == "MockRenderableElementContract");
    }

    TEST_CASE("2D renderable inherited name mutation does not alter class identity") {
        MockRenderableElement2D element;
        element.set_name("Sprite Layer");
        CHECK(element.get_name() == "Sprite Layer");
        CHECK(std::string(element.get_class_name()) == "MockRenderableElement2D");
    }

    TEST_CASE("2D renderable owner pointer is inherited and nonowning") {
        MockRenderableElement2D element;
        Salix::Entity owner;
        element.set_owner(&owner);
        CHECK(element.get_owner() == &owner);
        CHECK(owner.get_transform() != nullptr);
    }

    TEST_CASE("2D renderable visibility stays unchanged by render and load calls") {
        MockRenderableElement2D element;
        Salix::InitContext context{};
        element.set_visibility(false);
        element.render(nullptr);
        element.on_load(context);
        CHECK_FALSE(element.is_visible());
        CHECK(element.render_calls == 1);
        CHECK(element.load_calls == 1);
    }

    TEST_CASE("separate 2D renderables keep independent names visibility and counters") {
        MockRenderableElement2D first;
        MockRenderableElement2D second;
        first.set_name("First");
        first.set_visibility(false);
        first.render(nullptr);
        CHECK(first.get_name() == "First");
        CHECK_FALSE(first.is_visible());
        CHECK(second.get_name() == "Element");
        CHECK(second.is_visible());
        CHECK(second.render_calls == 0);
    }

    TEST_CASE("2D renderable supports polymorphic destruction through RenderableElement base") {
        std::unique_ptr<Salix::RenderableElement> element =
            std::make_unique<MockRenderableElement2D>();
        REQUIRE(element != nullptr);
        CHECK(std::string(element->get_class_name()) == "MockRenderableElement2D");
        CHECK(element->is_visible());
    }
}
