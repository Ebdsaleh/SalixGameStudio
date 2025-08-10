// Editor/management/RealmLoader.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/management/RealmLoader.h>
#include <Editor/Archetypes.h>
#include <yaml-cpp/yaml.h>
#include <Salix/core/SimpleGuid.h>


namespace Salix {

    // This is the function DEFINITION.
    // It provides the actual implementation.
    std::vector<EntityArchetype> load_archetypes_from_file(const std::string& filepath) {
        std::vector<EntityArchetype> archetypes;
        
        try {
            YAML::Node realm_yaml = YAML::LoadFile(filepath);
            
            if (!realm_yaml["Entities"] || !realm_yaml["Entities"].IsSequence()) {
                std::cerr << "[RealmLoader] Invalid or missing 'Entities' node in YAML file: " << filepath << std::endl;
                return archetypes;
            }

            std::cout << "[RealmLoader] Parsing " << realm_yaml["Entities"].size() << " entities..." << std::endl;

            for (const auto& entity_node : realm_yaml["Entities"]) {
                try {
                    if (!entity_node.IsMap()) {
                        std::cerr << "[RealmLoader] Invalid entity node (expected map)" << std::endl;
                        continue;
                    }

                    EntityArchetype entity;

                    // Required fields
                    if (!entity_node["name"] || !entity_node["id"]) {
                        std::cerr << "[RealmLoader] Entity missing required fields (name or id)" << std::endl;
                        continue;
                    }

                    entity.name = entity_node["name"].as<std::string>();
                    entity.id = entity_node["id"].as<SimpleGuid>();

                    // Optional parent
                    if (entity_node["parent"] && !entity_node["parent"].IsNull()) {
                        entity.parent_id = entity_node["parent"].as<SimpleGuid>();
                    }

                    // Parse elements if they exist
                    if (entity_node["elements"] && entity_node["elements"].IsMap()) {
                        for (const auto& element_kv : entity_node["elements"]) {
                            const std::string type_name = element_kv.first.as<std::string>();
                            const YAML::Node& element_data = element_kv.second;

                            if (!element_data.IsMap()) {
                                std::cerr << "    [Element] Invalid element data for type: " << type_name << std::endl;
                                continue;
                            }

                            if (!element_data["id"]) {
                                std::cerr << "    [Element] Missing ID for element of type: " << type_name << std::endl;
                                continue;
                            }

                            try {
                                ElementArchetype element;
                                element.type_name = type_name;
                                element.id = element_data["id"].as<SimpleGuid>();
                                element.data = YAML::Clone(element_data);
                                element.data.remove("id");

                                entity.elements.push_back(element);
                            } catch (const YAML::Exception& e) {
                                std::cerr << "    [Element] Error parsing element '" << type_name 
                                        << "': " << e.what() << std::endl;
                            }
                        }
                    }

                    archetypes.push_back(entity);

                } catch (const YAML::Exception& e) {
                    std::cerr << "[RealmLoader] Error parsing entity: " << e.what() 
                            << " (line " << e.mark.line << ")" << std::endl;
                }
            }

            std::cout << "[RealmLoader] Successfully loaded " << archetypes.size() << " entities" << std::endl;
            
        } catch (const YAML::Exception& e) {
            std::cerr << "[RealmLoader] YAML error: " << e.what() 
                    << " (line " << e.mark.line << ")" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[RealmLoader] Unexpected error: " << e.what() << std::endl;
        }

        return archetypes;
    }
} // namespace Salix