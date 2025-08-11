// Editor/reflection/ui/TypeDrawer.h
#pragma once
#include <Salix/serialization/YamlConverters.h>
#include <functional>
#include <unordered_map>
#include <Editor/EditorAPI.h>
#include <Salix/reflection/ByteMirror.h>


namespace Salix {
    class Element;
    class PropertyHandle;

    using DrawFunc = std::function<void(const Property& prop, Element* element)>;

    
    class EDITOR_API TypeDrawer {
    public:
    
        static void register_drawer(PropertyType type, DrawFunc draw_func) {
            type_drawer_registry[type] = draw_func;
        }

        static const DrawFunc& get_drawer(PropertyType type) {
            return type_drawer_registry.at(type);
        }

        static void register_all_type_drawers();

        // Draws the appropriate UI for any given property handle.
        static void draw_property(const char* label,PropertyHandle& handle);
        static void draw_yaml_property(const Property& prop, YAML::Node& data_node);
    private:
        static std::unordered_map<PropertyType, DrawFunc> type_drawer_registry;
    };
}