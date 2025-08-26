#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/Archetypes.h>
#include <string>
#include <vector>
namespace Salix {
    class Scene; // Forward-declare Scene
    struct InitContext;

    class EDITOR_API ArchetypeInstantiator {
    public:
        // Takes an archetype and a scene, and creates a fully formed live entity
        static void instantiate(const Salix::EntityArchetype& archetype, Salix::Scene* scene, const Salix::InitContext& context);
        // Add the new debug method declaration
        static void print_all_entity_ids(Salix::Scene* scene, const std::string& context_message);
        static void instantiate_realm(const std::vector<Salix::EntityArchetype>& realm, Salix::Scene* scene, const Salix::InitContext& context);
    };
}  // namespace Salix