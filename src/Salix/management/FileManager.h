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

        // Converts a full, absolute path to a path relative to the project root.
        // Returns the original absolute path if it's not within the project root.
        static std::string convert_to_relative(const std::string& project_root, const std::string& absolute_path);
        static std::string convert_to_relative_path(const std::string& project_root, const std::string& absolute_path);
        // Copies a file from a source path to a destination path.
        // Returns true on success.
        static bool copy_file(const std::string& source, const std::string& destination);

        // --- NEW DEBUG/UTILITY METHODS ---
        // Reads entire file content into a string
        static std::string read_file_content(const std::string& file_path);

        // Gets the size of a file
        static size_t get_file_size(const std::string& file_path);

        // Generates a hex dump string of file content
        static std::string generate_hex_dump(const std::string& file_path);

        // Checks if a file exists and is a regular file
        static bool is_regular_file_and_exists(const std::string& file_path);

        // You won't need get_file_stats directly, as generate_hex_dump and get_file_size cover that.
        // If you want a specific "stats" string, you can create it from get_file_size/is_regular_file_and_exists.path)
    };

} // namespace Salix