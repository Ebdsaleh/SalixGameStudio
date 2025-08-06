// Salix/reflection/ui/TypeDrawer.h
#pragma once
#include <functional>
#include <unordered_map>
#include <Salix/core/Core.h>
#include <Salix/reflection/ByteMirror.h>

namespace Salix {
    class Element;
    class PropertyHandle;

    using DrawFunc = std::function<void(const Property& prop, Element* element)>;

    
    class SALIX_API TypeDrawer {
    public:
    
        static void register_drawer(PropertyType type, DrawFunc draw_func) {
            type_drawer_registry[type] = draw_func;
        }

        static const DrawFunc& get_drawer(PropertyType type) {
            return type_drawer_registry.at(type);
        }

        static void register_all_type_drawers();

        // Draws the appropriate UI for any given property handle.
        static void draw_property(PropertyHandle& handle);
    private:
        static std::unordered_map<PropertyType, DrawFunc> type_drawer_registry;
    };
}