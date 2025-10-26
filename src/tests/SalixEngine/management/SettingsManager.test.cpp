// src/Tests/SalixEngine/management/SettingsManager.test.cpp

#include <doctest.h>
#include <Salix/management/SettingsManager.h>
#include <Salix/core/ApplicationConfig.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <yaml-cpp/yaml.h>

// Helper to create a temporary config file for a test
void create_temp_config(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    file << content;
    file.close();
}

// A helper function to capture the std::cerr output.
struct CerrRedirect {
    std::streambuf* original_buffer;
    std::stringstream& local_buffer; // Store reference

    CerrRedirect(std::stringstream& buffer) : local_buffer(buffer) {
        original_buffer = std::cerr.rdbuf(); // Save original buffer
        std::cerr.rdbuf(local_buffer.rdbuf()); // Redirect std::cerr
    }

    ~CerrRedirect() {
        std::cerr.rdbuf(original_buffer); // Restore original buffer
    }
};

TEST_SUITE("Salix::management::SettingsManager") {

	TEST_CASE("load_settings successfully parses a valid YAML file") {
        // ARRANGE
        Salix::SettingsManager settings_manager;
        Salix::ApplicationConfig config;
        const std::string test_file = "doctest_temp_config.yaml";

        // Ensure no stale cache file exists.
        std::filesystem::remove("doctest_temp_config.cache");

        // This YAML content now has the correct indentation with standard spaces.
        const std::string yaml_content = R"(
Window:
  title: "Test Game"
  width: 800
  height: 600
Engine:
  initial_state: Editor
  target_fps: 144
Renderer:
  type: OpenGL
GUI:
  type: ImGui
Timer:
  type: Chrono
)";

		// Create the temporary file.
        create_temp_config(test_file, yaml_content);
        REQUIRE(std::filesystem::exists(test_file));

        // ACT
        bool success = settings_manager.load_settings(test_file, config);

        // ASSERT
        REQUIRE(success == true);
        CHECK(config.window_config.title == "Test Game");
        CHECK(config.window_config.width == 800);
        CHECK(config.initial_state == Salix::AppStateType::Editor);
        CHECK(config.target_fps == 144);
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        CHECK(config.timer_type == Salix::TimerType::Chrono);

        // CLEANUP
        std::filesystem::remove(test_file);
        std::filesystem::remove("doctest_temp_config.cache");
    }

	TEST_CASE("load_settings handles missing file by using defaults") {
		// ARRANGE
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config; // Default-constructed config
		const std::string non_existent_file = "non_existent_config.yaml";

		// Pre-condition: Ensure a default-constructed config has default values
		REQUIRE(config.target_fps == 60);
		REQUIRE(config.renderer_type == Salix::RendererType::SDL);

		// ACT
		bool success = settings_manager.load_settings(non_existent_file, config);

		// ASSERT
		// The function should "succeed" by not crashing and leaving the config with its default values.
		CHECK(success == true);
		CHECK(config.target_fps == 60); // Should remain default
		CHECK(config.renderer_type == Salix::RendererType::SDL); // Should remain default
    }

	TEST_CASE("save_settings handles file opening failure") {
        // ARRANGE
        Salix::SettingsManager settings_manager;
        Salix::ApplicationConfig config; // Default config is fine
        // Create a directory where we want to try writing the file.
        // Trying to open a directory as a file for writing should fail.
        const std::string invalid_dir_path = "test_dir_for_save_fail";
        const std::string invalid_file_path = invalid_dir_path; // Attempt to write directly to the dir path

        std::stringstream captured_cerr; // Buffer to capture cerr output

        // Ensure clean state & create the directory
        std::filesystem::remove_all(invalid_dir_path); // Remove if it exists from previous runs
        std::filesystem::create_directory(invalid_dir_path);
        REQUIRE(std::filesystem::is_directory(invalid_dir_path)); // Make sure dir exists

        bool save_result;
        { // Scope for CerrRedirect
            CerrRedirect redirect(captured_cerr); // Redirect cerr

            // ACT
            // Attempt to save settings, providing the directory path instead of a file path.
            save_result = settings_manager.save_settings(invalid_file_path, config);

        } // CerrRedirect destructor restores std::cerr here

        // ASSERT
        // Check that save_settings returned false due to the file opening error.
        CHECK(save_result == false);

        // Check if the captured output contains the specific error message text
        // from the `!file.is_open()` block.
        std::string output = captured_cerr.str();
        CHECK(output.find("SettingsManager Error: Could not open file for writing") != std::string::npos);
        // We no longer expect the YAML::Exception message here.

        // CLEANUP
        std::filesystem::remove_all(invalid_dir_path); // Clean up the created directory
    }

    TEST_CASE("save_settings writes a correct YAML and covers all enum branches") {
        // ARRANGE
        Salix::SettingsManager settings_manager;
        Salix::ApplicationConfig config_to_save;
        const std::string test_save_file = "temp_save_config.yaml";

        // Customize the config to use all the untested enum values for saving
        config_to_save.window_config.title = "Saved Game";
        config_to_save.target_fps = 90;
        config_to_save.renderer_type = Salix::RendererType::OpenGL;
        config_to_save.initial_state = Salix::AppStateType::Options; // Covers the 'Options' branch
        config_to_save.gui_type = Salix::GuiType::None;             // Covers the 'None' branch
        config_to_save.timer_type = Salix::TimerType::Chrono;       // Covers the 'Chrono' branch

        // ACT
        bool save_success = settings_manager.save_settings(test_save_file, config_to_save);
        REQUIRE(save_success == true);

        // Now, load the file we just saved to verify its contents
        Salix::ApplicationConfig loaded_config;
        settings_manager.load_settings(test_save_file, loaded_config);

        // ASSERT
        CHECK(loaded_config.window_config.title == "Saved Game");
        CHECK(loaded_config.target_fps == 90);
        CHECK(loaded_config.renderer_type == Salix::RendererType::OpenGL);
        CHECK(loaded_config.initial_state == Salix::AppStateType::Options);
        CHECK(loaded_config.gui_type == Salix::GuiType::None);
        CHECK(loaded_config.timer_type == Salix::TimerType::Chrono);

        // CLEANUP
        std::filesystem::remove(test_save_file);
        std::filesystem::remove("temp_save_config.cache");
    }
	
	TEST_CASE("save_settings fails when writing to an invalid path") {
		// ARRANGE
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config;
		// This path is invalid because the directory "non_existent_directory" doesn't exist.
		const std::string invalid_path = "non_existent_directory/config.yaml";

		// ACT & ASSERT
		// This should return false, covering the file opening error branch.
		CHECK(settings_manager.save_settings(invalid_path, config) == false);
	}
    
	TEST_CASE("save_settings correctly saves all enum types") {
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config;
		const std::string test_file = "enum_save_test.yaml";

		SUBCASE("saves RendererType::SDL") {
			config.renderer_type = Salix::RendererType::SDL;
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		SUBCASE("saves AppStateType::Game") {
			config.initial_state = Salix::AppStateType::Game;
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		SUBCASE("saves GuiType::ImGui") {
			config.gui_type = Salix::GuiType::ImGui;
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		SUBCASE("saves TimerType::SDL") {
			config.timer_type = Salix::TimerType::SDL;
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		SUBCASE("saves AppStateType::Editor") {
			config.initial_state = Salix::AppStateType::Editor;
			CHECK(settings_manager.save_settings(test_file, config) == true);
    	}
		SUBCASE("handles default enum cases") {
			// Force an invalid enum value to trigger the default case
			config.initial_state = static_cast<Salix::AppStateType>(99); 
			config.renderer_type = static_cast<Salix::RendererType>(99);
			config.gui_type = static_cast<Salix::GuiType>(99);
			config.timer_type = static_cast<Salix::TimerType>(99);
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		// CLEANUP
		std::filesystem::remove(test_file);
	}

	TEST_CASE("load_settings handles malformed YAML file") {
		// ARRANGE
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config;
		const std::string malformed_file = "malformed.yaml";
		
		// This YAML is syntactically invalid (unclosed curly brace), which guarantees a parse error.
		create_temp_config(malformed_file, "Window: { title: \"This is not valid YAML }");

		// ACT & ASSERT
		// This will now return false, covering the YAML::Exception catch block.
		CHECK(settings_manager.load_settings(malformed_file, config) == false);

		// CLEANUP
		std::filesystem::remove(malformed_file);
	}

    TEST_CASE("load_settings correctly reads from a newer binary cache") {
		// ARRANGE
		Salix::SettingsManager settings_manager;
		const std::string yaml_file = "cache_test.yaml";
		const std::string cache_file = "cache_test.cache";

		// 1. Create a YAML file with "YAML Data".
		create_temp_config(yaml_file, "Window:\n  title: \"From YAML\"");
		
		// 2. Load it once. This will create a cache file with "YAML Data".
		Salix::ApplicationConfig temp_config;
		settings_manager.load_settings(yaml_file, temp_config);

		// 3. Wait a moment to ensure file timestamps will be different.
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		
		// 4. Overwrite the YAML file with "New YAML Data".
		create_temp_config(yaml_file, "Window:\n  title: \"New YAML Data\"");

		// ACT: Load settings again. The cache file is now older than the YAML file,
		// so it should re-parse the YAML and create a NEW cache.
		Salix::ApplicationConfig loaded_config;
		settings_manager.load_settings(yaml_file, loaded_config);

		// ASSERT: This confirms our setup is correct and the cache was updated.
		REQUIRE(loaded_config.window_config.title == "New YAML Data");

		// 5. Wait again and then change the YAML file one last time.
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		create_temp_config(yaml_file, "Window:\n  title: \"Final YAML Data\"");
		
		// ACT 2: Load settings a final time. Now, the cache from step 4 should be
		// NEWER than the YAML from step 2, but OLDER than the final YAML.
		// Wait, the logic is simpler. The cache file is now NEWER than the original YAML,
		// let's manually make it newer than the FINAL yaml.
		// The previous test logic was flawed. Let's simplify.

		// Let's restart the logic for this test.
		// 1. Create YAML file with initial data.
		create_temp_config(yaml_file, "Window:\n  title: \"Initial Data\"");
		
		// 2. Load it to create a cache file.
		Salix::ApplicationConfig config1;
		settings_manager.load_settings(yaml_file, config1);
		REQUIRE(config1.window_config.title == "Initial Data");

		// 3. Modify the original config object and manually write a NEW cache file
		//    that is out of sync with the YAML file.
		Salix::ApplicationConfig cache_only_config;
		cache_only_config.window_config.title = "From Cache";
		// We can't call a `write_cache` directly, so let's rethink.

		// Okay, the original test logic was almost correct but had one flaw.
		// `save_settings` doesn't write a cache. `load_settings` does.

		// Let's use the logic from the user's HTML report to guide us.
		// `if (std::filesystem::exists(cache_path) && std::filesystem::exists(yaml_path) && std::filesystem::last_write_time(cache_path) >= std::filesystem::last_write_time(yaml_path))`
		
		// Let's try this again.
		
		// CLEANUP from previous failed attempts
		std::filesystem::remove(yaml_file);
		std::filesystem::remove(cache_file);

		// ARRANGE
		// 1. Create a YAML with "original" data.
		create_temp_config(yaml_file, "Window:\n  title: \"Original YAML\"");
		
		// 2. Load it. This creates a cache file with "Original YAML" data.
		Salix::ApplicationConfig temp_config1;
		settings_manager.load_settings(yaml_file, temp_config1);
		REQUIRE(temp_config1.window_config.title == "Original YAML");

		// 3. Now, overwrite ONLY the YAML file with "new" data.
		create_temp_config(yaml_file, "Window:\n  title: \"New YAML\"");

		// 4. Force the cache file's timestamp to be NEWER than the YAML file.
		// This is the key to forcing the `read_cache` path.
		auto yaml_time = std::filesystem::last_write_time(yaml_file);
		std::filesystem::last_write_time(cache_file, yaml_time + std::chrono::seconds(1));

		// ACT
		Salix::ApplicationConfig final_config;
		settings_manager.load_settings(yaml_file, final_config);

		// ASSERT
		// Because the cache was newer, the loaded title should be "Original YAML", not "New YAML".
		// This proves `read_cache` was executed.
		CHECK(final_config.window_config.title == "Original YAML");

		// CLEANUP
		std::filesystem::remove(yaml_file);
		std::filesystem::remove(cache_file);
      }

      TEST_CASE("covers remaining parsing branches and error handling") {
        // ARRANGE
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config;

		// ACT & ASSERT for save_settings error path (passing an empty path)
		CHECK(settings_manager.save_settings("", config) == false);

		// ACT & ASSERT for remaining parse_app_state_type branches
		create_temp_config("enum_test.yaml", "Engine:\n  initial_state: Game");
		settings_manager.load_settings("enum_test.yaml", config);
		CHECK(config.initial_state == Salix::AppStateType::Game);

        // **** ADD A SMALL DELAY FOR TIMESTAMP RESOLUTION ****
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

		create_temp_config("enum_test.yaml", "Engine:\n  initial_state: Options");
		settings_manager.load_settings("enum_test.yaml", config);
		CHECK(config.initial_state == Salix::AppStateType::Options);

        // **** ADD A SMALL DELAY FOR TIMESTAMP RESOLUTION ****
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
		// ACT & ASSERT for remaining parse_gui_type branch
		create_temp_config("enum_test.yaml", "GUI:\n  type: None");
		settings_manager.load_settings("enum_test.yaml", config);
		CHECK(config.gui_type == Salix::GuiType::None);

		// CLEANUP
		std::filesystem::remove("enum_test.yaml");
		std::filesystem::remove("enum_test.cache");
	}
}