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
        static std::string to_upper(std::string str);

        // Converts the entire string to lowercase.
        // Example: "Hello World" -> "hello world"
        static std::string to_lower(std::string str);

        // Converts the first character of each word to uppercase.
        // Example: "hello world" -> "Hello World"
        static std::string to_title_case(std::string str);

        // Converts the first character of the string to uppercase and the rest to lowercase.
        // Example: "hELLO wORLD" -> "Hello world"
        static std::string capitalize(std::string str);
    };

} // namespace Salix
