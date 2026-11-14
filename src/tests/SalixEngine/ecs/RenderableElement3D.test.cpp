#include <doctest.h>
#include <Salix/ecs/RenderableElement3D.h>
#include <Salix/ecs/Entity.h>
#include <Salix/core/InitContext.h>
#include <Tests/SalixEngine/mocking/ecs/MockRenderableElement.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <memory>
#include <string>

TEST_SUITE("Salix::ecs::RenderableElement3DBatch1547") {
    TEST_CASE("3D renderable defaults visible through its own base layer") {
        MockRenderableElement3D element;
        Salix::RenderableElement3D* base = &element;
        CHECK(base->is_visible());
    }

    TEST_CASE("3D renderable visibility writes remain observable through RenderableElement base") {
        MockRenderableElement3D element;
        Salix::RenderableElement* parent = &element;
        parent->set_visibility(false);
        CHECK_FALSE(element.is_visible());
        element.set_visibility(true);
        CHECK(parent->is_visible());
    }

    TEST_CASE("3D renderable render dispatch receives exact renderer pointer") {
        MockRenderableElement3D element;
        MockIRenderer renderer;
        Salix::RenderableElement3D* base = &element;
        base->render(&renderer);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == &renderer);
    }

    TEST_CASE("3D renderable on load receives exact initialization context") {
        MockRenderableElement3D element;
        Salix::InitContext context{};
        Salix::RenderableElement3D* base = &element;
        base->on_load(context);
        CHECK(element.load_calls == 1);
        CHECK(element.last_context == &context);
    }

    TEST_CASE("3D renderable identity remains distinct from 2D renderable identity") {
        MockRenderableElement3D three_d;
        MockRenderableElement2D two_d;
        CHECK(std::string(three_d.get_class_name()) == "MockRenderableElement3D");
        CHECK(std::string(two_d.get_class_name()) == "MockRenderableElement2D");
    }

    TEST_CASE("3D renderable inherited display name changes independently of visibility") {
        MockRenderableElement3D element;
        element.set_visibility(false);
        element.set_name("Mesh Probe");
        CHECK(element.get_name() == "Mesh Probe");
        CHECK_FALSE(element.is_visible());
    }

    TEST_CASE("3D renderable owner pointer can be assigned and cleared") {
        MockRenderableElement3D element;
        Salix::Entity owner;
        element.set_owner(&owner);
        CHECK(element.get_owner() == &owner);
        element.set_owner(nullptr);
        CHECK(element.get_owner() == nullptr);
    }

    TEST_CASE("3D renderable render accepts null without altering persistent metadata") {
        MockRenderableElement3D element;
        element.set_name("Null Renderer Probe");
        const auto id = element.get_id();
        element.render(nullptr);
        CHECK(element.render_calls == 1);
        CHECK(element.last_renderer == nullptr);
        CHECK(element.get_name() == "Null Renderer Probe");
        CHECK(element.get_id() == id);
    }

    TEST_CASE("separate 3D renderables retain independent load contexts") {
        MockRenderableElement3D first;
        MockRenderableElement3D second;
        Salix::InitContext first_context{};
        Salix::InitContext second_context{};
        first.on_load(first_context);
        second.on_load(second_context);
        CHECK(first.last_context == &first_context);
        CHECK(second.last_context == &second_context);
        CHECK(first.last_context != second.last_context);
    }

    TEST_CASE("3D renderable supports polymorphic destruction through Element base") {
        std::unique_ptr<Salix::Element> element =
            std::make_unique<MockRenderableElement3D>();
        REQUIRE(element != nullptr);
        CHECK(std::string(element->get_class_name()) == "MockRenderableElement3D");
        CHECK(element->is_visible());
    }
}

TEST_SUITE("Salix::ecs::RenderableElement3DBatch6747") {
    TEST_CASE("batch6747 repeated render calls keep latest renderer") {
        MockRenderableElement3D element;
        MockIRenderer first;
        MockIRenderer second;
        element.render(&first);
        element.render(&second);
        CHECK(element.render_calls == 2);
        CHECK(element.last_renderer == &second);
    }

    TEST_CASE("batch6747 renderer pointer can be cleared with null render") {
        MockRenderableElement3D element;
        MockIRenderer renderer;
        element.render(&renderer);
        element.render(nullptr);
        CHECK(element.render_calls == 2);
        CHECK(element.last_renderer == nullptr);
    }

    TEST_CASE("batch6747 repeated load calls keep latest context pointer") {
        MockRenderableElement3D element;
        Salix::InitContext first{};
        Salix::InitContext second{};
        element.on_load(first);
        element.on_load(second);
        CHECK(element.load_calls == 2);
        CHECK(element.last_context == &second);
    }

    TEST_CASE("batch6747 hidden visibility survives render and load calls") {
        MockRenderableElement3D element;
        Salix::InitContext context{};
        element.set_visibility(false);
        element.render(nullptr);
        element.on_load(context);
        CHECK_FALSE(element.is_visible());
    }

    TEST_CASE("batch6747 inherited name survives render and load calls") {
        MockRenderableElement3D element;
        Salix::InitContext context{};
        element.set_name("ThreeD6747");
        element.render(nullptr);
        element.on_load(context);
        CHECK(element.get_name() == "ThreeD6747");
    }

    TEST_CASE("batch6747 owner replacement never affects render or load counts") {
        Salix::Entity first;
        Salix::Entity second;
        MockRenderableElement3D element;
        element.set_owner(&first);
        element.set_owner(&second);
        CHECK(element.get_owner() == &second);
        CHECK(element.render_calls == 0);
        CHECK(element.load_calls == 0);
    }

    TEST_CASE("batch6747 explicit identifier survives lifecycle calls") {
        MockRenderableElement3D element;
        Salix::InitContext context{};
        element.set_id(Salix::SimpleGuid::from_value(6747004ULL));
        element.render(nullptr);
        element.on_load(context);
        CHECK(element.get_id().get_value() == 6747004ULL);
    }

    TEST_CASE("batch6747 class identity remains stable after empty display name") {
        MockRenderableElement3D element;
        element.set_name("");
        CHECK(element.get_name().empty());
        CHECK(std::string(element.get_class_name()) == "MockRenderableElement3D");
    }

    TEST_CASE("batch6747 renderable parent shares same visibility storage") {
        MockRenderableElement3D element;
        Salix::RenderableElement& parent = element;
        element.set_visibility(false);
        CHECK_FALSE(parent.is_visible());
        parent.set_visibility(true);
        CHECK(element.is_visible());
    }

    TEST_CASE("batch6747 separate 3D renderables keep lifecycle state independent") {
        MockRenderableElement3D first;
        MockRenderableElement3D second;
        Salix::InitContext context{};
        first.render(nullptr);
        second.on_load(context);
        CHECK(first.render_calls == 1);
        CHECK(first.load_calls == 0);
        CHECK(second.render_calls == 0);
        CHECK(second.load_calls == 1);
    }
}
