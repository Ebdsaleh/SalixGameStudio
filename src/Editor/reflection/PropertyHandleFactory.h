// Editor/reflection/PropertyHandleFactory.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Salix/reflection/PropertyHandle.h>
#include <Editor/Archetypes.h>
#include <vector>
#include <memory>

namespace Salix {
    class EDITOR_API PropertyHandleFactory {
    public:
        // Creates a vector of property handles from a loaded EntityArchetype.
        static std::vector<std::unique_ptr<Salix::PropertyHandle>> create_handles_for_archetype(Salix::EntityArchetype* entity_archetype);
    };
} // namespace Salix