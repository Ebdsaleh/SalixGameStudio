// =================================================================================
// Filename:    Salix/management/FileManager.cpp
// Author:      SalixGameStudio
// Description: Implements the FileManager using the C++17 <filesystem> library.
// =================================================================================
#include <Salix/management/FileManager.h>
#include <filesystem> // <<< The C++ standard library for file system operations
#include <iostream>
#include <algorithm>  // For std::replace
#include <fstream>    // For std::ifstream, std::ios::binary
#include <sstream>    // For std::stringstream (used in hex dump formatting)
#include <iomanip>    // For std::hex, std::setw, std::setfill

namespace Salix {

    bool FileManager::path_exists(const std::string& path) {
        // std::filesystem::exists() is the modern, standard way to check this.
        return std::filesystem::exists(path);
    }

    bool FileManager::create_directory(const std::string& path) {
        // Let create_directory handle the atomic check.
        try {
            // create_directory returns true if created, false if it already existed.
            if (std::filesystem::create_directory(path)) {
                std::cout << "FileManager: Created directory at '" << path << "'" << std::endl;
            }
            // If we get here, it means either we created it (true) or it already existed (false).
            // Both scenarios are considered "Success" for this function's purpose.
            return true;
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "FileManager Error: Could not create directory '" << path << "'. " << e.what() << std::endl;
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
        }
        return false;
    }

    bool FileManager::copy_file(const std::string& source, const std::string& destination) {
        // Use the C++17 filesystem library's copy function.
        // It's robust and handles errors for us.
        try {
            // We'll tell it to overwrite the destination if it already exists.
            std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
            return true;
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "FileManager Error: Could not copy file from '" << source 
                << "' to '" << destination << "'. " << e.what() << std::endl;
        }
        return false;
    }
    
     std::string FileManager::read_file_content(const std::string& file_path) {
        std::string content;
        std::ifstream file_stream(file_path, std::ios::binary); // Use binary mode for exact byte reading
        if (file_stream.is_open()) {
            // Get file size to pre-allocate string
            std::streampos fileSize = file_stream.tellg();
            file_stream.seekg(0, std::ios::end);
            fileSize = file_stream.tellg() - fileSize;
            file_stream.seekg(0, std::ios::beg); // Rewind to beginning

            content.resize(static_cast<size_t>(fileSize)); // Pre-allocate string buffer

            // Read all bytes into the string
            if (fileSize > 0) { // Avoid reading from empty file (can cause issues)
                file_stream.read(&content[0], fileSize);
            }
            file_stream.close();
        } else {
            std::cerr << "FileManager::read_file_content Error: Could not open file '" << file_path << "'" << std::endl;
        }
        return content;
    }

    // Gets the size of a file
    size_t FileManager::get_file_size(const std::string& file_path) {
        try {
            // Purposefully removed the exists() and is_regular_file() checks.
            // std::filesystem::file_size throws an exception if the path 
            // is invalid, doesn't exist, or is a directory.
            // This allows the catch block to handle all failure cases naturally.
            return std::filesystem::file_size(file_path);
        } catch (const std::filesystem::filesystem_error& e) {
            // Passing a directory path or a non-existent path in your tests 
            // will now trigger this error handling.
            std::cerr << "FileManager::get_file_size Error: " << e.what() << std::endl;
        }
        return 0;
    }

    // Generates a hex dump string of file content
    std::string FileManager::generate_hex_dump(const std::string& file_path) {
        std::string content = read_file_content(file_path); // Use our new read_file_content
        std::stringstream ss;

        ss << "---START HEX DUMP (for: " << file_path << ")---\n";
        if (content.empty()) {
            ss << "File is empty or could not be read.\n";
        } else {
            ss << "Length: " << content.length() << " bytes\n";
            ss << "Content (hex and char):\n";
            for (size_t i = 0; i < content.length(); ++i) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(static_cast<unsigned char>(content[i]))) << " ";
                if ((i + 1) % 16 == 0) { // Newline every 16 bytes for readability
                    ss << "   ";
                    for (size_t j = i - 15; j <= i; ++j) {
                        char c = content[j];
                        ss << ((c >= 32 && c <= 126) ? c : '.'); // Print printable chars, else '.'
                    }
                    ss << std::endl;
                }
            }
            // Print remaining chars if not a multiple of 16
            if (content.length() % 16 != 0) {
                for (size_t i = 0; i < (16 - (content.length() % 16)); ++i) {
                    ss << "   ";
                }
                ss << "   ";
                for (size_t j = content.length() - (content.length() % 16); j < content.length(); ++j) {
                    char c = content[j];
                    ss << ((c >= 32 && c <= 126) ? c : '.');
                }
                ss << std::endl;
            }
        }
        ss << "---END HEX DUMP---" << std::endl;
        return ss.str();
    }

    // Checks if a file exists and is a regular file
    bool FileManager::is_regular_file_and_exists(const std::string& file_path) {
        std::error_code ec;
        // Using the (path, ec) overload prevents exceptions. 
        // It returns true if it's a regular file.
        // It returns false if it's not, or if ANY error occurs (setting 'ec').
        return std::filesystem::is_regular_file(file_path, ec);
    }

     std::string FileManager::convert_to_relative(const std::string& project_root, const std::string& absolute_path)
    {
        // Work on normalized copies so callers can mix Windows and generic separators.
        std::string root = project_root;
        std::string abs_path = absolute_path;
        std::replace(root.begin(), root.end(), '\\', '/');
        std::replace(abs_path.begin(), abs_path.end(), '\\', '/');

        // A path is only inside the project when the root ends on a component boundary.
        // Exact root equality is the conventional current-directory relative path.
        if (abs_path == root) {
            return ".";
        }

        std::string root_prefix = root;
        if (!root_prefix.empty() && root_prefix.back() != '/') {
            root_prefix.push_back('/');
        }

        if (!root_prefix.empty() && abs_path.rfind(root_prefix, 0) == 0) {
            return abs_path.substr(root_prefix.length());
        }

        // Prefix lookalikes such as C:/ProjectSibling are outside C:/Project.
        return absolute_path;
    }

    std::string FileManager::convert_to_relative_path(const std::string& project_root, const std::string& absolute_path) {
    try {
        // 1. Create filesystem::path objects from the strings.
        const std::filesystem::path root_p(project_root);
        const std::filesystem::path abs_p(absolute_path);
        
        // 2. Use the standard library function to get the relative path.
        //    This function is designed to handle all edge cases correctly.
        std::filesystem::path relative_p = std::filesystem::relative(abs_p, root_p);
        
        // Different Windows drive roots cannot be represented by a relative path.
        // std::filesystem::relative can return an empty path without throwing.
        if (relative_p.empty()) {
            return absolute_path;
        }

        // Return the path as a standardized string with forward slashes.
        return relative_p.generic_string();
    }
    catch (const std::filesystem::filesystem_error& e) {
        // This can happen if the paths are on different drives.
        // We fall back to the absolute path in case of an error.
        std::cerr << "FileManager Error: Could not make path relative. " << e.what() << std::endl;
        
    }
    return absolute_path;
}

} // namespace Salix