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
#include <Salix/ecs/BoxCollider.h>
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

    // Define the static map for the constructor registry.
    std::unordered_map<std::string, constructor_func> ByteMirror::constructor_registry;

    // Add the implementation for the factory functions.
    void ByteMirror::register_constructor(const std::string& name, constructor_func func) {
        constructor_registry[name] = func;
    }

    std::vector<Property> ByteMirror::get_all_properties_for_type(const TypeInfo* type_info) {
        std::vector<Property> all_properties;
        const TypeInfo* current_type = type_info;

        // Keep walking up the ancestor chain until we hit the top (nullptr)
        while (current_type) {
            // Add the properties from the current type to our list
            all_properties.insert(all_properties.end(),
                                current_type->properties.begin(),
                                current_type->properties.end());
            // Move to the parent
            current_type = current_type->ancestor;
        }
        return all_properties;
    }


    Element* ByteMirror::create_element_by_name(const std::string& name) {
        if (constructor_registry.count(name)) {
            // If we found a constructor for this name, call it.
            return constructor_registry.at(name)();
        }
        // Return nullptr if no constructor is registered for that name.
        return nullptr;
    }
    
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
                "id",                   // The name that will appear in the editor
                PropertyType::UInt64,   // The new type we added for 64-bit integers
                nullptr,                // No contained_type_info is needed

                // Getter: This lambda calls the new get_id_as_ptr() method.
                [](void* instance) {
                    return (void*)static_cast<Element*>(instance)->get_id_as_ptr();
                },

                // Setter: This lambda is empty, making the ID read-only in the editor.
                [](void* instance, void* data) {(void) data; (void) instance;}
            },
            {
                "name",
                PropertyType::String,
                nullptr,
                // Getter
                [](void* instance) {
                    return static_cast<void*>(const_cast<std::string*>(
                        &static_cast<Element*>(instance)->get_name()));
                },

                // Setter
                [](void* instance, void* data) {
                    static_cast<Element*>(instance)->set_name(*static_cast<std::string*>(data));
                }
            }
        };
        

        type_registry[typeid(Element)] = type_info;
    }
    

    // Step 2: Register the descendents.

    // Boxcollider
    template<>
    void ByteMirror::register_type<BoxCollider>() {
        TypeInfo type_info;
        type_info.name = std::string("BoxCollider");
        type_info.ancestor = get_type_info(typeid(Element));
        type_info.properties = {
            {
                "size", PropertyType::Vector3, nullptr,
                // getter_func
                [](void* instance) {return static_cast<void*>(const_cast<Vector3*>(
                    &static_cast<BoxCollider*>(instance)->get_size()));
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<BoxCollider*>(instance)->set_size(*static_cast<Vector3*>(data));
                }
            }
        };
        type_info.type_index = typeid(BoxCollider);
        type_registry[typeid(BoxCollider)] = type_info;
    }

    // Transform
    template<>
    void ByteMirror::register_type<Transform>() {
        TypeInfo type_info;
        type_info.name = std::string("Transform");
        type_info.ancestor = get_type_info(typeid(Element));
        type_info.properties = {
            { 
                "position", PropertyType::Vector3, nullptr,
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
                "rotation", PropertyType::Vector3, nullptr,
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
                "scale", PropertyType::Vector3, nullptr,
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

    // RenderableElement2D
    template<>
    void ByteMirror::register_type<RenderableElement2D>() {
        TypeInfo type_info;
        type_info.name = "RenderableElement2D";
        // Its ancestor is RenderableElement.
        type_info.ancestor = get_type_info(typeid(RenderableElement));
        type_info.type_index = typeid(RenderableElement2D);
        type_registry[typeid(RenderableElement2D)] = type_info;
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
                "script_name", PropertyType::String, nullptr,
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
                "color", PropertyType::Color, nullptr,
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
                "offset", PropertyType::Vector2, nullptr,
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
                "use_entity_rotation", PropertyType::Bool, nullptr,
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
                "pivot", PropertyType::Vector2, nullptr,
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
                "flip_h", PropertyType::Bool, nullptr,
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
                "flip_v", PropertyType::Bool, nullptr,
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
                "sorting_layer", PropertyType::Int, nullptr,
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
                "texture_path", PropertyType::String, nullptr, 
                // getter_func
                [](void* instance) { return static_cast<void*>(const_cast<std::string*>(
                    &static_cast<Sprite2D*>(instance)->get_texture_path()));
                },

                // setter_func
                [](void* instance, void* data) {
                    static_cast<Sprite2D*>(instance)->set_texture_path(*static_cast<std::string*>(data)); 
                },
                UIHint::ImageFile
            },
            {
                "width", PropertyType::Int, nullptr,
                // Getter for the texture width
                [](void* instance) {
                    // Use a stable memory location for the return value
                    thread_local static int value; 
                    value = static_cast<Sprite2D*>(instance)->get_texture_width();
                    return &value;
                },
                // Empty setter makes this property read-only in the UI
                [](void*, void*) {}
            },
            {
                "height", PropertyType::Int, nullptr,
                // Getter for the texture height
                [](void* instance) {
                    // Use a stable memory location for the return value
                    thread_local static int value; 
                    value = static_cast<Sprite2D*>(instance)->get_texture_height();
                    return &value;
                },
                // Empty setter makes this property read-only in the UI
                [](void*, void*) {}
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
                "projection_mode", PropertyType::EnumClass,
                ByteMirror::get_type_info(typeid(Salix::ProjectionMode)),
                // getter_func
                [](void* instance) {
                    // Create a stable memory location to hold our integer.
                    thread_local static int value;
                    // Get the enum, cast its value to an int, and store it.
                    value = static_cast<int>(static_cast<Camera*>(instance)->get_projection_mode());
                    // Return a stable pointer to our copied integer value.
                    return &value;
                },
                // setter_func
                [](void* instance, void* data) {
                    // 1. Get the integer value from the UI's void pointer.
                    int value_as_int = *static_cast<int*>(data);
                    // 2. Safely cast the integer to the proper enum type.
                    Salix::ProjectionMode mode = static_cast<Salix::ProjectionMode>(value_as_int);
                    // 3. Call the original, type-safe setter function.
                    static_cast<Camera*>(instance)->set_projection_mode(mode);
                }
            },

            {
                "field_of_view", PropertyType::Float, nullptr,
                [](void* instance) {
                    return static_cast<void*>(const_cast<float*>(
                        &static_cast<Camera*>(instance)->get_field_of_view()));
                },

                [](void* instance, void* data) {
                    static_cast<Camera*>(instance)->set_field_of_view(*static_cast<float*>(data));
                }
            },

            {
                "orthographic_size", PropertyType::Float, nullptr,
                [](void* instance) {
                    return static_cast<void*>(const_cast<float*>(
                        &static_cast<Camera*>(instance)->get_orthographic_size()));
                },

                [](void* instance, void* data) {
                    static_cast<Camera*>(instance)->set_orthographic_size(*static_cast<float*>(data));
                }
            },

            {
                "near_clip", PropertyType::Float, nullptr,
                [](void* instance) {
                    return static_cast<void*>(const_cast<float*>(
                        &static_cast<Camera*>(instance)->get_near_clip()));
                },

                [](void* instance, void* data) {
                    static_cast<Camera*>(instance)->set_near_clip(*static_cast<float*>(data));
                }
            },

            {
                "far_clip", PropertyType::Float, nullptr,
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
                "view_matrix", PropertyType::GlmMat4, nullptr,
                [](void* instance) {
                    return static_cast<void*>(const_cast<glm::mat4*>(
                        &static_cast<Camera*>(instance)->get_view_matrix()));
                },

                [](void*, void*) { /* Setter is empty as this is a read-only property. */ }
            },

            // This is a read-only property. We provide an empty setter lambda.
            {
                "projection_matrix", PropertyType::GlmMat4, nullptr,
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
        ByteMirror::register_type<RenderableElement2D>();
        ByteMirror::register_type<ScriptElement>();
        ByteMirror::register_type<Transform>();
        ByteMirror::register_type<ProjectionMode>();
        ByteMirror::register_type<Camera>();
        ByteMirror::register_type<Sprite2D>();
        ByteMirror::register_type<BoxCollider>();
        ByteMirror::register_type<CppScript>();

        // REGISTER CONSTRUCTORS
        ByteMirror::register_constructor("Transform",   []() -> Element* { return new Transform(); });
        ByteMirror::register_constructor("Sprite2D",    []() -> Element* { return new Sprite2D(); });
        ByteMirror::register_constructor("Camera",      []() -> Element* { return new Camera(); });
        ByteMirror::register_constructor("BoxCollider", []() -> Element* { return new BoxCollider(); });
        ByteMirror::register_constructor("CppScript",   []() -> Element* { return new CppScript();});
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
        for (const auto& prop : get_all_properties_for_type(type_info)) {
            // ...create a new PropertyHandle_Live, wrap it in a unique_ptr, and add it to our list.
            handles.push_back(std::make_unique<PropertyHandleLive>(prop, element));
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
        if (!entity_node || !(*entity_node)["elements"]) {
            return handles;
        }

        // Loop through each element in the YAML data (e.g., Transform, Sprite2D)
        for (auto element_node : (*entity_node)["elements"]) {

            // The element name is the first (and only) key in the node
            std::string element_name = element_node.begin()->first.as<std::string>();
            
            // Get the reflection data for this element type by its string name

            const TypeInfo* type_info = get_type_info_by_name(element_name);
            if (!type_info) {
                continue; // Skip if we don't have reflection data for this element
            }

            // The actual properties are in the value part of the element node
            YAML::Node properties_node = element_node.begin()->second;

            // For every property this element type has...
            for (const auto& prop : get_all_properties_for_type(type_info))
            {
                // ...if the property exists in the YAML file...
                if (properties_node[prop.name])
                {
                    // ...create a new PropertyHandle_YAML and add it to our list.
                    handles.push_back(std::make_unique<PropertyHandleYaml>(prop, &properties_node));
                }
            }
        }

        return handles;
    }

    

} // namespace Salix