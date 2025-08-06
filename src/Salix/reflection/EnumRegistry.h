// Salix/reflection/EnumRegistry.h
#pragma once
#include <Salix/core/Core.h>
#include <unordered_map>
#include <string>
#include <typeindex>

namespace Salix {
    class SALIX_API EnumRegistry {
    public:
        // A struct to hold the string representations of the enum values
        struct EnumData {
            std::unordered_map<int, std::string> value_to_string;
            std::vector<std::string> ordered_names;
            std::string get_name(int value) const {
                if (value_to_string.count(value)) return value_to_string.at(value);
                return "Unknown";
            }

            int get_value(const std::string& name) const {
                for (const auto& pair : value_to_string) {
                    if (pair.second == name) return pair.first;
                }
                return -1; // Or some other invalid value
            }

             const std::vector<std::string>& get_names() const {
                return ordered_names;
            }
        };

        // Registers an enum type with its string names
        static void register_enum(std::type_index type_index, EnumData&& data) {
            enum_data_registry[type_index] = std::move(data);
        }

        // Retrieves the data for an enum type
        static const EnumData& get_enum_data(std::type_index type_index) {
            return enum_data_registry.at(type_index);
        }
        
        // Retrieves the data for an enum type. Now returns a pointer.
        static const EnumData* get_enum_data_as_ptr(std::type_index type_index);
        
        static void register_all_enums();

    private:
        static std::unordered_map<std::type_index, EnumData> enum_data_registry;
    };
}