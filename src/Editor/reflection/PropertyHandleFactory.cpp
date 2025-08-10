// Editor/reflection/PropertyHandleFactory.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/reflection/PropertyHandleFactory.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/PropertyHandleYaml.h>

namespace Salix {
    std::vector<std::unique_ptr<Salix::PropertyHandle>> PropertyHandleFactory::create_handles_for_archetype(Salix::EntityArchetype* entity_archetype) {
        std::vector<std::unique_ptr<Salix::PropertyHandle>> handles;
        if (!entity_archetype) {
            return handles;
        }

        // Loop through each element archetype (e.g., Transform, Sprite2D)
        for (auto& element_archetype : entity_archetype->elements) {
            
            // Use the engine's ByteMirror to get reflection data by name
            const Salix::TypeInfo* type_info = Salix::ByteMirror::get_type_info_by_name(element_archetype.type_name);
            if (!type_info) continue;

            // The 'data' node is what PropertyHandleYaml needs to operate on
            YAML::Node* properties_node = &element_archetype.data;

            // For every property this element type has...
            for (const auto& prop : type_info->properties) {
                // ...if the property exists in the YAML data...
                if ((*properties_node)[prop.name]) {
                    // ...create a PropertyHandleYaml for it.
                    handles.push_back(std::make_unique<Salix::PropertyHandleYaml>(&prop, properties_node));
                }
            }
        }

        return handles;
    }

} // namespace Salix