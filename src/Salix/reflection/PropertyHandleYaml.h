// Salix/reflection/PropertyHandleYaml.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/reflection/PropertyHandle.h>
#include <Salix/reflection/YamlConverters.h>
#include <yaml-cpp/yaml.h>


namespace Salix {

    // Concrete implementation of PropertyHandle for YAML data nodes.
    // This class wraps a pointer to a YAML::Node that represents a component
    // (e.g., the 'Transform' node) and reads/writes property values
    // directly to and from that node.
    class SALIX_API PropertyHandleYaml : public PropertyHandle {
        public:

        // Constructor: Takes the reflection info and a pointer to the YAML node.
        PropertyHandleYaml(const Property* property_info, YAML::Node* component_node);

        // Default the virtual destructor.
        ~PropertyHandleYaml() override = default;

        // Override the pure virtual functions from the base class.
        PropertyValue get_value() const override;
        void set_value(const PropertyValue& value) override;

    private:
        // A pointer to the YAML node that this handle operates on.
        YAML::Node* component_node;

    };
}