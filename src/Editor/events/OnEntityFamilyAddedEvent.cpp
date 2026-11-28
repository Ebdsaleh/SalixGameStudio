// Editor/events/OnEntityFamilyAddedEvent.cpp
#include <Editor/events/OnEntityFamilyAddedEvent.h>
#include <utility>

namespace Salix {
    OnEntityFamilyAddedEvent::OnEntityFamilyAddedEvent(std::vector<EntityArchetype> new_archetypes)
        : archetypes(std::move(new_archetypes)) {
        for (auto& entity : archetypes) {
            for (auto& element : entity.elements) {
                // Rebind the handle; assignment may modify an aliased YAML node.
                element.data.reset(YAML::Clone(element.data));
            }
        }
    }

    OnEntityFamilyAddedEvent::OnEntityFamilyAddedEvent(const OnEntityFamilyAddedEvent& other)
        : OnEntityFamilyAddedEvent(other.archetypes) {
        handled = other.handled;
    }

    OnEntityFamilyAddedEvent& OnEntityFamilyAddedEvent::operator=(const OnEntityFamilyAddedEvent& other) {
        if (this != &other) {
            OnEntityFamilyAddedEvent detached(other);
            archetypes.swap(detached.archetypes);
            handled = detached.handled;
        }
        return *this;
    }

    EventType OnEntityFamilyAddedEvent::get_static_type() { return EventType::EditorOnEntityFamilyAdded; }
    EventType OnEntityFamilyAddedEvent::get_event_type() const { return get_static_type(); }
    const char* OnEntityFamilyAddedEvent::get_name() const { return "EditorOnEntityFamilyAdded"; }
    int OnEntityFamilyAddedEvent::get_category_flags() const { return static_cast<int>(EventCategory::Editor); }
    std::unique_ptr<IEvent> OnEntityFamilyAddedEvent::clone() const {
        return std::make_unique<OnEntityFamilyAddedEvent>(*this);
    }
}
