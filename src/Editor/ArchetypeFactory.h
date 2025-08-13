// Editor/ArchetypeFactory.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/Archetypes.h>
#include <string>

namespace Salix {

    class EDITOR_API ArchetypeFactory {
    public:
        // Creates an ElementArchetype (e.g., a Transform) with default values.
        static ElementArchetype CreateElementArchetype(const std::string& type_name);

        // Creates an EntityArchetype with a default name and a Transform component.
        static EntityArchetype CreateEntityArchetype(const std::string& name = "Entity");
    };

} // namespace Salix