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

// Isolated FileManager fixture: never deletes or overwrites user/project paths.
#include <chrono>
#include <system_error>
#include <iterator>
namespace {
    struct FileManagerOwnedTemp20261019 {
        std::filesystem::path root;
        bool owned=false;

        FileManagerOwnedTemp20261019() {
            std::error_code ec;
            auto base=std::filesystem::temp_directory_path(ec);
            if(ec) return;
            const auto tick=std::chrono::steady_clock::now().time_since_epoch().count();
            for(int i=0;i<32;++i) {
                root=base/("salix-fm-20261019-"+std::to_string(tick)+"-"+std::to_string(i));
                ec.clear();
                if(std::filesystem::create_directory(root,ec)) {owned=true;return;}
                if(ec) break;
            }
            root.clear();
        }
        FileManagerOwnedTemp20261019(const FileManagerOwnedTemp20261019&)=delete;
        FileManagerOwnedTemp20261019& operator=(const FileManagerOwnedTemp20261019&)=delete;
        ~FileManagerOwnedTemp20261019() {
            if(!owned) return;
            std::error_code ec;
            std::filesystem::remove_all(root,ec);
        }
        std::string file(const char* name) const {return (root/name).string();}
        void write(const char* name,const std::string& data) const {
            std::ofstream out(file(name),std::ios::binary);
            out.write(data.data(),static_cast<std::streamsize>(data.size()));
        }
    };
}
TEST_SUITE("Salix::management::FileManagerAdditional20261019") {
    TEST_CASE("file manager isolated owned directory creation supports arbitrarily nested names") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        const auto nested=temp.root/"one"/"two"/"three";
        CHECK(Salix::FileManager::create_directories(nested.string()));
        CHECK(std::filesystem::is_directory(nested));
        CHECK(Salix::FileManager::create_directories(nested.string()));
    }

    TEST_CASE("file manager isolated single directory creation accepts a preexisting owned directory") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        CHECK(Salix::FileManager::create_directory(temp.root.string()));
        CHECK(std::filesystem::is_directory(temp.root));
    }

    TEST_CASE("file manager isolated file obstruction cannot silently become a directory tree") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        temp.write("blocker","data");
        CHECK_FALSE(Salix::FileManager::create_directories((temp.root/"blocker"/"nested").string()));
        CHECK(Salix::FileManager::is_regular_file_and_exists(temp.file("blocker")));
    }

    TEST_CASE("file manager isolated binary file read retains embedded null and high bytes") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        const std::string bytes("\x00\x01\x7f\xffX", 5);
        temp.write("binary.dat",bytes);
        CHECK(Salix::FileManager::read_file_content(temp.file("binary.dat"))==bytes);
        CHECK(Salix::FileManager::get_file_size(temp.file("binary.dat"))==bytes.size());
    }

    TEST_CASE("file manager isolated empty file is distinguished from a missing regular file") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        temp.write("empty.dat","");
        CHECK(Salix::FileManager::path_exists(temp.file("empty.dat")));
        CHECK(Salix::FileManager::is_regular_file_and_exists(temp.file("empty.dat")));
        CHECK(Salix::FileManager::get_file_size(temp.file("empty.dat"))==0);
        CHECK_FALSE(Salix::FileManager::path_exists(temp.file("missing.dat")));
    }

    TEST_CASE("file manager isolated copying onto an existing file replaces destination without changing source") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        temp.write("source.dat","replacement");
        temp.write("target.dat","old");
        CHECK(Salix::FileManager::copy_file(temp.file("source.dat"),temp.file("target.dat")));
        CHECK(Salix::FileManager::read_file_content(temp.file("target.dat"))=="replacement");
        CHECK(Salix::FileManager::read_file_content(temp.file("source.dat"))=="replacement");
    }

    TEST_CASE("file manager isolated missing copy source leaves destination absent") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        CHECK_FALSE(Salix::FileManager::copy_file(temp.file("absent.dat"),temp.file("output.dat")));
        CHECK_FALSE(Salix::FileManager::path_exists(temp.file("output.dat")));
    }

    TEST_CASE("file manager isolated hex dump encodes control bytes and printable bytes separately") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        temp.write("bytes.dat",std::string{'A','\n','B'});
        const auto dump=Salix::FileManager::generate_hex_dump(temp.file("bytes.dat"));
        CHECK(dump.find("Length: 3 bytes")!=std::string::npos);
        CHECK(dump.find("41 0a 42")!=std::string::npos);
        CHECK(dump.find("A.B")!=std::string::npos);
    }

    TEST_CASE("file manager isolated hex dump emits an ASCII group at the sixteen-byte boundary") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        temp.write("line.dat","0123456789ABCDEF");
        const auto dump=Salix::FileManager::generate_hex_dump(temp.file("line.dat"));
        CHECK(dump.find("Length: 16 bytes")!=std::string::npos);
        CHECK(dump.find("0123456789ABCDEF")!=std::string::npos);
    }

    TEST_CASE("file manager isolated relative conversion reports owned nested paths without touching files") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        const auto child=temp.root/"assets"/"level.dat";
        const auto relative=Salix::FileManager::convert_to_relative_path(temp.root.string(),child.string());
        CHECK(std::filesystem::path(relative).generic_string()=="assets/level.dat");
        const auto string_relative=Salix::FileManager::convert_to_relative(temp.root.string(),child.string());
        CHECK(std::filesystem::path(string_relative).generic_string()=="assets/level.dat");
    }
}


// Additional stable contracts for the 847+ coverage batch.
TEST_SUITE("Salix::management::FileManagerBatch847") {
    TEST_CASE("file manager reads embedded NUL bytes without truncation") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        const std::string bytes("A\0B\0C", 5);
        temp.write("embedded-null.bin", bytes);
        const auto loaded = Salix::FileManager::read_file_content(temp.file("embedded-null.bin"));
        REQUIRE(loaded.size() == 5);
        CHECK(loaded == bytes);
    }

    TEST_CASE("file manager reports exact size for binary data containing high bytes") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        const std::string bytes("\x00\x7f\x80\xff", 4);
        temp.write("high.bin", bytes);
        CHECK(Salix::FileManager::get_file_size(temp.file("high.bin")) == 4);
    }

    TEST_CASE("file manager copy overwrites an existing destination atomically from caller perspective") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        temp.write("src-overwrite.bin", "new-data");
        temp.write("dst-overwrite.bin", "old");
        REQUIRE(Salix::FileManager::copy_file(temp.file("src-overwrite.bin"), temp.file("dst-overwrite.bin")));
        CHECK(Salix::FileManager::read_file_content(temp.file("dst-overwrite.bin")) == "new-data");
    }

    TEST_CASE("file manager failed copy leaves an existing destination unchanged") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        temp.write("protected.txt", "keep-me");
        CHECK_FALSE(Salix::FileManager::copy_file(temp.file("missing-source.txt"), temp.file("protected.txt")));
        CHECK(Salix::FileManager::read_file_content(temp.file("protected.txt")) == "keep-me");
    }

    TEST_CASE("file manager regular file predicate rejects directories but accepts empty files") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        temp.write("empty.txt", "");
        CHECK_FALSE(Salix::FileManager::is_regular_file_and_exists(temp.root.string()));
        CHECK(Salix::FileManager::is_regular_file_and_exists(temp.file("empty.txt")));
    }

    TEST_CASE("file manager reading a missing file returns an empty string without creating it") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        const auto missing = temp.file("never-created.bin");
        CHECK(Salix::FileManager::read_file_content(missing).empty());
        CHECK_FALSE(std::filesystem::exists(missing));
    }

    TEST_CASE("file manager hex dump emits printable ASCII and dots for control bytes") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        temp.write("hex-contract.bin", std::string("A\nB", 3));
        const auto dump = Salix::FileManager::generate_hex_dump(temp.file("hex-contract.bin"));
        CHECK(dump.find("41 0a 42") != std::string::npos);
        CHECK(dump.find("A.B") != std::string::npos);
    }

    TEST_CASE("file manager hex dump marks an unreadable missing file as empty or unreadable") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        const auto dump = Salix::FileManager::generate_hex_dump(temp.file("missing.hex"));
        CHECK(dump.find("File is empty or could not be read.") != std::string::npos);
        CHECK(dump.find("---END HEX DUMP---") != std::string::npos);
    }

    TEST_CASE("file manager string relative conversion normalizes mixed separators inside root") {
        const std::string root = "C:\\Project";
        const std::string absolute = "C:/Project/Assets\\Sprites/player.png";
        CHECK(Salix::FileManager::convert_to_relative(root, absolute) == "Assets/Sprites/player.png");
    }

    TEST_CASE("file manager nested directory creation remains successful when repeated") {
        FileManagerOwnedTemp20261019 temp;
        REQUIRE(temp.owned);
        const auto nested = temp.root / "a" / "b" / "c";
        REQUIRE(Salix::FileManager::create_directories(nested.string()));
        REQUIRE(std::filesystem::is_directory(nested));
        CHECK(Salix::FileManager::create_directories(nested.string()));
        CHECK(std::filesystem::is_directory(nested));
    }
}

TEST_SUITE("Salix::management::FileManagerBatch2247") {
    TEST_CASE("batch2247 path existence distinguishes owned directory file and missing child") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned); temp.write("present.txt","x");
        CHECK(Salix::FileManager::path_exists(temp.root.string()));
        CHECK(Salix::FileManager::path_exists(temp.file("present.txt")));
        CHECK_FALSE(Salix::FileManager::path_exists(temp.file("missing.txt")));
    }
    TEST_CASE("batch2247 create directory succeeds when target already exists") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned);
        CHECK(Salix::FileManager::create_directory(temp.root.string()));
        CHECK(std::filesystem::is_directory(temp.root));
    }
    TEST_CASE("batch2247 nested directory creation fails when an intermediate component is a file") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned); temp.write("block","data");
        CHECK_FALSE(Salix::FileManager::create_directories((temp.root/"block"/"child").string()));
        CHECK(Salix::FileManager::is_regular_file_and_exists(temp.file("block")));
    }
    TEST_CASE("batch2247 binary copy preserves embedded null and high bytes exactly") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned);
        const std::string bytes("\0A\x7f\xffZ",5); temp.write("src.bin",bytes);
        REQUIRE(Salix::FileManager::copy_file(temp.file("src.bin"),temp.file("dst.bin")));
        CHECK(Salix::FileManager::read_file_content(temp.file("dst.bin"))==bytes);
    }
    TEST_CASE("batch2247 failed copy preserves an existing destination payload") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned); temp.write("dst.txt","keep");
        CHECK_FALSE(Salix::FileManager::copy_file(temp.file("missing.txt"),temp.file("dst.txt")));
        CHECK(Salix::FileManager::read_file_content(temp.file("dst.txt"))=="keep");
    }
    TEST_CASE("batch2247 file size returns zero for missing path without creating it") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned); const auto missing=temp.file("missing-size.bin");
        CHECK(Salix::FileManager::get_file_size(missing)==0); CHECK_FALSE(std::filesystem::exists(missing));
    }
    TEST_CASE("batch2247 seventeen byte hex dump emits first full ASCII row and final remainder") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned); temp.write("seventeen.bin","0123456789ABCDEFG");
        const auto dump=Salix::FileManager::generate_hex_dump(temp.file("seventeen.bin"));
        CHECK(dump.find("Length: 17 bytes")!=std::string::npos);
        CHECK(dump.find("0123456789ABCDEF")!=std::string::npos);
        CHECK(dump.find("47 ")!=std::string::npos);
    }
    TEST_CASE("batch2247 string relative conversion leaves outside path unchanged") {
        const std::string root="C:/ProjectRoot"; const std::string outside="D:/External/file.txt";
        CHECK(Salix::FileManager::convert_to_relative(root,outside)==outside);
    }
    TEST_CASE("batch2247 string relative conversion normalizes backslashes before extracting child path") {
        const std::string root="C:\\ProjectRoot"; const std::string child="C:\\ProjectRoot\\Assets\\Audio\\theme.wav";
        CHECK(Salix::FileManager::convert_to_relative(root,child)=="Assets/Audio/theme.wav");
    }
    TEST_CASE("batch2247 filesystem relative conversion returns nested path components in order") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned);
        const auto nested=temp.root/"Assets"/"Realms"/"Main.json";
        const auto relative=std::filesystem::path(Salix::FileManager::convert_to_relative_path(temp.root.string(),nested.string())).generic_string();
        CHECK(relative=="Assets/Realms/Main.json");
    }
}

TEST_SUITE("Salix::management::FileManagerBatch2847") {
    TEST_CASE("batch2847 nested directory creation is repeatable at the same path") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned);
        const auto nested=temp.root/"one"/"two"/"three";
        REQUIRE(Salix::FileManager::create_directories(nested.string()));
        CHECK(Salix::FileManager::create_directories(nested.string()));
        CHECK(std::filesystem::is_directory(nested));
    }
    TEST_CASE("batch2847 regular-file predicate rejects missing paths and directories") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned);
        CHECK_FALSE(Salix::FileManager::is_regular_file_and_exists(temp.root.string()));
        CHECK_FALSE(Salix::FileManager::is_regular_file_and_exists(temp.file("missing2847.bin")));
    }
    TEST_CASE("batch2847 empty file reads empty while still existing as a regular file") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned); temp.write("empty2847.bin","");
        CHECK(Salix::FileManager::read_file_content(temp.file("empty2847.bin")).empty());
        CHECK(Salix::FileManager::is_regular_file_and_exists(temp.file("empty2847.bin")));
        CHECK(Salix::FileManager::get_file_size(temp.file("empty2847.bin"))==0);
    }
    TEST_CASE("batch2847 multiline text is read byte for byte") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned);
        const std::string text="first\nsecond\r\nthird";
        temp.write("text2847.txt",text);
        CHECK(Salix::FileManager::read_file_content(temp.file("text2847.txt"))==text);
    }
    TEST_CASE("batch2847 overwrite copy replaces longer destination content completely") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned);
        temp.write("src2847.txt","short"); temp.write("dst2847.txt","this destination used to be much longer");
        REQUIRE(Salix::FileManager::copy_file(temp.file("src2847.txt"),temp.file("dst2847.txt")));
        CHECK(Salix::FileManager::read_file_content(temp.file("dst2847.txt"))=="short");
    }
    TEST_CASE("batch2847 one-byte hex dump includes length hex byte and ASCII group") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned); temp.write("one2847.bin","A");
        const auto dump=Salix::FileManager::generate_hex_dump(temp.file("one2847.bin"));
        CHECK(dump.find("Length: 1 bytes")!=std::string::npos);
        CHECK(dump.find("41 ")!=std::string::npos);
        CHECK(dump.find("A\n")!=std::string::npos);
    }
    TEST_CASE("batch2847 fifteen-byte hex dump exercises padded final ASCII row") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned); temp.write("fifteen2847.bin","123456789ABCDEF");
        const auto dump=Salix::FileManager::generate_hex_dump(temp.file("fifteen2847.bin"));
        CHECK(dump.find("Length: 15 bytes")!=std::string::npos);
        CHECK(dump.find("123456789ABCDEF")!=std::string::npos);
    }
    TEST_CASE("batch2847 sixteen-byte hex dump emits a complete ASCII row") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned); temp.write("sixteen2847.bin","123456789ABCDEFG");
        const auto dump=Salix::FileManager::generate_hex_dump(temp.file("sixteen2847.bin"));
        CHECK(dump.find("Length: 16 bytes")!=std::string::npos);
        CHECK(dump.find("123456789ABCDEFG")!=std::string::npos);
    }
    TEST_CASE("batch2847 string relative conversion preserves nested filename after separator normalization") {
        const std::string root="D:\\Games\\Project";
        const std::string child="D:/Games/Project/Assets\\Models\\ship.obj";
        CHECK(Salix::FileManager::convert_to_relative(root,child)=="Assets/Models/ship.obj");
    }
    TEST_CASE("batch2847 filesystem relative conversion can walk from root to sibling path") {
        FileManagerOwnedTemp20261019 temp; REQUIRE(temp.owned);
        const auto root=temp.root/"Project"; const auto sibling=temp.root/"Sibling"/"file.txt";
        std::filesystem::create_directories(root); std::filesystem::create_directories(sibling.parent_path());
        const auto rel=std::filesystem::path(Salix::FileManager::convert_to_relative_path(root.string(),sibling.string())).generic_string();
        CHECK(rel=="../Sibling/file.txt");
    }
}

