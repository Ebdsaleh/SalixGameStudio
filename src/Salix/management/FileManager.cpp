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
            return true;
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "FileManager Error: Could not copy file from '" << source 
                << "' to '" << destination << "'. " << e.what() << std::endl;
            return false;
        }
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
            if (std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path)) {
                return std::filesystem::file_size(file_path);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "FileManager::get_file_size Error: " << e.what() << std::endl;
        }
        return 0; // Return 0 if file doesn't exist, is not regular, or error occurs
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
        try {
            return std::filesystem::is_regular_file(file_path);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "FileManager::is_regular_file_and_exists Error: " << e.what() << std::endl;
            return false;
        }
    }

     std::string FileManager::convert_to_relative(const std::string& project_root, const std::string& absolute_path)
    {
        // 1. Create copies to work with that we can modify.
        std::string root = project_root;
        std::string abs_path = absolute_path;

        // 2. Normalize all path separators to forward slashes for consistent comparison.
        std::replace(root.begin(), root.end(), '\\', '/');
        std::replace(abs_path.begin(), abs_path.end(), '\\', '/');

        // 3. Check if the absolute path starts with the project root path.
        //    string::rfind is a reliable C++ way to check for a prefix.
        if (abs_path.rfind(root, 0) == 0) {
            // 4. If it does, get the part of the string that comes after the root path.
            //    The +1 is to also remove the leading slash from the result.
            return abs_path.substr(root.length() + 1);
        }

        // 5. If the path is not inside the project, return the original absolute path as a fallback.
        return absolute_path;
    }

} // namespace Salix