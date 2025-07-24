// Editor/camera/EditorCamera.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/EditorContext.h>
#include <Salix/rendering/ICamera.h>
#include <Salix/events/IEventListener.h>
#include <memory>

namespace Salix {
    struct EditorContext;
    class EDITOR_API EditorCamera : public ICamera, public IEventListener {
    public:
        EditorCamera();
        ~EditorCamera() override;
        void initialize(EditorContext* context);
        // The main update loop for handling input
        void on_update(float delta_time);

        const glm::mat4& get_view_matrix() override;
        const glm::mat4& get_projection_matrix() override;
        void on_event(IEvent& event) override;
        void set_viewport_size(int width, int height);
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
}