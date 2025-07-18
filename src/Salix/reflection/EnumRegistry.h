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
        };

        // Registers an enum type with its string names
        static void register_enum(std::type_index type_index, EnumData&& data) {
            enum_data_registry[type_index] = std::move(data);
        }

        // Retrieves the data for an enum type
        static const EnumData& get_enum_data(std::type_index type_index) {
            return enum_data_registry.at(type_index);
        }

    private:
        static std::unordered_map<std::type_index, EnumData> enum_data_registry;
    };
}