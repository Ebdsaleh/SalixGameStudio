#pragma once

#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/RenderableElement2D.h>
#include <Salix/ecs/RenderableElement3D.h>
#include <Salix/core/InitContext.h>

class MockRenderableElementContract : public Salix::RenderableElement {
public:
    const char* get_class_name() const override { return "MockRenderableElementContract"; }

    void render(Salix::IRenderer* renderer) override {
        last_renderer = renderer;
        ++render_calls;
    }

    void on_load(const Salix::InitContext& context) override {
        last_context = &context;
        ++load_calls;
    }

    Salix::IRenderer* last_renderer = nullptr;
    const Salix::InitContext* last_context = nullptr;
    int render_calls = 0;
    int load_calls = 0;
};

class MockRenderableElement2D : public Salix::RenderableElement2D {
public:
    const char* get_class_name() const override { return "MockRenderableElement2D"; }

    void render(Salix::IRenderer* renderer) override {
        last_renderer = renderer;
        ++render_calls;
    }

    void on_load(const Salix::InitContext& context) override {
        last_context = &context;
        ++load_calls;
    }

    Salix::IRenderer* last_renderer = nullptr;
    const Salix::InitContext* last_context = nullptr;
    int render_calls = 0;
    int load_calls = 0;
};

class MockRenderableElement3D : public Salix::RenderableElement3D {
public:
    const char* get_class_name() const override { return "MockRenderableElement3D"; }

    void render(Salix::IRenderer* renderer) override {
        last_renderer = renderer;
        ++render_calls;
    }

    void on_load(const Salix::InitContext& context) override {
        last_context = &context;
        ++load_calls;
    }

    Salix::IRenderer* last_renderer = nullptr;
    const Salix::InitContext* last_context = nullptr;
    int render_calls = 0;
    int load_calls = 0;
};
