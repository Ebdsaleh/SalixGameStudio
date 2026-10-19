// =================================================================================
// Filename:    src/Tests/SalixEngine/management/FileManager.test.cpp
// Description: Contains unit tests for the FileManager class.
// =================================================================================

#include <doctest.h>
#include <Salix/management/FileManager.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <Tests/TestFixtures.h>

// Helper to create a dummy file for testing read/copy operations
void create_dummy_file(const std::string& path, const std::string& content) {
    // ADD std::ios::binary HERE vvv
    std::ofstream out(path, std::ios::binary); 
    out << content;
    out.close();
}

TEST_SUITE("Salix::management::FileManager") {

    // Define test paths that will be created and destroyed
    const std::string test_dir = "temp_test_dir";
    const std::string nested_test_dir = test_dir + "/nested";
    const std::string test_file = test_dir + "/test_file.txt";
    const std::string test_file_content = "Hello, FileManager!";
    const std::string copied_file = test_dir + "/copied_file.txt";
    // For hex dump test
    const std::string hex_test_file = test_dir + "/hex_test.bin";
    const std::string hex_test_content = "Test\nData"; // 9 bytes
    const std::string empty_test_file = test_dir + "/empty.txt";

    // --- Test Case Fixture for Setup/Teardown ---
    // This fixture creates the test directory and file before each test case
    // and automatically cleans them up afterward, even if tests fail.
    struct FileManagerTestFixture {
        FileManagerTestFixture() {
            // Setup: Ensure a clean state before each test
            std::filesystem::remove_all(test_dir);
            
            // Create the base directory and a dummy file inside it
            Salix::FileManager::create_directory(test_dir);
            create_dummy_file(test_file, test_file_content);

            // This creates the files that your hex_dump test case needs *before* it runs.
            create_dummy_file(hex_test_file, hex_test_content);
            create_dummy_file(empty_test_file, "");
        }

        ~FileManagerTestFixture() {
            // Teardown: Clean up all test artifacts
            std::filesystem::remove_all(test_dir);
        }
    };

    TEST_CASE_FIXTURE(FileManagerTestFixture, "path_exists correctly checks files and directories") {
        // ASSERT: Check paths created by the fixture
        CHECK(Salix::FileManager::path_exists(test_dir) == true);
        CHECK(Salix::FileManager::path_exists(test_file) == true);
        
        // ASSERT: Check a path that does not exist
        CHECK(Salix::FileManager::path_exists("non_existent_file.txt") == false);
    }

    TEST_CASE("create_directory and create_directories") {
        // ARRANGE: Clean up any previous runs
        std::filesystem::remove_all("temp_create_dir");
        std::filesystem::remove_all("temp_create_nested");

        SUBCASE("create_directory creates a single directory") {
            // ACT
            bool success = Salix::FileManager::create_directory("temp_create_dir");
            
            // ASSERT
            CHECK(success == true);
            CHECK(std::filesystem::is_directory("temp_create_dir") == true);
        }

        SUBCASE("create_directory returns true if directory already exists") {
            // ARRANGE: Create the directory once
            REQUIRE(Salix::FileManager::create_directory("temp_create_dir") == true);
            
            // ACT: Call it a second time
            bool success = Salix::FileManager::create_directory("temp_create_dir");
            
            // ASSERT: Should return true (covering line 26)
            CHECK(success == true);
        }

        SUBCASE("create_directories creates a nested directory structure") {
            // ACT
            bool success = Salix::FileManager::create_directories("temp_create_nested/level1/level2");
            
            // ASSERT
            CHECK(success == true);
            CHECK(std::filesystem::is_directory("temp_create_nested/level1/level2") == true);
        }

        // CLEANUP (done in destructors of subcases)
        std::filesystem::remove_all("temp_create_dir");
        std::filesystem::remove_all("temp_create_nested");
    }

    TEST_CASE_FIXTURE(FileManagerTestFixture, "copy_file successfully copies a file") {
        SUBCASE("copies an existing file") {
            // ACT
            bool success = Salix::FileManager::copy_file(test_file, copied_file);
            
            // ASSERT
            CHECK(success == true);
            CHECK(std::filesystem::exists(copied_file) == true);
            
            // Verify content is identical
            std::ifstream ifs(copied_file);
            std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            CHECK(content == test_file_content);
        }

        SUBCASE("fails to copy a non-existent file") {
            // ACT: Try to copy a file that does not exist
            bool success = Salix::FileManager::copy_file("non_existent_source.txt", copied_file);
            
            // ASSERT: This should fail and cover the catch block (lines 58-62)
            CHECK(success == false);
            CHECK(std::filesystem::exists(copied_file) == false);
        }
    }

    TEST_CASE("convert_to_relative and convert_to_relative_path") {
        // ARRANGE
        // Use generic_string() to ensure forward slashes
        std::string root = std::filesystem::absolute("project").generic_string();
        std::string abs_path = std::filesystem::absolute("project/Assets/Textures/player.png").generic_string();
        // This path is deliberately outside the root
        std::string outside_abs_path = std::filesystem::absolute("other_folder/file.txt").generic_string();

        SUBCASE("convert_to_relative (string-based)") {
            // ACT
            std::string relative = Salix::FileManager::convert_to_relative(root, abs_path);
            std::string outside = Salix::FileManager::convert_to_relative(root, outside_abs_path);

            // ASSERT
            CHECK(relative == "Assets/Textures/player.png");
            // This should hit the fallback on line 151
            CHECK(outside == outside_abs_path);
        }

        SUBCASE("convert_to_relative_path (filesystem-based)") {
            // ACT
            std::string relative = Salix::FileManager::convert_to_relative_path(root, abs_path);
            std::string outside = Salix::FileManager::convert_to_relative_path(root, outside_abs_path);

            // ASSERT
            CHECK(relative == "Assets/Textures/player.png");
            // filesystem::relative will produce a ".." path
            CHECK(outside.find("..") != std::string::npos);
        }
    }

    TEST_CASE_FIXTURE(FileManagerTestFixture, "file utility functions") {
        SUBCASE("get_file_size") {
            // ACT: Get size of the file created in the fixture
            size_t size = Salix::FileManager::get_file_size(test_file);
            // ASSERT
            CHECK(size == test_file_content.length());

            // ACT: Get size of a non-existent file
            size_t non_existent_size = Salix::FileManager::get_file_size("non_existent.txt");
            // ASSERT: Should return 0 (covering line 91)
            CHECK(non_existent_size == 0);

            // ACT: Get size of a directory
            size_t dir_size = Salix::FileManager::get_file_size(test_dir);
            // ASSERT: Should return 0 (covering line 91)
            CHECK(dir_size == 0);
        }

        SUBCASE("read_file_content") {
            // ACT: Read the file from the fixture
            std::string content = Salix::FileManager::read_file_content(test_file);
            // ASSERT
            CHECK(content == test_file_content);

            // ACT: Read a non-existent file
            std::string non_existent_content = Salix::FileManager::read_file_content("non_existent.txt");
            // ASSERT: Should return empty string and cover line 78
            CHECK(non_existent_content.empty() == true);
        }

        SUBCASE("is_regular_file_and_exists identifies file types") {
            // ACT & ASSERT
            CHECK(Salix::FileManager::is_regular_file_and_exists(test_file) == true);
            CHECK(Salix::FileManager::is_regular_file_and_exists(test_dir) == false);
            CHECK(Salix::FileManager::is_regular_file_and_exists("non_existent.txt") == false);
        }
    }
    
    TEST_CASE_FIXTURE(FileManagerTestFixture, "generate_hex_dump") {
        // This entire test case covers lines 95-126

        SUBCASE("generates correct dump for file with content") {
            // ARRANGE: Content is "Test\nData" (9 bytes)
            // T  e  s  t  \n D  a  t  a
            // 54 65 73 74 0a 44 61 74 61
            
            // ACT
            std::string dump = Salix::FileManager::generate_hex_dump(hex_test_file);
            
            // ASSERT
            CHECK(dump.find("Length: 9 bytes") != std::string::npos);
            // Check for the hex content
            CHECK(dump.find("54 65 73 74 0a 44 61 74 61") != std::string::npos);
            // Check for the char content (covering the remainder loop, lines 120-125)
            CHECK(dump.find("Test.Data") != std::string::npos);
        }

        SUBCASE("generates correct dump for an empty file") {
            // ACT
            std::string dump = Salix::FileManager::generate_hex_dump(empty_test_file);
            
            // ASSERT: Covers the `if (content.empty())` branch
            CHECK(dump.find("File is empty or could not be read.") != std::string::npos);
        }

        SUBCASE("generates correct dump for a file with 16 bytes") {
            // ARRANGE
            const std::string full_line_file = test_dir + "/full_line.txt";
            const std::string full_line_content = "0123456789ABCDEF"; // Exactly 16 bytes
            create_dummy_file(full_line_file, full_line_content);

            // ACT
            std::string dump = Salix::FileManager::generate_hex_dump(full_line_file);

            // ASSERT: This covers the `if ((i + 1) % 16 == 0)` branch (lines 114-118)
            CHECK(dump.find("Length: 16 bytes") != std::string::npos);
            CHECK(dump.find("0123456789ABCDEF") != std::string::npos);
            // This checks that the remainder loop (line 120) is *not* entered
            CHECK(dump.find("Test.Data") == std::string::npos);
        }
    }

    TEST_CASE_FIXTURE(FileManagerTestFixture, "exception handling for filesystem errors") {
        // These tests force std::filesystem to throw exceptions to verify the catch blocks in FileManager.cpp

        SUBCASE("create_directory throws when parent does not exist") {
            // create_directory (singular) fails if the parent dir doesn't exist. 
            // It throws a filesystem_error, which FileManager should catch and return false.
            bool success = Salix::FileManager::create_directory("non_existent_parent/new_dir");
            CHECK(success == false);
        }

        SUBCASE("create_directories throws when obstructed by a file") {
            // 1. Create a file.
            std::string file_blocker = test_dir + "/BlockerFile";
            create_dummy_file(file_blocker, "I am a file");

            // 2. Try to create a directory *inside* that file.
            // This is physically impossible and throws filesystem_error.
            bool success = Salix::FileManager::create_directories(file_blocker + "/ImpossibleDir");
            CHECK(success == false);
        }
        
        SUBCASE("is_regular_file_and_exists handles errors gracefully") {
            // Passing invalid characters (on Windows) forces std::filesystem to throw,
            // triggering the catch block which returns false.
            bool result = Salix::FileManager::is_regular_file_and_exists("invalid|path?*");
            CHECK(result == false);
        }

        SUBCASE("copy_file throws when destination is a directory") {
            // 1. Create a source file.
            std::string src = test_dir + "/src.txt";
            create_dummy_file(src, "data");

            // 2. Create a destination *directory*.
            std::string dest_dir = test_dir + "/DestDir";
            Salix::FileManager::create_directory(dest_dir);

            // 3. Try to copy a file *onto* the directory path.
            // This usually throws "File exists" or "Is a directory" exception.
            bool success = Salix::FileManager::copy_file(src, dest_dir);
            CHECK(success == false);
        }

        SUBCASE("convert_to_relative_path preserves cross-drive absolute paths on Windows") {
            // Windows has no relative path between distinct drive roots.
            // std::filesystem::relative may return an empty path without throwing.
            const std::string root = "C:/Project";
            const std::string target = "D:/Assets/File.png";
            const std::string result = Salix::FileManager::convert_to_relative_path(root, target);

#ifdef _WIN32
            // Preserve the original path so the caller can still access/import it.
            CHECK(result == target);
#else
            // C: and D: are not Windows volumes on other platforms.
            CHECK(!result.empty());
#endif
        }
        
        SUBCASE("get_file_size handles errors gracefully") {
            // Attempting to check a path with invalid characters (on Windows) often throws.
            // On Linux this is a valid filename, so we just ensure it returns 0 and doesn't crash.
            size_t size = Salix::FileManager::get_file_size("invalid|path?*");
            CHECK(size == 0);
        }
    }
}