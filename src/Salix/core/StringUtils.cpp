// =================================================================================
// Filename:    Salix/core/StringUtils.cpp
// Author:      SalixGameStudio
// Description: Implements the StringUtils class.
// =================================================================================
#include <Salix/core/StringUtils.h>
#include <algorithm> // For std::transform
#include <cctype>    // For std::toupper, std::tolower, std::isspace

namespace Salix {

    std::string StringUtils::to_upper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c){ return std::toupper(c); });
        return str;
    }

    std::string StringUtils::to_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c){ return std::tolower(c); });
        return str;
    }

    std::string StringUtils::to_title_case(std::string str) {
        if (str.empty()) {
            return "";
        }

        bool new_word = true;
        std::transform(str.begin(), str.end(), str.begin(),
            [&new_word](unsigned char c) -> unsigned char { // Explicitly set return type
                if (std::isspace(c)) {
                    new_word = true;
                    return c;
                }
                
                if (new_word) {
                    new_word = false;
                    // Cast the result back to the correct type
                    return static_cast<unsigned char>(std::toupper(c));
                }
                
                // Cast the result back to the correct type
                return static_cast<unsigned char>(std::tolower(c));
            });
        
        return str;
    }

    std::string StringUtils::capitalize(std::string str) {
        if (str.empty()) {
            return "";
        }

        // First, convert the whole string to lowercase.
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c){ return std::tolower(c); });

        // Then, capitalize the first letter.
        str[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[0])));

        return str;
    }

} // namespace Salix
