// =================================================================================
// Filename:    Salix/core/StringUtils.cpp
// Author:      SalixGameStudio
// Description: Implements the StringUtils class.
// =================================================================================
#include <Salix/core/StringUtils.h>
#include <algorithm> // For std::transform
#include <cctype>    // For std::toupper, std::tolower, std::isspace

namespace Salix {

    std::string StringUtils::to_upper(std::string str, bool de_snake_case) {
        if (de_snake_case) {
            str = convert_from_snake_case(str);
        }
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c){ return std::toupper(c); });
        return str;
    }

    std::string StringUtils::to_lower(std::string str, bool de_snake_case) {
        if (de_snake_case) {
            str = convert_from_snake_case(str);
        }
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c){ return std::tolower(c); });
        return str;
    }

    std::string StringUtils::to_title_case(std::string str, bool de_snake_case) {
        if (de_snake_case) {
            str = convert_from_snake_case(str);
        }
        if (str.empty()) {
            return "";
        }

        bool new_word = true;
        std::transform(str.begin(), str.end(), str.begin(),
            [&new_word](unsigned char c) -> unsigned char {
                if (std::isspace(c)) {
                    new_word = true;
                    return c;
                }
                if (new_word) {
                    new_word = false;
                    return static_cast<unsigned char>(std::toupper(c));
                }
                return static_cast<unsigned char>(std::tolower(c));
            });
        
        return str;
    }

    std::string StringUtils::capitalize(std::string str, bool de_snake_case) {
        if (de_snake_case) {
            str = convert_from_snake_case(str);
        }
        if (str.empty()) {
            return "";
        }

        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c){ return std::tolower(c); });
        str[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[0])));
        return str;
    }

    // --- Snake Case Utilities ---

    bool StringUtils::is_snake_case(const std::string& str) {
        if (str.empty()) {
            return false;
        }

        bool found_underscore = false;
        for (size_t i = 0; i < str.length(); ++i) {
            const char c = str[i];
            if (c == '_') {
                found_underscore = true;
                // Check for valid neighbors
                if (i == 0 || i == str.length() - 1 || !std::isalnum(str[i - 1]) || !std::isalnum(str[i + 1])) {
                    return false; // Invalid underscore placement
                }
            } else if (!std::isalnum(c)) {
                return false; // Contains invalid characters
            }
        }
        // Must contain at least one underscore to be considered snake_case
        return found_underscore;
    }

    bool StringUtils::contains_snake_case(const std::string& str) {
        for (size_t i = 1; i < str.length() - 1; ++i) {
            if (str[i] == '_' && std::isalnum(str[i - 1]) && std::isalnum(str[i + 1])) {
                return true; // Found a valid snake_case segment
            }
        }
        return false;
    }

    std::string StringUtils::convert_from_snake_case(std::string str) {
        if (str.empty()) {
            return "";
        }
        
        std::string result;
        result.reserve(str.length()); // Pre-allocate memory for efficiency

        for (size_t i = 0; i < str.length(); ++i) {
            // Check if it's a valid underscore to replace
            if (str[i] == '_' && i > 0 && i < str.length() - 1 && std::isalnum(str[i - 1]) && std::isalnum(str[i + 1])) {
                result += ' ';
            } else {
                result += str[i];
            }
        }
        return result;
    }
} // namespace Salix
