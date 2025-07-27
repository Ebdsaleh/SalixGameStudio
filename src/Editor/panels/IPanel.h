// =================================================================================
// Filename:    Editor/panels/IPanel.h
// Author:      SalixGameStudio
// Description: Defines the abstract interface for all editor panels.
// =================================================================================
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/behaviours/EditorBehaviour.h>
#include <Salix/events/IEventListener.h>
#include <imgui/imgui.h>
namespace Salix {

    struct EditorContext;

    class EDITOR_API IPanel : public EditorBehaviour, public IEventListener{
    public:
        // A virtual destructor is essential for any class intended for polymorphism.
        virtual ~IPanel() = default;

        virtual void initialize(EditorContext* context) = 0;

        // This is the core function for any panel.
        // The editor will call this every frame, and the panel is responsible
        // for all of its ImGui rendering code inside this function.

        virtual void on_gui_update() = 0;
        virtual void on_gui_render() = 0;
        virtual void on_render() = 0;
        virtual void set_visibility(bool visibility) = 0;
        virtual bool get_visibility() const = 0;
        virtual void set_name(const std::string& new_name) = 0;
        virtual const std::string& get_name() = 0;
    };

} // namespace Salix