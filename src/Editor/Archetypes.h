// Editor/Archetypes.h
#pragma once
#include <Editor/EditorAPI.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
#include <Salix/core/SimpleGuid.h>
#include <yaml-cpp/yaml.h>
#include <string>
#include <vector>
#include <variant>

namespace Salix {

    enum class EDITOR_API ArchetypeState {
        Unmodified, // Loaded from file, unchanged (White)
        New,        // Created this session, unsaved (Green)
        Modified    // Loaded from file, changed this session (Orange)
    };

    struct EDITOR_API ElementArchetype {
        // The type of the element, e.g., "Transform", "Sprite2D".
        std::string type_name;
        std::string name;
        SimpleGuid id = SimpleGuid::invalid();
        bool allows_duplication = true;
        ArchetypeState state = ArchetypeState::Unmodified;
        // The raw YAML data for this element's properties.
        YAML::Node data;
        // Empty default constructor
         ElementArchetype() : id(SimpleGuid::invalid()) {} // Auto-generate ID on creation
    };

    struct EDITOR_API EntityArchetype {
        // Data from your Entity class
        std::string name;
        SimpleGuid id;
        
        // We replace raw pointers with IDs for hierarchy
        SimpleGuid parent_id = SimpleGuid::invalid();
        std::vector<SimpleGuid> child_ids;
       
        // A list of its element archetypes instead of live elements
        std::vector<ElementArchetype> elements;
        ArchetypeState state = ArchetypeState::Unmodified;
        // Empty default constructor
        EntityArchetype() : id(SimpleGuid::invalid()){}
        
    };

    
    
}