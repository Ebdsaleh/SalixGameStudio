// =================================================================================
// Filename:    Salix/management/FileManager.cpp
// Author:      SalixGameStudio
// Description: Implements the FileManager using the C++17 <filesystem> library.
// =================================================================================
#include <Salix/management/FileManager.h>
#include <filesystem> // <<< The C++ standard library for file system operations
#include <iostream>

namespace Salix {

    bool FileManager::path_exists(const std::string& path) {
        // std::filesystem::exists() is the modern, standard way to check this.
        return std::filesystem::exists(path);
    }

    bool FileManager::create_directory(const std::string& path) {
        // First, check if it already exists. If so, it's a success.
        if (path_exists(path)) {
            return true;
        }

        // Try to create the directory. This function returns true on success.
        // We use a try-catch block to handle potential errors from the OS.
        try {
            if (std::filesystem::create_directory(path)) {
                std::cout << "FileManager: Created directory at '" << path << "'" << std::endl;
                return true;
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "FileManager Error: Could not create directory '" << path << "'. " << e.what() << std::endl;
            return false;
        }
        return false;
    }

    bool FileManager::create_directories(const std::string& path) {
        // This is the convenient, multi-directory version.
        try {
            // It returns true if it creates directories, false if they already existed.
            // For our API, we'll just return true in either success case.
            std::filesystem::create_directories(path);
            return true;
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "FileManager Error: Could not create directories for path '" <<
                path << "'. " << e.what() << std::endl;

            return false;
        }
    }

    bool FileManager::copy_file(const std::string& source, const std::string& destination) {
        // Use the C++17 filesystem library's copy function.
        // It's robust and handles errors for us.
        try {
            // We'll tell it to overwrite the destination if it already exists.
            std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "FileManager Error: Could not copy file from '" << source 
                << "' to '" << destination << "'. " << e.what() << std::endl;
            return false;
        }
    }
} // namespace Salix