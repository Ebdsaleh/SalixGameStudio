// =================================================================================
// Filename:    Salix/serializers/ISerializer.h
// Author:      SalixGameStudio
// Description: Declares the abstract interface for any data serializer.
// =================================================================================
#pragma once

#include <Salix/core/Core.h>
#include <string>

namespace Salix {

    class SALIX_API ISerializer {
    public:
        virtual ~ISerializer() = default;

        // --- Core Serializer Contract ---

        // Loads and parses data from a file on disk. Returns true on success.
        virtual bool parse_file(const std::string& file_path) = 0;

        // Parses data from a string in memory. Returns true on success.
        virtual bool parse_string(const std::string& data_string) = 0;
        
        // --- NOTE ---
        // We will add getter methods here later, like get_string, get_int, etc.
        // For now, this defines the core loading functionality.
    };

} // namespace Salix