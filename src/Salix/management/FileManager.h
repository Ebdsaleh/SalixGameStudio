// =================================================================================
// Filename:    Salix/management/FileManager.h
// Author:      SalixGameStudio
// Description: A utility class for handling platform-specific file system operations.
// =================================================================================
#pragma once

#include <Salix/core/Core.h>
#include <string>

namespace Salix {

    // Note: This class is simple enough (no complex std:: members) that it
    // doesn't need the PIMPL idiom, even though it's exported.
    class SALIX_API FileManager {
    public:
        // This class can be all static methods, as it doesn't need to hold state.
        // This means we don't even need a constructor.

        // Checks if a file or directory exists at the given path.
        static bool path_exists(const std::string& path);

        // Creates a directory at the given path.
        // Returns true on success or if the directory already exists.
        static bool create_directory(const std::string& path);

        // Creates a directory, including all necessary parent directories.
        static bool create_directories(const std::string& path);
    };

} // namespace Salix