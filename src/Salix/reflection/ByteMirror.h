// Salix/reflection/ByteMirror.h
#pragma once
#include <Salix/core/Core.h>

#include <string>
#include <vector>
#include <cstddef> // Required for offsetof
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <optional>
#include <memory>
#include <yaml-cpp/yaml.h>



namespace Salix {

    class PropertyHandle;
    class Element;
    // An enum to represent the types we can edit in the UI.
    // We'll add more as needed (e.g., Color, Quaternion, etc.)
    enum class PropertyType
    {   
        Int,
        UInt64,
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


    // A generic function that takes a element instance and returns a pointer to the property's data.
    using getter_func = std::function<void*(void* type_instance)>;

    // A generic function that takes a element instance and a pointer to the new data to set.
    using setter_func = std::function<void(void* type_instance, void* data_to_set)>;

    // A type definition for a function that constructs an Element.
    using constructor_func = std::function<Element*()>;

    // Used for telling the TypeDrawer what to draw the property as, 
    // ie. does it need a dialog box to find a file or can it just draw it's values...
    enum class UIHint {
    None,
    FilePath,
    ImageFile,
    MultilineText,
    ColorEdit,
    AudioFile,
    TextFile,
    SourceFile
    };
    // Describes a single editable property of a element.
    struct Property
    {
        std::string name;
        PropertyType type;
        const TypeInfo* contained_type_info = nullptr;
        getter_func get_data;
        setter_func set_data;
        UIHint hint = UIHint::None;
        
    };

    // Contains all the reflection information for a given element type.
    struct TypeInfo
    {
        std::string name;
        std::vector<Property> properties;
        const TypeInfo* ancestor = nullptr;
        std::optional<std::type_index> type_index;

    };


    class SALIX_API ByteMirror {

        public:
           
            // Registers a new element type with the reflection system.
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
            
            static const TypeInfo* get_type_info_by_name(const std::string& name);

            // Creates a vector of property handles for a given live element.
            static std::vector<std::unique_ptr<PropertyHandle>> create_handles_for(Element* element);
            
            // Creates a vector of property handles for a given YAML entity node.
            static std::vector<std::unique_ptr<PropertyHandle>> create_handles_for_yaml(YAML::Node* entity_node);

            // NEW: Factory functions to create elements from a string name.
            static void register_constructor(const std::string& name, constructor_func func);
            static Element* create_element_by_name(const std::string& name);
            
            // Recursively collects all properties from a type and its ancestors.
            static std::vector<Property> get_all_properties_for_type(const TypeInfo* type_info);

        private:
            // The static registry mapping a type_index to its reflection data.
            static std::unordered_map<std::type_index, TypeInfo> type_registry;

            // A registry to store the constructor for each component type.
            static std::unordered_map<std::string, constructor_func> constructor_registry;
        };

        // Will need to define the static map in a corresponding .cpp file.
        // In ByteMirror.cpp:
        // std::unordered_map<std::type_index, TypeInfo> ByteMirror::type_registry;
}  // namespace Salix