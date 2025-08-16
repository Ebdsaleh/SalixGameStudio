// Editor/management/RealmSnapshot.h

#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/Archetypes.h>
#include <unordered_map>
#include <vector>
#include <string>

namespace Salix {
    struct EDITOR_API RealmSnapshot {
        std::unordered_map<SimpleGuid, EntityArchetype> entity_archetype_map;
        // later: std::unordered_map<SimpleGuid, ElementArchetype> element_archetype_map;

        // optional: file or source info
        std::string source_filepath;

        // convenience function to build a snapshot from file
        static RealmSnapshot load_from_file(const std::string& filepath);

        static RealmSnapshot load_from_entity_archetype_vector(const std::vector<EntityArchetype>& achetype_vector);
    };

} // namespace Salix