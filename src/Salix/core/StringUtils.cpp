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


    // --- Pascal Case Utilities ---
    
    bool StringUtils::is_pascal_case(const std::string& str) {
        if (str.empty() || !std::isupper(str[0])) {
            return false; // Must not be empty and must start with an uppercase letter.
        }
        // Check that all other characters are alphanumeric.
        for (char c : str) {
            if (!std::isalnum(c)) {
                return false;
            }
        }
        return true;
    }



    bool StringUtils::contains_pascal_case(const std::string& str) {
        if (str.length() < 2) {
            return false;
        }
        // Look for the pattern of a lowercase letter followed by an uppercase letter.
        for (size_t i = 1; i < str.length(); ++i) {
            if (std::islower(str[i - 1]) && std::isupper(str[i])) {
                return true;
            }
        }
        return false;
    }


    

    std::string StringUtils::convert_from_pascal_case(std::string str) {
        if (str.empty()) {
            return "";
        }

        std::string result;
        result += str[0]; // Start with the first character

        for (size_t i = 1; i < str.length(); ++i) {
            char current = str[i];
            char previous = str[i - 1];

            // Case 1: An uppercase letter follows a lowercase letter (e.g., the 'C' in "BoxCollider")
            bool is_transition_lower_to_upper = std::islower(previous) && std::isupper(current);

            // Case 2: A number follows a letter (e.g., the '2' in "Sprite2D")
            bool is_transition_alpha_to_digit = std::isalpha(previous) && std::isdigit(current);

            // Case 3: An uppercase letter follows a number... but only if it's the start of a new word.
            // This is how we handle "Version2Alpha" correctly, but not "Sprite2D".
            bool is_transition_digit_to_word = false;
            if (std::isdigit(previous) && std::isupper(current)) {
                // Look ahead: if the character after this one is lowercase, then it's a new word.
                if ((i + 1) < str.length() && std::islower(str[i + 1])) {
                    is_transition_digit_to_word = true; // e.g., the 'A' in "Version2Alpha"
                }
            }
            
            if (is_transition_lower_to_upper || is_transition_alpha_to_digit || is_transition_digit_to_word) {
                result += ' ';
            }

            result += current;
        }
        return result;
    }

} // namespace Salix
