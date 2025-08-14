// =================================================================================
// Filename:    Salix/core/StringUtils.h
// Author:      SalixGameStudio
// Description: Provides a set of static utility functions for string manipulation.
// =================================================================================
#pragma once
#include <Salix/core/Core.h>
#include <string>

namespace Salix {

    class SALIX_API StringUtils {
    public:
        // Converts the entire string to uppercase.
        // Example: "hello world" -> "HELLO WORLD"
        static std::string to_upper(std::string str, bool de_snake_case = false);

        // Converts the entire string to lowercase.
        // Example: "Hello World" -> "hello world"
        static std::string to_lower(std::string str, bool de_snake_case = false);

        // Converts the first character of each word to uppercase.
        // Example: "hello world" -> "Hello World"
        static std::string to_title_case(std::string str, bool de_snake_case = false);

        // Converts the first character of the string to uppercase and the rest to lowercase.
        // Example: "hELLO wORLD" -> "Hello world"
        static std::string capitalize(std::string str, bool de_snake_case = false);

        // --- Snake Case Utilities ---
        // Returns true only if the entire string is valid snake_case.
        static bool is_snake_case(const std::string& str);
        // Returns true if any part of the string contains a valid snake_case segment.
        static bool contains_snake_case(const std::string& str);
        // Replaces valid underscores between alphanumeric characters with spaces.
        static std::string convert_from_snake_case(std::string str);
    
        // --- Pascal Case Utilities ---
        // Returns true only if the string is valid PascalCase (e.g., "BoxCollider").
        static bool is_pascal_case(const std::string& str);

        // Returns true if a PascalCase segment is found (e.g., an uppercase letter following a lowercase one).
        static bool contains_pascal_case(const std::string& str);

        // Converts a PascalCase string to a space-separated one (e.g., "BoxCollider" -> "Box Collider").
        static std::string convert_from_pascal_case(std::string str);
    };

} // namespace Salix
