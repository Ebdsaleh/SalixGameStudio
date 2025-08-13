// Editor/reflection/ui/TypeDrawer.cpp
#include <Salix/serialization/YamlConverters.h>
#include <Editor/reflection/ui/TypeDrawer.h>
#include <Salix/reflection/PropertyHandle.h>
#include <Salix/reflection/EnumRegistry.h>
#include <unordered_map>
#include <imgui/imgui.h>
#include <Salix/math/Color.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Point.h>
#include <Salix/ecs/Camera.h>
#include <Salix/rendering/ICamera.h>
#include <glm/glm.hpp>
#include <string>


namespace Salix {

    std::unordered_map<PropertyType, DrawFunc> TypeDrawer::type_drawer_registry;

    void draw_nested_object(const Property& prop, Element* element) {
        const TypeInfo* nested_type_info = prop.contained_type_info;

        if (ImGui::CollapsingHeader(prop.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            // Iterate through all the properties of the nested object.
            for (const auto& nested_prop : nested_type_info->properties) {
                // Here is the recursion! We call the TypeDrawer again to draw the nested property.
                TypeDrawer::get_drawer(nested_prop.type)(nested_prop, element);
            }
        }
    }


    void TypeDrawer::register_all_type_drawers() {
        TypeDrawer::register_drawer(PropertyType::Int, [](const Property& prop, Element* element) {
            int value = *static_cast<int*>(prop.get_data(element));
            if (ImGui::InputInt(prop.name.c_str(), &value)) {
                prop.set_data(element, &value);
            }
        });

        TypeDrawer::register_drawer(PropertyType::Float, [](const Property& prop, Element* element) {
            float value = *static_cast<float*>(prop.get_data(element));
            if (ImGui::InputFloat(prop.name.c_str(), &value)) {
                prop.set_data(element, &value);
            }
        });

        TypeDrawer::register_drawer(PropertyType::Bool, [](const Property& prop, Element* element) {
            bool value = *static_cast<bool*>(prop.get_data(element));
            if (ImGui::Checkbox(prop.name.c_str(), &value)) {
                prop.set_data(element, &value);
            }
        });

        TypeDrawer::register_drawer(PropertyType::Vector2, [](const Property& prop, Element* element) {
            Salix::Vector2 value = *static_cast<Salix::Vector2*>(prop.get_data(element));
            // The DragFloat2 function just needs a pointer to the first float.
            if (ImGui::DragFloat2(prop.name.c_str(), &value.x)) {
                prop.set_data(element, &value);
            }
        });

        TypeDrawer::register_drawer(PropertyType::Vector3, [](const Property& prop, Element* element) {
            Salix::Vector3 value = *static_cast<Salix::Vector3*>(prop.get_data(element));
            if (ImGui::DragFloat3(prop.name.c_str(), &value.x)) {
                prop.set_data(element, &value);
            }
        });

        TypeDrawer::register_drawer(PropertyType::Color, [](const Property& prop, Element* element) {
            // We get a pointer to the color, then dereference it to get the object
            Salix::Color value = *static_cast<Salix::Color*>(prop.get_data(element));
            // We use ColorEdit4 and pass a pointer to the first float, which is 'r'
            if (ImGui::ColorEdit4(prop.name.c_str(), &value.r)) {
                // If the value changes, we call the setter to update the original object
                prop.set_data(element, &value);
            }
        });

        TypeDrawer::register_drawer(PropertyType::Point, [](const Property& prop, Element* element) {
            Salix::Point value = *static_cast<Salix::Point*>(prop.get_data(element));
            if (ImGui::InputInt2(prop.name.c_str(), &value.x)) {
                prop.set_data(element, &value);
            }
        });

        TypeDrawer::register_drawer(PropertyType::String, [](const Property& prop, Element* element) {
            std::string value = *static_cast<std::string*>(prop.get_data(element));
            // We need a temporary buffer.
            char text_buffer[256];
            // This is the correct way to copy the string into the buffer.
            strcpy_s(text_buffer, sizeof(text_buffer), value.c_str());

            // The InputText function takes a char* buffer and its size.
            if (ImGui::InputText(prop.name.c_str(), text_buffer, sizeof(text_buffer))) {
                // If the user changed the text, we create a new std::string from the buffer.
                std::string new_string = text_buffer;
                prop.set_data(element, &new_string);
            }
        });


        TypeDrawer::register_drawer(PropertyType::GlmMat4, [](const Property& prop, Element* element) { 
            // We get the pointer and dereference it to create a local copy.
            glm::mat4 value = *static_cast<glm::mat4*>(prop.get_data(element));

            // A unique ID is needed for each DragFloat4 widget.
            ImGui::PushID(prop.name.c_str());

            // Loop through each of the four columns
            for (int i = 0; i < 4; ++i) {
                // Use a temporary label for each column (e.g., "Column 1")
                char label[16];
                sprintf_s(label, "Col %d", i + 1);

                if (ImGui::DragFloat4(label, &value[i][0])) {
                    prop.set_data(element, &value);
                }
            }
            
            ImGui::PopID();  // Clean up
        });


        TypeDrawer::register_drawer(PropertyType::Class, draw_nested_object);
        TypeDrawer::register_drawer(PropertyType::Struct, draw_nested_object);

        TypeDrawer::register_drawer(PropertyType::Enum, [](const Property& prop, Element* element) {
            int value = *static_cast<int*>(prop.get_data(element));
            if (prop.contained_type_info && prop.contained_type_info->type_index.has_value()) {
                const EnumRegistry::EnumData& enum_data = EnumRegistry::get_enum_data(*prop.contained_type_info->type_index);

                std::vector<const char*> item_names;
                for (const auto& name : enum_data.ordered_names) {
                    item_names.push_back(name.c_str());
                }

                if (ImGui::Combo(prop.name.c_str(), &value, item_names.data(), static_cast<int>(item_names.size()))) {
                    prop.set_data(element, &value);
                }
            }
        });
        
        TypeDrawer::register_drawer(PropertyType::EnumClass, [](const Property& prop, Element* element) {
            int value = *static_cast<int*>(prop.get_data(element));
            if (prop.contained_type_info && prop.contained_type_info->type_index.has_value()) {
                const EnumRegistry::EnumData& enum_data = EnumRegistry::get_enum_data(*prop.contained_type_info->type_index);
            
                std::vector<const char*> item_names;
                for (const auto& name : enum_data.ordered_names) {
                    item_names.push_back(name.c_str());
                }

                if (ImGui::Combo(prop.name.c_str(), &value, item_names.data(), static_cast<int>(item_names.size()))) {
                prop.set_data(element, &value);
                }
            }
        });

    }



    bool Salix::TypeDrawer::draw_property(const char* label, PropertyHandle& handle) {
        // Use the property's name as the label for the ImGui widget
        bool value_changed = false;
        switch (handle.get_type())
        {
            case PropertyType::Int: {
                int value = std::get<int>(handle.get_value());
                if (ImGui::DragInt(label, &value)) {
                    handle.set_value(value);
                    value_changed = true;
                }
                break;
            }

            case PropertyType::Float: {
                float value = std::get<float>(handle.get_value());
                if (ImGui::DragFloat(label, &value, 0.1f)) {
                    handle.set_value(value);
                    value_changed = true;
                }
                break;
            }

            case PropertyType::Bool: {
                bool value = std::get<bool>(handle.get_value());
                if (ImGui::Checkbox(label, &value)) {
                    handle.set_value(value);
                    value_changed = true;
                }
                break;
            }

            case PropertyType::String: {
                std::string value = std::get<std::string>(handle.get_value());
                // ImGui's InputText works with char buffers, so we need this boilerplate
                char buffer[256];
                strncpy_s(buffer, sizeof(buffer), value.c_str(), sizeof(buffer) - 1);
                if (ImGui::InputText(label, buffer, sizeof(buffer))) {
                    handle.set_value(std::string(buffer));
                    value_changed = true;
                }
                break;
            }

            case PropertyType::Vector2: {
                Vector2 value = std::get<Vector2>(handle.get_value());
                if (ImGui::DragFloat2(label, &value.x, 0.1f)) {
                    handle.set_value(value);
                    value_changed = true;
                }
                break;
            }

            case PropertyType::Vector3: {
                Vector3 value = std::get<Vector3>(handle.get_value());
                if (ImGui::DragFloat3(label, &value.x, 0.1f)) {
                    handle.set_value(value);
                    value_changed = true;
                }
                break;
            }

            case PropertyType::Color: {
                // 1. Get the current color value from the handle.
                Color current_color = std::get<Color>(handle.get_value());

                // 2. Use your helper function to convert it to an ImVec4 for ImGui.
                ImVec4 im_color = current_color.to_imvec4();

                // 3. Pass a pointer to the ImVec4's data to the color editor.
                if (ImGui::ColorEdit4(label, &im_color.x)) {
                    
                    // 4. If the user changed the color, create a new Salix::Color from the result.
                    Color new_color(im_color.x, im_color.y, im_color.z, im_color.w);
                    
                    // 5. Set the new value.
                    handle.set_value(new_color);
                    value_changed = true;
                }
                break;
            }

            case PropertyType::Enum: {
                int current_value_int = std::get<int>(handle.get_value());
                auto enum_data = EnumRegistry::get_enum_data_as_ptr(handle.get_contained_type_info()->type_index.value());

                if (enum_data) {
                    // Get the string name for the currently selected integer value.
                    const char* current_item_name = enum_data->get_name(current_value_int).c_str();
                    
                    // Start the combo box, displaying the current name.
                    if (ImGui::BeginCombo(label, current_item_name)) {
                        // Loop through all possible enum names from the registry.
                        for (const auto& name : enum_data->get_names()) {
                            bool is_selected = (current_item_name == name);
                            
                            // Draw a selectable item for this name.
                            if (ImGui::Selectable(name.c_str(), is_selected)) {
                                // If the user clicks it, set the value to the integer for THIS name.
                                handle.set_value(enum_data->get_value(name));
                                value_changed = true;
                            }

                            // Set focus to the currently selected item when the dropdown opens.
                            if (is_selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                break;
            }
            case PropertyType::EnumClass: {
                int current_value_int = std::get<int>(handle.get_value());
                auto enum_data = EnumRegistry::get_enum_data_as_ptr(handle.get_contained_type_info()->type_index.value());

                
                if (enum_data) {
                    // Get the string name for the currently selected integer value.
                    std::string current_item_name_str = enum_data->get_name(current_value_int);
                    const char* current_item_name = current_item_name_str.c_str();
                    
                    // Start the combo box, displaying the current name.
                    if (ImGui::BeginCombo(label, current_item_name)) {
                        // Loop through all possible enum names from the registry.
                        for (const auto& name : enum_data->get_names()) {
                            bool is_selected = (current_item_name_str == name);
                            
                            // Draw a selectable item for this name.
                            if (ImGui::Selectable(name.c_str(), is_selected)) {
                                // Debug output:
                                int new_value_int = enum_data->get_value(name);
                                std::cout << "[TypeDrawer] User selected '" << name 
                                    << "', preparing to set new value to: " << new_value_int << std::endl;
                                // End of Debug output
                                // If the user clicks it, set the value to the integer for THIS name.
                                handle.set_value(enum_data->get_value(name));
                                value_changed = true;
                            }

                            // Set focus to the currently selected item when the dropdown opens.
                            if (is_selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                break;
            }
            

            case PropertyType::GlmMat4: {
                // This is a read-only property, so we just display it
                ImGui::Text("%s: (Read-only Matrix)", label);
                // You could add code here to display the matrix values if you wanted
                break;
            }

            // Add cases for Point and Rect here following the same pattern...

            default:
                ImGui::Text("%s: (UI not implemented for this type)", label);
                break;
        }
        return value_changed; // Return the flag
    }

    void TypeDrawer::draw_yaml_property(const Property& prop, YAML::Node& data_node) {
        // Use the property's name as the key into the YAML node
        const std::string& name = prop.name;

        switch (prop.type)
        {
            case PropertyType::Int: {
                // Read -> Draw -> Write
                int value = data_node[name] ? data_node[name].as<int>() : 0;
                if (ImGui::DragInt(name.c_str(), &value)) {
                    data_node[name] = value;
                }
                break;
            }

            case PropertyType::Float: {
                // Read -> Draw -> Write
                float value = data_node[name] ? data_node[name].as<float>() : 0.0f;
                if (ImGui::DragFloat(name.c_str(), &value, 0.1f)) {
                    data_node[name] = value;
                }
                break;
            }

            case PropertyType::Bool: {
                // Read -> Draw -> Write
                bool value = data_node[name] ? data_node[name].as<bool>() : false;
                if (ImGui::Checkbox(name.c_str(), &value)) {
                    data_node[name] = value;
                }
                break;
            }

            case PropertyType::String: {
                // Read -> Draw -> Write
                std::string value = data_node[name] ? data_node[name].as<std::string>() : "";
                char buffer[256];
                strncpy_s(buffer, sizeof(buffer), value.c_str(), sizeof(buffer) - 1);
                if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer))) {
                    data_node[name] = std::string(buffer);
                }
                break;
            }

            case PropertyType::Vector2: {
                // Read -> Draw -> Write
                Vector2 value = data_node[name] ? data_node[name].as<Vector2>() : Vector2();
                float values[2] = { value.x, value.y };
                if (ImGui::DragFloat2(name.c_str(), values, 0.1f)) {
                    data_node[name] = Vector2(values[0], values[1]);
                }
                break;
            }

            case PropertyType::Vector3: {
                // Read -> Draw -> Write
                Vector3 value = data_node[name] ? data_node[name].as<Vector3>() : Vector3();
                float values[3] = { value.x, value.y, value.z };
                if (ImGui::DragFloat3(name.c_str(), values, 0.1f)) {
                    data_node[name] = Vector3(values[0], values[1], values[2]);
                }
                break;
            }

            case PropertyType::Color: {
                // Read -> Draw -> Write
                Color value = data_node[name] ? data_node[name].as<Color>() : Color();
                float values[4] = { value.r, value.g, value.b, value.a };
                if (ImGui::ColorEdit4(name.c_str(), values)) {
                    data_node[name] = Color(values[0], values[1], values[2], values[3]);
                }
                break;
            }

            default:
                ImGui::Text("Unsupported property type");
                break;
        }
    }

}   // namespace Salix