#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/Archetypes.h>

namespace Salix {
    class Scene; // Forward-declare Scene
    struct InitContext;

    class EDITOR_API ArchetypeInstantiator {
    public:
        // Takes an archetype and a scene, and creates a fully formed live entity
        static void instantiate(const Salix::EntityArchetype& archetype, Salix::Scene* scene, const Salix::InitContext& context);
    };
}  // namespace Salix