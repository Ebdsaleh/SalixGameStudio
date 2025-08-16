// Editor/management/RealmSnapshot.cpp
#include <Editor/management/RealmSnapshot.h>
#include <Editor/management/RealmLoader.h>

namespace Salix {

    RealmSnapshot RealmSnapshot::load_from_file(const std::string& filepath) {
        RealmSnapshot snapshot;
        snapshot.source_filepath = filepath;

        auto archetypes = load_archetypes_from_file(filepath);
        for (auto& a : archetypes) {
            snapshot.entity_archetype_map[a.id] = std::move(a);
        }

        return snapshot;
    }

    RealmSnapshot RealmSnapshot::load_from_entity_archetype_vector(const std::vector<EntityArchetype>& archetype_vector) {
        RealmSnapshot snapshot;
        for (const auto& archetype : archetype_vector) {
            snapshot.entity_archetype_map[archetype.id] = archetype;
        }
        return snapshot;
    }

}