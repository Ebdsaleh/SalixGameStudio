// Salix/reflection/ByteMirror.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/reflection/PropertyHandle.h>
#include <string>
#include <vector>
#include <cstddef> // Required for offsetof
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <optional>

namespace Salix {
    // An enum to represent the types we can edit in the UI.
    // We'll add more as needed (e.g., Color, Quaternion, etc.)
    enum class PropertyType
    {   
        Int,
        Float,
        Vector2,
        Vector3,
        Point,
        Rect,
        Bool,
        String,
        Color,
        Class,
        Struct,
        Enum,
        EnumClass,
        GlmMat4
    };

    // Forward delcare to break dependency loop.
    struct TypeInfo;


    // A generic function that takes a component instance and returns a pointer to the property's data.
    using getter_func = std::function<void*(void* type_instance)>;

    // A generic function that takes a component instance and a pointer to the new data to set.
    using setter_func = std::function<void(void* type_instance, void* data_to_set)>;


    // Describes a single editable property of a component.
    struct Property
    {
        std::string name;
        PropertyType type;
        const TypeInfo* contained_type_info = nullptr;
        getter_func get_data;
        setter_func set_data;
    };

    // Contains all the reflection information for a given component type.
    struct TypeInfo
    {
        std::string name;
        std::vector<Property> properties;
        const TypeInfo* ancestor = nullptr;
        std::optional<std::type_index> type_index;

    };


    class SALIX_API ByteMirror {

        public:
           
            // Registers a new component type with the reflection system.
            template<typename T>
            static void register_type();

            // Retrieves the reflection data for a given type.
            static const TypeInfo* get_type_info(std::type_index type_index)
            {
                if (type_registry.count(type_index))
                {
                    return &type_registry.at(type_index);
                }
                return nullptr;
            }

            static void register_all_types();

            // Creates a vector of property handles for a given live element.
            static std::vector<std::unique_ptr<PropertyHandle>> create_handles_for(Element* element);
        private:
            // The static registry mapping a type_index to its reflection data.
            static std::unordered_map<std::type_index, TypeInfo> type_registry;
        };

        // Will need to define the static map in a corresponding .cpp file.
        // In ByteMirror.cpp:
        // std::unordered_map<std::type_index, TypeInfo> ByteMirror::type_registry;
}  // namespace Salix