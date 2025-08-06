// Salix/reflection/ByteMirror.cpp
#include <cstddef>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>
#include <Salix/core/SimpleGuid.h>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/reflection/PropertyHandleLive.h>
#include <Salix/reflection/PropertyHandleYaml.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/ecs/CppScript.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/RenderableElement2D.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Transform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Salix {

    // Define the static map here, outside of the class definition.
    // A static class cannot have a constructor.
    std::unordered_map<std::type_index, TypeInfo> ByteMirror::type_registry;
    
    // Step 1: Register the base classes first.
    template<>
    void ByteMirror::register_type<Element>() {
        TypeInfo type_info;
        type_info.name = "Element";
        type_info.ancestor = nullptr;
        type_info.type_index = typeid(Element);

       
        // Expose the Element's ID as a read-only property.
        type_info.properties = {
            {
                "ID",                   // The name that will appear in the editor
                PropertyType::UInt64,   // The new type we added for 64-bit integers
                nullptr,                // No contained_type_info is needed

                // Getter: This lambda calls the new get_id_as_ptr() method.
                [](void* instance) {
                    return (void*)static_cast<Element*>(instance)->get_id_as_ptr();
                },

                // Setter: This lambda is empty, making the ID read-only in the editor.
                [](void* instance, void* data) {}
            }
        };
        

        type_registry[typeid(Element)] = type_info;
    }
    

    // Step 2: Register the descendents.

    // Transform
    template<>
    void ByteMirror::register_type<Transform>() {
        TypeInfo type_info;
        type_info.name = std::string("Transform");
        type_info.ancestor = get_type_info(typeid(Element));
        type_info.properties = {
            { 
                "Position", PropertyType::Vector3, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(const_cast<Vector3*>(
                    &static_cast<Transform*>(instance)->get_position()));
                },

                //setter_func
                [](void* instance, void* data) {
                    static_cast<Transform*>(instance)->set_position(*static_cast<Vector3*>(data)); 
                }
            },

            { 
                "Rotation", PropertyType::Vector3, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(const_cast<Vector3*>(
                    &static_cast<Transform*>(instance)->get_rotation()));
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<Transform*>(instance)->set_rotation(*static_cast<Vector3*>(data));
                }

            },
            
            { 
                "Scale", PropertyType::Vector3, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(const_cast<Vector3*>(
                    &static_cast<Transform*>(instance)->get_scale())); 
                },

                // setter_func
                [](void* instance, void* data) { 
                    static_cast<Transform*>(instance)->set_scale(*static_cast<Vector3*>(data)); 
                }
            }
              
        };
        type_info.type_index = typeid(Transform);
        type_registry[typeid(Transform)] = type_info;
    }




    //RenderableElement
    template<>
    void ByteMirror::register_type<RenderableElement>() {
        TypeInfo type_info;
        type_info.name = std::string("RenderableElement");
        // The ancestor is Element.
        type_info.ancestor = get_type_info(typeid(Element));
        // It has no properties of its own to reflect.
        type_info.type_index = typeid(RenderableElement);
        type_registry[typeid(RenderableElement)] = type_info;
    }

    


    // ScriptElement
    template<>
    void ByteMirror::register_type<ScriptElement>() {
        TypeInfo type_info;
        type_info.name = std::string("ScriptElement");
        type_info.ancestor = get_type_info(typeid(Element));
        type_info.type_index = typeid(ScriptElement);
        type_registry[typeid(ScriptElement)] = type_info;
    }



    // CppScript
    template<>
    void ByteMirror::register_type<CppScript>() {
        TypeInfo type_info;
        type_info.name = std::string("CppScript");
        type_info.ancestor = get_type_info(typeid(ScriptElement));
        type_info.properties = {
            { 
                "Script Name", PropertyType::String, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(const_cast<std::string*>(
                    &static_cast<CppScript*>(instance)->get_script_name()));
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<CppScript*>(instance)->set_script_name(*static_cast<std::string*>(data)); 
                }
            }
        };
        type_info.type_index = typeid(CppScript);
        type_registry[typeid(CppScript)] = type_info;
    }




    // Sprite2D
    template<>
    void ByteMirror::register_type<Sprite2D>() { 
        TypeInfo type_info;
        type_info.name = std::string("Sprite2D");
        type_info.ancestor = get_type_info(typeid(RenderableElement2D));
        type_info.properties = {
            { 
                "Color", PropertyType::Color, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(
                    &static_cast<Sprite2D*>(instance)->get_color());
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<Sprite2D*>(instance)->set_color(*static_cast<Color*>(data));
                }
            },

            {
                "Offset", PropertyType::Vector2, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(
                    &static_cast<Sprite2D*>(instance)->get_offset()); 
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<Sprite2D*>(instance)->set_offset(*static_cast<Vector2*>(data));
                }
            },

            {
                "Use Entity Rotation", PropertyType::Bool, nullptr,
                // getter_func
                [](void* instance) {return static_cast<void*>(
                    &static_cast<Sprite2D*>(instance)->get_use_entity_rotation());
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<Sprite2D*>(instance)->set_use_entity_rotation(*static_cast<bool*>(data)); 
                }

            },

            {
                "Pivot", PropertyType::Vector2, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(
                    &static_cast<Sprite2D*>(instance)->get_pivot()); 
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<Sprite2D*>(instance)->set_pivot(*static_cast<Vector2*>(data)); 
                }
            },

            {
                "Flip H", PropertyType::Bool, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(
                    &static_cast<Sprite2D*>(instance)->get_flip_h()); 
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<Sprite2D*>(instance)->set_flip_h(*static_cast<bool*>(data)); 
                }
            },

            {
                "Flip V", PropertyType::Bool, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(
                    &static_cast<Sprite2D*>(instance)->get_flip_v()); 
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<Sprite2D*>(instance)->set_flip_v(*static_cast<bool*>(data));
                }
            },

            {
                "Sorting Layer", PropertyType::Int, nullptr,
                // getter_func
                [](void* instance) { return static_cast<void*>(
                    &static_cast<Sprite2D*>(instance)->get_sorting_layer()); 
                },

                // setter_func
                [](void* instance, void* data) { 
                    static_cast<Sprite2D*>(instance)->set_sorting_layer(*static_cast<int*>(data));
                }
            },

            {
                "Texture Path", PropertyType::String, nullptr, 
                // getter_func
                [](void* instance) { return static_cast<void*>(const_cast<std::string*>(
                    &static_cast<Sprite2D*>(instance)->get_texture_path()));
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<Sprite2D*>(instance)->set_texture_path(*static_cast<std::string*>(data)); 
                }
            }
        };
        type_info.type_index = typeid(Sprite2D);
        type_registry[typeid(Sprite2D)] = type_info;
    }


    // Camera
    template<>
    void ByteMirror::register_type<Camera>() {
        TypeInfo type_info;
        type_info.name = std::string("Camera");
        type_info.ancestor = get_type_info(typeid(Element));
        type_info.properties = {
            {
                "Projection Mode", PropertyType::EnumClass,
                ByteMirror::get_type_info(typeid(Salix::ProjectionMode)),
                [](void* instance) { 
                    return static_cast<void*>(
                        const_cast<Salix::ProjectionMode*>(
                            &static_cast<Camera*>(instance)->get_projection_mode()));
                    },

                [](void* instance, void* data) {
                    static_cast<Camera*>(instance)->set_projection_mode(*static_cast<Salix::ProjectionMode*>(data));
                }
            },

            {
                "Field of View", PropertyType::Float, nullptr,
                [](void* instance) {
                    return static_cast<void*>(const_cast<float*>(
                        &static_cast<Camera*>(instance)->get_field_of_view()));
                },

                [](void* instance, void* data) {
                    static_cast<Camera*>(instance)->set_field_of_view(*static_cast<float*>(data));
                }
            },

            {
                "Orthographic Size", PropertyType::Float, nullptr,
                [](void* instance) {
                    return static_cast<void*>(const_cast<float*>(
                        &static_cast<Camera*>(instance)->get_orthographic_size()));
                },

                [](void* instance, void* data) {
                    static_cast<Camera*>(instance)->set_orthographic_size(*static_cast<float*>(data));
                }
            },

            {
                "Near Clip", PropertyType::Float, nullptr,
                [](void* instance) {
                    return static_cast<void*>(const_cast<float*>(
                        &static_cast<Camera*>(instance)->get_near_clip()));
                },

                [](void* instance, void* data) {
                    static_cast<Camera*>(instance)->set_near_clip(*static_cast<float*>(data));
                }
            },

            {
                "Far Clip", PropertyType::Float, nullptr,
                [](void* instance) {
                    return static_cast<void*>(
                        const_cast<float*>(&static_cast<Camera*>(instance)->get_far_clip()));
                },

                [](void* instance, void* data) {
                    static_cast<Camera*>(instance)->set_far_clip(*static_cast<float*>(data));
                }
            },

            // This is a read-only property. We provide an empty setter lambda.
            {
                "View Matrix", PropertyType::GlmMat4, nullptr,
                [](void* instance) {
                    return static_cast<void*>(const_cast<glm::mat4*>(
                        &static_cast<Camera*>(instance)->get_view_matrix()));
                },

                [](void*, void*) { /* Setter is empty as this is a read-only property. */ }
            },

            // This is a read-only property. We provide an empty setter lambda.
            {
                "Projection Matrix", PropertyType::GlmMat4, nullptr,
                [](void* instance) {
                    return static_cast<void*>(const_cast<glm::mat4*>(
                        &static_cast<Camera*>(instance)->get_projection_matrix()));
                },

                [](void*, void*) { /* Setter is empty as this is a read-only property. */ }
            }
        };
        type_info.type_index = typeid(Camera);
        type_registry[typeid(Camera)] = type_info;
    }

    // ProjectionMode
    template<>
    void ByteMirror::register_type<Salix::ProjectionMode>() {
        TypeInfo type_info;
        type_info.name = "Salix::ProjectionMode"; // A descriptive name for reflection, not directly displayed
        type_info.type_index = typeid(Salix::ProjectionMode);
        type_info.properties = {}; // Enums don't have properties
        type_info.ancestor = nullptr;
        ByteMirror::type_registry[typeid(Salix::ProjectionMode)] = type_info;
    }

    void ByteMirror::register_all_types() {

        ByteMirror::register_type<Element>();
        ByteMirror::register_type<RenderableElement>();
        ByteMirror::register_type<ScriptElement>();
        ByteMirror::register_type<Transform>();
        ByteMirror::register_type<ProjectionMode>();
        ByteMirror::register_type<Camera>();
        ByteMirror::register_type<Sprite2D>();
    } 

    std::vector<std::unique_ptr<PropertyHandle>> ByteMirror::create_handles_for(Element* element) {
        std::vector<std::unique_ptr<PropertyHandle>> handles;
        if (!element) {
            return handles;
        }

        const TypeInfo* type_info = get_type_info(typeid(*element));
        if (!type_info) {
            return handles;
        }

        // For every property this type has...
        for (const auto& prop : type_info->properties)
        {
            // ...create a new PropertyHandle_Live, wrap it in a unique_ptr, and add it to our list.
            handles.push_back(std::make_unique<PropertyHandleLive>(&prop, element));
        }

        return handles;
    }


    const TypeInfo* ByteMirror::get_type_info_by_name(const std::string& name) {
        if (name.empty()) return nullptr;

        // Loop through all the registered types in our map
        for (const auto& pair : type_registry) {
        // pair.first is the type_index, pair.second is the TypeInfo struct
        if (pair.second.name == name) {
            // If we find a TypeInfo whose name matches, return a pointer to it
            return &pair.second;
        }
    }

    // If we loop through everything and find no match, return nullptr
    return nullptr;
    }


    std::vector<std::unique_ptr<PropertyHandle>> ByteMirror::create_handles_for_yaml(YAML::Node* entity_node) {

        std::vector<std::unique_ptr<PropertyHandle>> handles;
        if (!entity_node || !(*entity_node)["components"]) {
            return handles;
        }

        // Loop through each component in the YAML data (e.g., Transform, Sprite2D)
        for (auto component_node : (*entity_node)["components"]) {

            // The component name is the first (and only) key in the node
            std::string component_name = component_node.begin()->first.as<std::string>();
            
            // Get the reflection data for this component type by its string name

            const TypeInfo* type_info = get_type_info_by_name(component_name);
            if (!type_info) {
                continue; // Skip if we don't have reflection data for this component
            }

            // The actual properties are in the value part of the component node
            YAML::Node properties_node = component_node.begin()->second;

            // For every property this component type has...
            for (const auto& prop : type_info->properties)
            {
                // ...if the property exists in the YAML file...
                if (properties_node[prop.name])
                {
                    // ...create a new PropertyHandle_YAML and add it to our list.
                    handles.push_back(std::make_unique<PropertyHandleYaml>(&prop, &properties_node));
                }
            }
        }

        return handles;
    }

    

} // namespace Salix