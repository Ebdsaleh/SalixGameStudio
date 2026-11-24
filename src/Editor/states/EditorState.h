// =================================================================================
// Filename:    Editor/states/EditorState.h
// Author:      SalixGameStudio
// Description: The main state for the Salix Editor application.
// =================================================================================
#pragma once
#include <Editor/EditorAPI.h>
#include <Salix/states/IAppState.h>
#include <Salix/events/IEventListener.h>

#include <memory>

namespace Salix {
    struct EditorContext;

    class EDITOR_API EditorState : public IAppState, public IEventListener{
    public:
        EditorState();
        ~EditorState();

        // IAppState interface implementation
        void on_enter(const InitContext& new_context) override;
        void on_exit() override;
        void update(float delta_time) override;
        void render(IRenderer* renderer_param) override;
        void on_event(IEvent& event) override;
#ifdef SALIX_TESTS_ENABLED
        EditorContext* get_context_for_testing() const;
#endif
    private:
        // Using the Pimpl idiom to hide implementation details
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
}