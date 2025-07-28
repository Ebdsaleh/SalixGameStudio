// Salix/reflection/ByteMirror.cpp
#include <cstddef>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <Salix/reflection/ByteMirror.h>
#include <Salix/ecs/Camera.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/ecs/CppScript.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/Transform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Salix {

    // Define the static map here, outside of the class definition.
    // A static class cannot have a constructor.
    std::unordered_map<std::type_index, TypeInfo> ByteMirror::type_registry;
    
    // Step 1: Register the base classes first.
    // The Element class has no properties to expose to the editor in this example.
    template<>
    void ByteMirror::register_type<Element>() {
    TypeInfo type_info;
    type_info.name = "Element";
    type_info.properties = {};
    type_info.ancestor = nullptr;
    type_info.type_index = typeid(Element);

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
        type_info.ancestor = get_type_info(typeid(RenderableElement));
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

    

} // namespace Salix