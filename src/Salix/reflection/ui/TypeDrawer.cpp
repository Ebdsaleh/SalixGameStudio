// Salix/reflection/ui/TypeDrawer.cpp
#include <Salix/reflection/ui/TypeDrawer.h>
#include <Salix/reflection/EnumRegistry.h>
#include <unordered_map>
#include <imgui/imgui.h>
#include <Salix/math/Color.h>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Point.h>
#include <Salix/ecs/Camera.h>
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

                if (ImGui::Combo(prop.name.c_str(), &value, item_names.data(), item_names.size())) {
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

                if (ImGui::Combo(prop.name.c_str(), &value, item_names.data(), item_names.size())) {
                prop.set_data(element, &value);
                }
            }
        });

    }




}   // namespace Salix